class MaintenanceConfig
{
    float maintenanceRadius; // Raio de manutenção
    int resourcesPerCycle; // Recursos por ciclo de manutenção
    float checkInterval; // Intervalo de verificação
    float damagePerCycle; // Dano para estruturas por ciclo
    float vehicleDamagePerCycle; // Dano para veículos por ciclo
    ref array<string> affectedStructureTypes; // Tipos de estruturas afetadas pela manutenção
    string consumedItemType; // Tipo de item consumido na manutenção
    bool enableKitCrafting; // Habilita a criação de kits
    bool enableKitDecrafting; // Habilita a desmontagem de kits
    bool protectVehicles; // Protege veículos de danos
    ref array<string> restrictedPlacementItems; // Itens restritos para colocação
    bool requireMaintenanceUnitForKits; // Requer unidade de manutenção para kits
    bool enableOwnershipSystem; // Habilita o sistema de propriedade
    int minDistanceBetweenUnits; // Distância mínima entre unidades de manutenção
    bool limitDeployablesByLevel; // Habilita o sistema de limite de deployables por nível
    ref array<int> maxContainersPerLevel; // Limite de containers por nível
    ref array<int> maxBaseBuildingPerLevel; // Limite de partes de base por nível
    ref array<ref MaintenanceLevel> levelDefinitions; // Define os níveis e seus requisitos
    int minDistanceBetweenBuildings; // Distância mínima entre edifícios

    // Novo mapa para compatibilidade com mods de mobília
    ref map<string, string> ManagedFurnitureMap;

    // Construtor: define valores padrão para todas as configurações
    void MaintenanceConfig()
    {
        maintenanceRadius = 60.0;
        resourcesPerCycle = 10;
        checkInterval = 3600.0; // 1 hora
        damagePerCycle = 10000.0; // Dano padrão para estruturas
        vehicleDamagePerCycle = 250.0; // Dano padrão para veículos
        affectedStructureTypes = {"Fence", "WatchTower"}; // Tipos de estruturas afetadas
        consumedItemType = "Nail";
        enableKitCrafting = true;
        enableKitDecrafting = true;
        protectVehicles = true; // Habilitado por padrão
        requireMaintenanceUnitForKits = true; // Habilitado por padrão
        restrictedPlacementItems = new array<string>; // Inicializa a lista de itens restritos
        // Adiciona os kits de construção padrão como exemplo
        restrictedPlacementItems.Insert("FenceKit");
        restrictedPlacementItems.Insert("WatchtowerKit");
        restrictedPlacementItems.Insert("ShelterKit");
        enableOwnershipSystem = true; // Habilitado por padrão
        minDistanceBetweenUnits = 150; // Distância mínima em metros entre unidades de manutenção
        minDistanceBetweenBuildings = 25; // Distância mínima em metros entre edifícios
        limitDeployablesByLevel = true;
        maxContainersPerLevel = {2, 4, 6};
        maxBaseBuildingPerLevel = {1, 2, 3};


        // Inicializa as definições de nível com exemplos
        levelDefinitions = new array<ref MaintenanceLevel>;
        
        MaintenanceLevel level2 = new MaintenanceLevel();
        level2.level = 2;
        level2.requiredAttachments = {"M_UpgradeTo_Lvl2"};
        levelDefinitions.Insert(level2);

        MaintenanceLevel level3 = new MaintenanceLevel();
        level3.level = 3;
        level3.requiredAttachments = {"M_UpgradeTo_Lvl2", "M_UpgradeTo_Lvl3"};
        levelDefinitions.Insert(level3);
        
        // Inicializa o novo mapa
        ManagedFurnitureMap = new map<string, string>;
        ManagedFurnitureMap.Insert("ExampleFurnitureKit", "ExampleContainer"); // Exemplo: Kit -> Objeto Final
    }
}