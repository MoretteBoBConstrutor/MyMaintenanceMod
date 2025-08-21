// Estrutura para armazenar os dados de sobrevivência de um único jogador.
class MMS_PlayerData
{
    string m_PlayerID;      // Steam64ID do jogador
    float m_SurvivalTime;   // Tempo de vida acumulado em segundos para o personagem atual
    int m_SurvivalLevel;    // Nível atual da DogTag (0 = base, 1 = TypeOne, etc.)

    void MMS_PlayerData(string id)
    {
        m_PlayerID = id;
        m_SurvivalTime = 0;
        m_SurvivalLevel = 0;
    }
}

// Classe que representa o banco de dados de todos os jogadores.
class PlayerDatabase
{
    ref array<ref MMS_PlayerData> m_AllPlayerData;

    void PlayerDatabase()
    {
        m_AllPlayerData = new array<ref MMS_PlayerData>;
    }
}