/**
 * @class MilitaryEventData
 * @brief Dados de um evento militar para persistência
 */
class MilitaryEventData
{
    string m_EventID;           // ID único do evento
    vector m_Position;          // Posição do evento
    int m_Timestamp;            // Timestamp de quando foi criado
    bool m_IsActive;            // Se o evento ainda está ativo

    void MilitaryEventData(string eventID = "", vector position = "0 0 0", int timestamp = 0)
    {
        m_EventID = eventID;
        m_Position = position;
        m_Timestamp = timestamp;
        m_IsActive = true;
    }

    /**
     * @brief Gera um ID único baseado no ID numérico
     */
    static string GenerateEventID(int eventID)
    {
        return "Convoy_" + eventID;
    }

    /**
     * @brief Verifica se o evento é muito antigo (mais de 2 horas)
     */
    bool IsExpired()
    {
        int currentTime = GetGame().GetTime();
        int maxAge = 2 * 60 * 60 * 1000; // 2 horas em milissegundos
        return (currentTime - m_Timestamp) > maxAge;
    }
}
