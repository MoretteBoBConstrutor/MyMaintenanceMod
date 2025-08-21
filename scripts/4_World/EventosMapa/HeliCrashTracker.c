/**
 * @class HeliCrashTracker
 * @brief Sistema global para rastrear todos os HeliCrashes ativos no servidor
 */
class HeliCrashTracker
{
    private static ref array<CrashBase> m_ActiveCrashes;
    private static ref HeliCrashTracker m_Instance;

    void HeliCrashTracker()
    {
        if (!m_ActiveCrashes)
        {
            m_ActiveCrashes = new array<CrashBase>;
        }
        if (!m_CrashPositions)
        {
            m_CrashPositions = new array<vector>;
        }
    }

    /**
     * @brief Obtém a instância singleton do tracker
     */
    static HeliCrashTracker GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new HeliCrashTracker();
        }
        return m_Instance;
    }

    /**
     * @brief Registra um novo crash no sistema
     */
    static void RegisterCrash(CrashBase crash)
    {
        if (!crash) return;
        
        GetInstance(); // Garante que está inicializado
        
        // Verifica se já não está registrado
        if (m_ActiveCrashes.Find(crash) == -1)
        {
            m_ActiveCrashes.Insert(crash);
            
            // Registra no sistema de persistência
            if (GetGame().IsServer())
            {
                HeliCrashDatabaseManager.GetInstance().RegisterCrash(crash.GetPosition());
            }
            
            Print("[HeliCrashTracker] Crash registrado na posição: " + crash.GetPosition());
            Print("[HeliCrashTracker] Total de crashes ativos: " + m_ActiveCrashes.Count());
        }
    }

    /**
     * @brief Remove um crash do sistema
     */
    static void UnregisterCrash(CrashBase crash)
    {
        if (!crash) return;
        
        GetInstance(); // Garante que está inicializado
        
        vector crashPos = crash.GetPosition();
        int index = m_ActiveCrashes.Find(crash);
        
        if (index != -1)
        {
            // Crash encontrado na lista ativa - remove da memória
            m_ActiveCrashes.Remove(index);
            Print("[HeliCrashTracker] Crash removido da lista ativa. Total restante: " + m_ActiveCrashes.Count());
        }
        else
        {
            // Crash não está na lista ativa (provavelmente crash antigo persistido)
            Print("[HeliCrashTracker] Crash não encontrado na lista ativa - tentando remover crash antigo do database");
        }
        
        // SEMPRE tenta remover do database, independente se estava na lista ativa ou não
        if (GetGame().IsServer())
        {
            HeliCrashDatabaseManager.GetInstance().UnregisterCrash(crashPos);
        }
    }

    /**
     * @brief Obtém array com todos os crashes ativos
     */
    static array<CrashBase> GetAllCrashes()
    {
        GetInstance(); // Garante que está inicializado
        
        // Limpa crashes inválidos
        CleanupInvalidCrashes();
        
        return m_ActiveCrashes;
    }

    /**
     * @brief Remove crashes que foram destruídos ou não são mais válidos
     */
    static void CleanupInvalidCrashes()
    {
        if (!m_ActiveCrashes) return;
        
        for (int i = m_ActiveCrashes.Count() - 1; i >= 0; i--)
        {
            CrashBase crash = m_ActiveCrashes.Get(i);
            if (!crash || crash.IsSetForDeletion())
            {
                m_ActiveCrashes.Remove(i);
                Print("[HeliCrashTracker] Crash inválido removido da lista");
            }
        }
    }

    /**
     * @brief Obtém o número de crashes ativos
     */
    static int GetCrashCount()
    {
        GetInstance();
        CleanupInvalidCrashes();
        return m_ActiveCrashes.Count();
    }
    
    /**
     * @brief Sincroniza dados de crashes do servidor (via RPC)
     */
    static void SyncFromServer(HeliCrashMapData data)
    {
        if (!data) return;
        
        GetInstance();
        
        // Limpa arrays atuais COMPLETAMENTE
        m_ActiveCrashes.Clear();
        m_CrashPositions.Clear(); // ← CORREÇÃO: Limpar também o array de posições
        
        // Para cada posição recebida, cria dados simples apenas com posição
        for (int i = 0; i < data.m_CrashPositions.Count(); i++)
        {
            vector pos = data.m_CrashPositions.Get(i);
            // Adiciona posição diretamente ao array (agora limpo)
            m_CrashPositions.Insert(pos);
        }
        
        Print("[HeliCrashTracker] Sincronizado " + data.GetCrashCount() + " crashes do servidor");
    }
    
    /**
     * @brief Obtém array de posições simples para o mapa
     */
    static array<vector> GetCrashPositions()
    {
        GetInstance();
        
        // Se estamos no servidor, inclui dados persistidos
        if (GetGame().IsServer())
        {
            // Combina crashes ativos + crashes persistidos
            array<vector> allPositions = new array<vector>();
            
            // Adiciona crashes ativos em memória primeiro
            if (m_CrashPositions)
            {
                foreach (vector pos : m_CrashPositions)
                {
                    allPositions.Insert(pos);
                }
            }
            
            // Adiciona crashes persistidos que podem não estar em memória
            array<vector> persistedPositions = HeliCrashDatabaseManager.GetInstance().GetActiveCrashPositions();
            if (persistedPositions)
            {
                foreach (vector persistedPos : persistedPositions)
                {
                    // Verifica duplicatas com tolerância melhorada (15 metros)
                    bool exists = false;
                    foreach (vector existingPos : allPositions)
                    {
                        if (vector.Distance(existingPos, persistedPos) < 15.0) // Tolerância aumentada
                        {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists)
                    {
                        allPositions.Insert(persistedPos);
                    }
                }
            }
            
            Print("[HeliCrashTracker] GetCrashPositions servidor: " + allPositions.Count() + " posições únicas combinadas");
            return allPositions;
        }
        
        // No cliente, retorna apenas dados sincronizados (já limpos)
        Print("[HeliCrashTracker] GetCrashPositions cliente: " + m_CrashPositions.Count() + " posições");
        return m_CrashPositions;
    }
    
    /**
     * @brief Carrega dados persistidos na inicialização do servidor
     */
    static void LoadPersistedData()
    {
        if (GetGame().IsServer())
        {
            HeliCrashDatabaseManager.GetInstance().LoadData();
            Print("[HeliCrashTracker] Dados persistidos carregados");
        }
    }
    
    /**
     * @brief Validação periódica de crashes persistidos
     * Reimplementado com raio de 100m e mantendo CrashBase para teste
     */
    static void ValidatePersistedCrashes()
    {
        if (!GetGame().IsServer()) return;
        
        Print("[HeliCrashTracker] ValidatePersistedCrashes iniciado");
        HeliCrashDatabaseManager.GetInstance().ValidateCrashes();
        Print("[HeliCrashTracker] ValidatePersistedCrashes concluído");
    }
    
    /**
     * @brief Array simples de posições para uso no cliente
     */
    private static ref array<vector> m_CrashPositions;
}

