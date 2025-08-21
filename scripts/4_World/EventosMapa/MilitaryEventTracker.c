/**
 * @class MilitaryEventTracker
 * @brief Sistema para rastrear eventos militares para exibição no mapa
 */
class MilitaryEventTracker
{
    private static ref array<vector> m_ActiveEvents;
    private static ref array<vector> m_EventPositions;
    private static ref MilitaryEventTracker m_Instance;
    
    void MilitaryEventTracker()
    {
        if (!m_ActiveEvents)
        {
            m_ActiveEvents = new array<vector>;
        }
        if (!m_EventPositions)
        {
            m_EventPositions = new array<vector>;
        }
    }
    
    /**
     * @brief Obtém a instância singleton
     */
    static MilitaryEventTracker GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new MilitaryEventTracker();
        }
        return m_Instance;
    }
    
    /**
     * @brief Carrega dados persistidos na inicialização do servidor
     * CORRIGIDO: Sincroniza array em memória imediatamente (igual ao HeliCrash)
     */
    static void LoadPersistedData()
    {
        if (GetGame().IsServer())
        {
            MilitaryEventDatabaseManager.GetInstance().LoadData();
            
            // NOVO: Sincroniza array em memória imediatamente após carregar JSON
            // Isso previne duplicação na inicialização do servidor
            GetInstance();
            m_ActiveEvents.Clear();
            
            array<vector> persistedPositions = MilitaryEventDatabaseManager.GetInstance().GetActiveEventPositions();
            if (persistedPositions)
            {
                foreach (vector pos : persistedPositions)
                {
                    m_ActiveEvents.Insert(pos);
                }
                Print("[MilitaryEventTracker] Array sincronizado com " + persistedPositions.Count() + " eventos do JSON");
            }
            
            Print("[MilitaryEventTracker] Dados persistidos carregados e sincronizados");
        }
    }
    
    /**
     * @brief Registra um novo evento militar
     */
    static void RegisterEvent(vector position)
    {
        GetInstance();
        
        // Verifica se já existe evento próximo (100m)
        foreach (vector existingPos : m_ActiveEvents)
        {
            if (vector.Distance(existingPos, position) < 100.0)
            {
                Print("[MilitaryEventTracker] Evento duplicado ignorado: " + position);
                return;
            }
        }
        
        // Adiciona ao array de eventos ativos
        m_ActiveEvents.Insert(position);
        
        // Registra no sistema de persistência (igual ao HeliCrash)
        if (GetGame().IsServer())
        {
            MilitaryEventDatabaseManager.GetInstance().RegisterEvent(position);
        }
        
        Print("[MilitaryEventTracker] Evento registrado: " + position + " (Total: " + m_ActiveEvents.Count() + ")");
    }
    
    /**
     * @brief Remove um evento militar
     */
    static void UnregisterEvent(vector position)
    {
        GetInstance();
        
        // Remove evento do array em memória por posição próxima (tolerância de 50m)
        for (int i = m_ActiveEvents.Count() - 1; i >= 0; i--)
        {
            vector eventPos = m_ActiveEvents.Get(i);
            if (vector.Distance(eventPos, position) < 50.0)
            {
                m_ActiveEvents.Remove(i);
                Print("[MilitaryEventTracker] Evento militar removido: " + position);
                Print("[MilitaryEventTracker] Total de eventos restantes: " + m_ActiveEvents.Count());
                break;
            }
        }
        
        // Remove do banco de dados persistente
        if (GetGame().IsServer())
        {
            MilitaryEventDatabaseManager.GetInstance().UnregisterEvent(position);
        }
    }
    
    /**
     * @brief Obtém todas as posições de eventos militares
     * No servidor: combina dados em memória + persistidos
     * No cliente: apenas dados sincronizados
     */
    static array<vector> GetEventPositions()
    {
        GetInstance();
        
        if (GetGame().IsServer())
        {
            // Combina eventos ativos + eventos persistidos (igual ao HeliCrash)
            array<vector> allPositions = new array<vector>();
            
            // Adiciona eventos ativos em memória primeiro
            if (m_EventPositions)
            {
                foreach (vector pos : m_EventPositions)
                {
                    allPositions.Insert(pos);
                }
            }
            
            // Adiciona eventos persistidos que podem não estar em memória
            array<vector> persistedPositions = MilitaryEventDatabaseManager.GetInstance().GetActiveEventPositions();
            if (persistedPositions)
            {
                foreach (vector persistedPos : persistedPositions)
                {
                    // Verifica duplicatas com tolerância (15 metros como o HeliCrash)
                    bool exists = false;
                    foreach (vector existingPos : allPositions)
                    {
                        if (vector.Distance(existingPos, persistedPos) < 15.0)
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
            
            Print("[MilitaryEventTracker] GetEventPositions servidor: " + allPositions.Count() + " posições únicas combinadas");
            return allPositions;
        }
        
        // No cliente, retorna apenas dados sincronizados (já limpos)
        Print("[MilitaryEventTracker] GetEventPositions cliente: " + m_EventPositions.Count() + " posições");
        return m_EventPositions;
    }
    
    /**
     * @brief Remove eventos antigos (limpeza periódica)
     */
    static void CleanupOldEvents()
    {
        GetInstance();
        
        // Implementar lógica de cleanup se necessário
        // Por enquanto, eventos ficam até restart do servidor
        Print("[MilitaryEventTracker] Cleanup executado. Eventos ativos: " + m_ActiveEvents.Count());
    }
    
    /**
     * @brief Validação física - Remove eventos que não existem mais no mundo
     * Agora usa o sistema de banco de dados para validação completa
     */
    static void ValidateEvents()
    {
        if (!GetGame().IsServer()) return;
        
        Print("[MilitaryEventTracker] ValidateEvents iniciado - usando banco de dados");
        MilitaryEventDatabaseManager.GetInstance().ValidateEvents();
        Print("[MilitaryEventTracker] ValidateEvents concluído");
    }
    
    /**
     * @brief Limpa todos os eventos (restart)
     */
    static void ClearAllEvents()
    {
        GetInstance();
        m_ActiveEvents.Clear();
        Print("[MilitaryEventTracker] Todos os eventos militares limpos");
    }
    
    /**
     * @brief Sincroniza eventos do servidor (para clientes)
     */
    static void SyncFromServer(array<vector> serverEvents)
    {
        GetInstance();
        m_ActiveEvents.Clear();
        m_EventPositions.Clear();
        
        for (int i = 0; i < serverEvents.Count(); i++)
        {
            vector pos = serverEvents.Get(i);
            m_ActiveEvents.Insert(pos);
            m_EventPositions.Insert(pos);
        }
        
        Print("[MilitaryEventTracker] Sincronizado " + m_EventPositions.Count() + " eventos militares do servidor");
    }
}
