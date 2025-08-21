/**
 * @class EventBeaconConfig
 * @brief Configuração para o sistema de monitoramento de eventos via beacon
 */
class EventBeaconConfig
{
    private static ref EventBeaconConfig m_Instance;
    private const string JSON_PATH = "$profile:MaintenanceMod/EventBeaconConfig.json";
    
    bool m_EnableEventMonitoring = true;          // Ativar/desativar sistema
    bool m_EnableDebugLogs = true;               // Logs detalhados
    
    /**
     * @brief Obtém a instância singleton
     */
    static EventBeaconConfig GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new EventBeaconConfig();
            m_Instance.Load();
        }
        return m_Instance;
    }
    
    /**
     * @brief Salva configuração em JSON
     */
    void Save()
    {
        if (!FileExist("$profile:MaintenanceMod/"))
        {
            MakeDirectory("$profile:MaintenanceMod/");
        }
        JsonFileLoader<EventBeaconConfig>.JsonSaveFile(JSON_PATH, this);
        Print("[EventBeaconConfig] Configuração salva em: " + JSON_PATH);
    }
    
    /**
     * @brief Carrega configuração do JSON
     */
    void Load()
    {
        if (FileExist(JSON_PATH))
        {
            JsonFileLoader<EventBeaconConfig>.JsonLoadFile(JSON_PATH, this);
            Print("[EventBeaconConfig] Configuração carregada do arquivo: " + JSON_PATH);
        }
        else
        {
            Print("[EventBeaconConfig] Arquivo não encontrado. Criando configuração padrão em: " + JSON_PATH);
            Save(); // Cria arquivo padrão
        }
    }
}
