/**
 * @class MilitaryEventDatabase
 * @brief Database para persistir dados de eventos militares
 */
class MilitaryEventDatabase
{
    ref array<ref MilitaryEventData> m_AllEventData;
    int m_NextEventID; // Controle de próximo ID sequencial

    void MilitaryEventDatabase()
    {
        m_AllEventData = new array<ref MilitaryEventData>();
        m_NextEventID = 1; // Inicia com ID 1
    }

    /**
     * @brief Obtém e incrementa o próximo ID disponível
     */
    int GetNextEventID()
    {
        int currentID = m_NextEventID;
        m_NextEventID++;
        return currentID;
    }

    /**
     * @brief Adiciona um novo evento ao database
     */
    void AddEvent(MilitaryEventData eventData)
    {
        if (eventData)
        {
            m_AllEventData.Insert(eventData);
        }
    }

    /**
     * @brief Remove um evento do database pelo ID - REMOÇÃO REAL
     */
    bool RemoveEvent(string eventID)
    {
        for (int i = 0; i < m_AllEventData.Count(); i++)
        {
            if (m_AllEventData[i].m_EventID == eventID)
            {
                m_AllEventData.Remove(i);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Encontra um evento pelo ID
     */
    MilitaryEventData FindEvent(string eventID)
    {
        foreach (MilitaryEventData data : m_AllEventData)
        {
            if (data.m_EventID == eventID)
                return data;
        }
        return null;
    }

    /**
     * @brief Remove eventos expirados
     */
    int CleanupExpiredEvents()
    {
        int removed = 0;
        for (int i = m_AllEventData.Count() - 1; i >= 0; i--)
        {
            if (m_AllEventData[i].IsExpired())
            {
                m_AllEventData.Remove(i);
                removed++;
            }
        }
        return removed;
    }

    /**
     * @brief Obtém todas as posições ativas
     */
    array<vector> GetActivePositions()
    {
        array<vector> positions = new array<vector>();
        foreach (MilitaryEventData data : m_AllEventData)
        {
            if (data.m_IsActive && !data.IsExpired())
            {
                positions.Insert(data.m_Position);
            }
        }
        return positions;
    }
}
