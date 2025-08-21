/**
 * @class MilitaryEventDatabaseManager
 * @brief Gerenciador de persistência para eventos militares
 */
class MilitaryEventDatabaseManager
{
    private static ref MilitaryEventDatabaseManager m_Instance;
    private ref MilitaryEventDatabase m_Database;
    private const string JSON_PATH = "$profile:MaintenanceMod/MilitaryEventDatabase.json";

    static MilitaryEventDatabaseManager GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new MilitaryEventDatabaseManager();
        }
        return m_Instance;
    }

    private void MilitaryEventDatabaseManager()
    {
        m_Database = new MilitaryEventDatabase();
    }

    /**
     * @brief Carrega dados do JSON
     */
    void LoadData()
    {
        if (FileExist(JSON_PATH))
        {
            JsonFileLoader<MilitaryEventDatabase>.JsonLoadFile(JSON_PATH, m_Database);
            
            // Limpa eventos expirados após carregar
            int removed = m_Database.CleanupExpiredEvents();
            if (removed > 0)
            {
                SaveData(); // Salva após limpeza
            }
            
            Print("[MilitaryEventDB] Database carregado com " + m_Database.m_AllEventData.Count() + " eventos. " + removed + " expirados removidos.");
        }
        else
        {
            Print("[MilitaryEventDB] Arquivo de database não encontrado. Um novo será criado.");
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
        JsonFileLoader<MilitaryEventDatabase>.JsonSaveFile(JSON_PATH, m_Database);
        Print("[MilitaryEventDB] Database salvo com " + m_Database.m_AllEventData.Count() + " eventos.");
    }

    /**
     * @brief Registra um novo evento com ID sequencial
     */
    void RegisterEvent(vector position)
    {
        int eventID = m_Database.GetNextEventID();
        int timestamp = GetGame().GetTime();
        
        string eventIDStr = MilitaryEventData.GenerateEventID(eventID);
        MilitaryEventData newEvent = new MilitaryEventData(eventIDStr, position, timestamp);
        m_Database.AddEvent(newEvent);
        
        SaveData();
        Print("[MilitaryEventDB] Novo evento militar registrado: ID=" + eventID + " na posição " + position);
    }

    /**
     * @brief Remove um evento completamente (quando é destruído)
     */
    void UnregisterEvent(vector position)
    {
        // Remove evento por posição próxima (tolerância de 20m)
        for (int i = m_Database.m_AllEventData.Count() - 1; i >= 0; i--)
        {
            MilitaryEventData data = m_Database.m_AllEventData[i];
            if (vector.Distance(data.m_Position, position) < 20.0)
            {
                Print("[MilitaryEventDB] Removendo evento ID=" + data.m_EventID + " na posição " + position);
                m_Database.m_AllEventData.Remove(i);
                SaveData();
                return;
            }
        }
    }

    /**
     * @brief Obtém todas as posições ativas
     */
    array<vector> GetActiveEventPositions()
    {
        return m_Database.GetActivePositions();
    }

    /**
     * @brief Validação periódica - REMOVE eventos que não existem mais no mundo
     * Remove realmente em vez de marcar como inativo, usando raio de 15m
     */
    void ValidateEvents()
    {
        if (!GetGame().IsServer()) return;
        
        int validated = 0;
        int removed = 0;
        
        // Percorre todas as posições de eventos de trás para frente
        for (int i = m_Database.m_AllEventData.Count() - 1; i >= 0; i--)
        {
            MilitaryEventData eventData = m_Database.m_AllEventData[i];
            validated++;
            
            // Procura por EventBeacon num raio de 15m da posição registrada
            array<Object> nearbyObjects = new array<Object>;
            GetGame().GetObjectsAtPosition3D(eventData.m_Position, 15.0, nearbyObjects, null);
            
            bool foundBeacon = false;
            foreach (Object obj : nearbyObjects)
            {
                if (obj.IsKindOf("EventBeacon"))
                {
                    foundBeacon = true;
                    break;
                }
            }
            
            // Se não encontrou o beacon físico OU está expirado, remove da lista
            if (!foundBeacon || eventData.IsExpired())
            {
                Print("[MilitaryEventDB] Evento órfão/expirado removido: ID=" + eventData.m_EventID + " posição=" + eventData.m_Position);
                m_Database.m_AllEventData.Remove(i);
                removed++;
            }
        }
        
        if (removed > 0)
        {
            SaveData(); // Salva apenas se houve mudanças
        }
        
        Print("[MilitaryEventDB] Validação concluída: " + validated + " verificados, " + removed + " removidos");
    }
}
