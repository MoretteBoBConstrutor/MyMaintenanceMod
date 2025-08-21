class MMS_PlayerDatabaseManager
{
    private static ref MMS_PlayerDatabaseManager m_Instance;
    private ref PlayerDatabase m_Database;
    private const string JSON_PATH = "$profile:MaintenanceMod/PlayerDatabase.json";

    static MMS_PlayerDatabaseManager GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new MMS_PlayerDatabaseManager();
        }
        return m_Instance;
    }

    private void MMS_PlayerDatabaseManager()
    {
        m_Database = new PlayerDatabase();
    }

    void LoadData()
    {
        if (FileExist(JSON_PATH))
        {
            JsonFileLoader<PlayerDatabase>.JsonLoadFile(JSON_PATH, m_Database);
            MMLogger.Log("[PlayerDB] Banco de dados de jogadores carregado com " + m_Database.m_AllPlayerData.Count() + " jogadores.");
        }
        else
        {
            MMLogger.Log("[PlayerDB] Arquivo de banco de dados de jogadores não encontrado. Um novo será criado.");
        }
    }

    void SaveData()
    {
        if (!FileExist("$profile:MaintenanceMod/"))
        {
            MakeDirectory("$profile:MaintenanceMod/");
        }
        JsonFileLoader<PlayerDatabase>.JsonSaveFile(JSON_PATH, m_Database);
        MMLogger.Log("[PlayerDB] Banco de dados de jogadores salvo.");
    }

    MMS_PlayerData FindPlayerData(string playerID)
    {
        if (playerID == "") return null;

        foreach (MMS_PlayerData data : m_Database.m_AllPlayerData)
        {
            if (data.m_PlayerID == playerID)
            {
                return data;
            }
        }
        return null;
    }

    MMS_PlayerData GetOrCreatePlayerData(string playerID)
    {
        MMS_PlayerData data = FindPlayerData(playerID);
        if (!data)
        {
            data = new MMS_PlayerData(playerID);
            m_Database.m_AllPlayerData.Insert(data);
            MMLogger.Log("[PlayerDB] Nova entrada de dados criada para o jogador: " + playerID);
        }
        return data;
    }
}