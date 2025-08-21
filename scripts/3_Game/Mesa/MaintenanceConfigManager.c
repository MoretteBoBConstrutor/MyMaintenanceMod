class MaintenanceConfigManager
{
    private static ref MaintenanceConfigManager m_Instance;
    private ref MaintenanceConfig m_Config;

    // Construtor privado: carrega ou cria o arquivo de configuração
    private void MaintenanceConfigManager()
    {
        m_Config = new MaintenanceConfig();
        string dir_path = "$profile:MaintenanceMod";
        string file_path = dir_path + "/config.json";
        // Cria diretório se não existir
        if (!FileExist(dir_path))
        {
            MakeDirectory(dir_path);
        }
        // Carrega ou cria o arquivo de configuração
        if (FileExist(file_path))
        {
            JsonFileLoader<MaintenanceConfig>.JsonLoadFile(file_path, m_Config);
            MMLogger.Log("Configuração carregada de: " + file_path);
        }
        else
        {
            JsonFileLoader<MaintenanceConfig>.JsonSaveFile(file_path, m_Config);
            MMLogger.Log("Arquivo de configuração não encontrado. Criando padrão em: " + file_path);
        }

    }


    // Singleton: retorna a instância única do gerenciador
    static MaintenanceConfigManager GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new MaintenanceConfigManager();
        }
        return m_Instance;
    }

    // Retorna o raio de atuação da unidade
    float GetMaintenanceRadius()
    {
        return m_Config.maintenanceRadius;
    }

    // Retorna a quantidade de recursos consumidos por ciclo
    int GetResourcesPerCycle()
    {
        return m_Config.resourcesPerCycle;
    }

    // Retorna o intervalo entre ciclos de manutenção
    float GetCheckInterval()
    {
        return m_Config.checkInterval;
    }

    // Retorna o dano por ciclo para estruturas
    float GetDamagePerCycle()
    {
        return m_Config.damagePerCycle;
    }

    // Retorna o dano por ciclo para veículos
    float GetVehicleDamagePerCycle()
    {
        return m_Config.vehicleDamagePerCycle;
    }

    // Retorna os tipos de estruturas afetadas
    array<string> GetAffectedStructureTypes()
    {
        return m_Config.affectedStructureTypes;
    }

    // Retorna o tipo de item consumido
    string GetConsumedItemType()
    {
        return m_Config.consumedItemType;
    }

    // Retorna se o crafting do kit está habilitado
    bool GetEnableKitCrafting()
    {
        return m_Config.enableKitCrafting;
    }

    // Retorna se o decrafting do kit está habilitado
    bool GetEnableKitDecrafting()
    {
        return m_Config.enableKitDecrafting;
    }

    // Retorna se veículos são protegidos
    bool GetProtectVehicles()
    {
        return m_Config.protectVehicles;
    }

    // Retorna lista de itens restritos para placement
    array<string> GetRestrictedPlacementItems()
    {
        return m_Config.restrictedPlacementItems;
    }

    // Retorna se é necessário unidade de manutenção para kits
    bool GetRequireMaintenanceUnitForKits()
    {
        return m_Config.requireMaintenanceUnitForKits;
    }

    // Retorna a distância mínima entre unidades
    int GetMinDistanceBetweenUnits()
    {
        return m_Config.minDistanceBetweenUnits;
    }
    // Retorna a distância mínima entre edifícios
    int GetMinDistanceBetweenBuildings()
    {
        return m_Config.minDistanceBetweenBuildings;
    }

    // Retorna se o sistema de propriedade está habilitado
    bool GetEnableOwnershipSystem()
    {
        return m_Config.enableOwnershipSystem;
    }

    // Retorna as definições de nível
    array<ref MaintenanceLevel> GetLevelDefinitions()
    {
        return m_Config.levelDefinitions;
    }

    // Retorna se o limite de deployables por nível está habilitado
    bool GetLimitDeployablesByLevel()
    {
        return m_Config.limitDeployablesByLevel;
    }

    /**
     * @brief Retorna o número máximo de containers permitidos para um determinado nível.
     * @param level O nível da unidade (1, 2, 3...).
     * @return O limite de containers para aquele nível.
     */
    int GetMaxContainersForLevel(int level)
    {
        // O nível é 1-based, o array é 0-based.
        int index = level - 1;
        if (index < 0) index = 0; // Segurança para nível 0 ou menor.

        if (!m_Config.maxContainersPerLevel || m_Config.maxContainersPerLevel.Count() == 0)
            return 0; // Retorna 0 se a lista estiver vazia.

        // Se o nível for maior que os limites definidos, retorna o maior limite disponível.
        if (index >= m_Config.maxContainersPerLevel.Count())
        {
            index = m_Config.maxContainersPerLevel.Count() - 1;
        }

        return m_Config.maxContainersPerLevel.Get(index);
    }

    /**
     * @brief Retorna o número máximo de partes de construção permitidas para um determinado nível.
     * @param level O nível da unidade (1, 2, 3...).
     * @return O limite de partes de construção para aquele nível.
     */
    int GetMaxBaseBuildingForLevel(int level)
    {
        // O nível é 1-based, o array é 0-based.
        int index = level - 1;
        if (index < 0) index = 0;

        if (!m_Config.maxBaseBuildingPerLevel || m_Config.maxBaseBuildingPerLevel.Count() == 0)
            return 0;

        if (index >= m_Config.maxBaseBuildingPerLevel.Count())
        {
            index = m_Config.maxBaseBuildingPerLevel.Count() - 1;
        }

        return m_Config.maxBaseBuildingPerLevel.Get(index);
    }

    /**
     * @brief Retorna o nome da classe do objeto final para um determinado kit gerenciado.
     * @param kitClassName O nome da classe do kit a ser verificado.
     * @return O nome da classe do objeto final, ou uma string vazia se não for encontrado.
     */
    string GetDeployedTypeForKit(string kitClassName)
    {
        if (m_Config && m_Config.ManagedFurnitureMap)
            return m_Config.ManagedFurnitureMap.Get(kitClassName);
        
        return "";
    }
};