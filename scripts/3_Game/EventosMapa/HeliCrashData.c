/**
 * @class HeliCrashData
 * @brief Dados de um HeliCrash para persistência
 */
class HeliCrashData
{
    string m_CrashID;           // ID único do crash
    vector m_Position;          // Posição do crash
    int m_Timestamp;            // Timestamp de quando foi criado
    bool m_IsActive;            // Se o crash ainda está ativo

    void HeliCrashData(string crashID = "", vector position = "0 0 0", int timestamp = 0)
    {
        m_CrashID = crashID;
        m_Position = position;
        m_Timestamp = timestamp;
        m_IsActive = true;
    }

    /**
     * @brief Gera um ID único baseado na posição e timestamp
     */
    static string GenerateCrashID(vector position)
    {
        int timestamp = GetGame().GetTime();
        return "crash_" + position[0] + "_" + position[2] + "_" + timestamp;
    }

    /**
     * @brief Verifica se o crash é muito antigo (mais de 4 horas)
     */
    bool IsExpired()
    {
        int currentTime = GetGame().GetTime();
        int maxAge = 4 * 60 * 60 * 1000; // 4 horas em milissegundos
        return (currentTime - m_Timestamp) > maxAge;
    }
}
