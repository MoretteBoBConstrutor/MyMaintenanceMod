/**
 * @class HeliCrashDatabase
 * @brief Database para persistir dados de HeliCrashes
 */
class HeliCrashDatabase
{
    ref array<ref HeliCrashData> m_AllCrashData;
    int m_NextCrashID; // Controle de próximo ID sequencial

    void HeliCrashDatabase()
    {
        m_AllCrashData = new array<ref HeliCrashData>();
        m_NextCrashID = 1; // Inicia com ID 1
    }

    /**
     * @brief Obtém e incrementa o próximo ID disponível
     */
    int GetNextCrashID()
    {
        int currentID = m_NextCrashID;
        m_NextCrashID++;
        return currentID;
    }

    /**
     * @brief Adiciona um novo crash ao database
     */
    void AddCrash(HeliCrashData crashData)
    {
        if (crashData)
        {
            m_AllCrashData.Insert(crashData);
        }
    }

    /**
     * @brief Remove um crash do database pelo ID - REMOÇÃO REAL
     */
    bool RemoveCrash(string crashID)
    {
        for (int i = 0; i < m_AllCrashData.Count(); i++)
        {
            if (m_AllCrashData[i].m_CrashID == crashID)
            {
                m_AllCrashData.Remove(i);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Encontra um crash pelo ID
     */
    HeliCrashData FindCrash(string crashID)
    {
        foreach (HeliCrashData data : m_AllCrashData)
        {
            if (data.m_CrashID == crashID)
                return data;
        }
        return null;
    }

    /**
     * @brief Remove crashes expirados
     */
    int CleanupExpiredCrashes()
    {
        int removed = 0;
        for (int i = m_AllCrashData.Count() - 1; i >= 0; i--)
        {
            if (m_AllCrashData[i].IsExpired())
            {
                m_AllCrashData.Remove(i);
                removed++;
            }
        }
        return removed;
    }

    /**
     * @brief Obtém array de posições - todos os crashes são ativos agora (não há inativos)
     */
    array<vector> GetActivePositions()
    {
        array<vector> positions = new array<vector>();
        foreach (HeliCrashData data : m_AllCrashData)
        {
            // Não precisa mais verificar m_IsActive - crashes inativos são removidos completamente
            positions.Insert(data.m_Position);
        }
        return positions;
    }
}
