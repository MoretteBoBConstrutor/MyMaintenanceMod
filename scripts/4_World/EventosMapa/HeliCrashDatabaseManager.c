/**
 * @class HeliCrashDatabaseManager
 * @brief Gerenciador de persistência para HeliCrashes
 */
class HeliCrashDatabaseManager
{
    private static ref HeliCrashDatabaseManager m_Instance;
    private ref HeliCrashDatabase m_Database;
    private const string JSON_PATH = "$profile:MaintenanceMod/HeliCrashDatabase.json";

    static HeliCrashDatabaseManager GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new HeliCrashDatabaseManager();
        }
        return m_Instance;
    }

    private void HeliCrashDatabaseManager()
    {
        m_Database = new HeliCrashDatabase();
    }

    /**
     * @brief Carrega dados do JSON
     */
    void LoadData()
    {
        if (FileExist(JSON_PATH))
        {
            JsonFileLoader<HeliCrashDatabase>.JsonLoadFile(JSON_PATH, m_Database);
            
            // Limpa crashes expirados após carregar
            int removed = m_Database.CleanupExpiredCrashes();
            if (removed > 0)
            {
                SaveData(); // Salva após limpeza
            }
            
            Print("[HeliCrashDB] Database carregado com " + m_Database.m_AllCrashData.Count() + " crashes. " + removed + " expirados removidos.");
        }
        else
        {
            Print("[HeliCrashDB] Arquivo de database não encontrado. Um novo será criado.");
        }
    }

    /**
     * @brief Salva dados no JSON
     */
    void SaveData()
    {
        if (!FileExist("$profile:MaintenanceMod/"))
        {
            MakeDirectory("$profile:MaintenanceMod/");
        }
        JsonFileLoader<HeliCrashDatabase>.JsonSaveFile(JSON_PATH, m_Database);
        Print("[HeliCrashDB] Database salvo com " + m_Database.m_AllCrashData.Count() + " crashes.");
    }

    /**
     * @brief Registra um novo crash com ID sequencial
     */
    void RegisterCrash(vector position)
    {
        int crashID = m_Database.GetNextCrashID();
        int timestamp = GetGame().GetTime();
        
        HeliCrashData newCrash = new HeliCrashData(crashID.ToString(), position, timestamp);
        m_Database.AddCrash(newCrash);
        
        SaveData();
        Print("[HeliCrashDB] Novo crash registrado: ID=" + crashID + " na posição " + position);
    }

    /**
     * @brief Remove um crash completamente (quando é destruído)
     */
    void UnregisterCrash(vector position)
    {
        // Encontra crash por posição próxima (tolerância de 10 metros)
        for (int i = m_Database.m_AllCrashData.Count() - 1; i >= 0; i--)
        {
            HeliCrashData data = m_Database.m_AllCrashData[i];
            if (vector.Distance(data.m_Position, position) < 20.0)
            {
                Print("[HeliCrashDB] Removendo crash ID=" + data.m_CrashID + " na posição " + position);
                m_Database.m_AllCrashData.Remove(i);
                SaveData();
                return;
            }
        }
    }

    /**
     * @brief Obtém todas as posições ativas
     */
    array<vector> GetActiveCrashPositions()
    {
        return m_Database.GetActivePositions();
    }

    /**
     * @brief Validação periódica - REMOVE crashes que não existem mais no mundo
     * Agora remove realmente em vez de marcar como inativo, usando raio de 50m
     */
    void ValidateCrashes()
    {
        if (!GetGame().IsServer()) return;
        
        int validated = 0;
        int removed = 0;
        
        Print("[HeliCrashDB] Iniciando validação de crashes...");
        
        // Itera de trás para frente para poder remover itens com segurança
        for (int i = m_Database.m_AllCrashData.Count() - 1; i >= 0; i--)
        {
            HeliCrashData data = m_Database.m_AllCrashData[i];
            validated++;
            
            // Verifica se existe algum helicrash próximo a esta posição (raio de 50m)
            bool found = false;
            array<Object> objects = new array<Object>();
            GetGame().GetObjectsAtPosition3D(data.m_Position, 50.0, objects, null);
            
            Print("[HeliCrashDB] Verificando ID=" + data.m_CrashID + " na posição " + data.m_Position + " - encontrados " + objects.Count() + " objetos em 50m");
            
            foreach (Object obj : objects)
            {
                // Verifica tipos específicos de helicrash
                if (obj.IsKindOf("CrashBase") || obj.IsKindOf("Wreck_UH1Y") || obj.IsKindOf("Wreck_Mi8_Crashed"))
                {
                    found = true;
                    Print("[HeliCrashDB] Helicrash confirmado ID=" + data.m_CrashID + " (Tipo: " + obj.GetType() + ")");
                    break;
                }
            }
            
            // Se não encontrou, REMOVE completamente do database
            if (!found)
            {
                Print("[HeliCrashDB] Removendo crash ID=" + data.m_CrashID + " - não encontrado no mundo");
                m_Database.m_AllCrashData.Remove(i);
                removed++;
            }
        }
        
        if (removed > 0)
        {
            SaveData();
            Print("[HeliCrashDB] Validação concluída: " + validated + " verificados, " + removed + " REMOVIDOS permanentemente.");
        }
        else
        {
            Print("[HeliCrashDB] Validação concluída: " + validated + " verificados, nenhum removido.");
        }
    }
}
