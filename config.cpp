class CfgPatches
{
    class MyMaintenanceMod
    {
        units[] = {"MaintenanceUnit"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Characters", "DZ_Gear_Camping", "DZ_Structures_Military", "DZ_CommunityFramework"};
    };
};

class CfgMods
{
    class MyMaintenanceMod
    {
        dir = "MyMaintenanceMod";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "MyMaintenanceMod";
        credits = "";
        author = "SeuNome";
        version = "1.0";
        type = "mod";
        inputs = "MyMaintenanceMod\inputs\inputs.xml";
        dependencies[] = {"Game","World","Mission"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"MyMaintenanceMod/scripts/Common","MyMaintenanceMod/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"MyMaintenanceMod/scripts/Common","MyMaintenanceMod/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"MyMaintenanceMod/scripts/Common","MyMaintenanceMod/scripts/5_Mission"};
            };
        };
    };
};


class CfgSlots
{
    
   // Slot para a DogTag que fica presa ao jogador.
   class Slot_Kdu_link
   {
       name = "Kdu_link";
       displayName = "";
       ghostIcon = "set:dayz_inventory image:pouchright";
   };

    //Slot para a DogTag que é inserida no Jammer.
    class Slot_JammerDogTag
    {
        name = "JammerDogTag";
        displayName = "Anexar Dog Tag";
        ghostIcon = "set:dayz_inventory image:pouchright";
    };

    class Slot_Upgrade1
    {
        name = "Upgrade1";
        displayName = "Upgrade Lvl 1";
        ghostIcon = "set:dayz_inventory image:paper";
    };
    class Slot_Upgrade2
    {
        name = "Upgrade2";
        displayName = "Upgrade Lvl 2";
        ghostIcon = "set:dayz_inventory image:paper";
    };
    class Slot_Upgrade3
    {
        name = "Upgrade3";
        displayName = "Upgrade Lvl 3";
        ghostIcon = "set:dayz_inventory image:paper";
    };
    class Slot_Upgrade4
    {
        name = "Upgrade4";
        displayName = "Upgrade Lvl 4";
        ghostIcon = "set:dayz_inventory image:paper";
    };
};

class CfgSoundShaders
{
    class Jammer_Sound_Loop_Shader
    {
        // Aponta para o seu arquivo de som customizado. O caminho é a partir da raiz do PBO.
        samples[] = {{"MyMaintenanceMod\data\sounds\jammer_loop", 1}};
        // Volume base do som.
        volume = 0.5;
        // Distância máxima em metros que o som pode ser ouvido.
        range = 30;
    };
    class Jammer_Sound_Instant_Shader
    {
        // Aponta para o seu arquivo de som customizado. O caminho é a partir da raiz do PBO.
        samples[] = {{"MyMaintenanceMod\data\sounds\jammer_break", 1}};
        // Volume base do som.
        volume = 1;
        // Distância máxima em metros que o som pode ser ouvido.
        range = 10;
    };
};
class CfgSoundSets
{
    class Jammer_Loop_SoundSet
    {
        // Aponta para o SoundShader que definimos acima.
        soundShaders[] = {"Jammer_Sound_Loop_Shader"};
        // Reutilizamos configurações de som 3D do gerador para garantir um comportamento correto.
        sound3DProcessingType = "powerGenerator_Sound3DProcessingType";
        volumeCurve = "speech_SoundCurve";
        // Multiplicador de volume geral.
        volumeFactor = 1.0;
        // 1 = Som 3D posicional.
        spatial = 1;
        // 1 = Tocar em loop contínuo.
        loop = 1;
    };
    class Jammer_Instant_SoundSet
    {
        soundShaders[] = {"Jammer_Sound_Instant_Shader"};
        sound3DProcessingType = "powerGenerator_Sound3DProcessingType";
        volumeCurve = "speech_SoundCurve";
        volumeFactor = 1.0;
        spatial = 1;
        loop = 0;
    };
};
class CfgSoundCurves
{
    // Nossa curva customizada para os bipes do Jammer
    class Jammer_Beep_SoundCurve
    {
        points[] = 
        {
            {0, 1},     // A 0 metros, volume máximo (100%)
            {5, 0.6},   // A 5 metros, o volume já caiu para 60%
            {15, 0}     // A 15 metros (o 'range' do nosso som), o volume é zero.
        };
    };
};

class CfgVehicles
{
    class BaseBuildingBase;
    class Container_Base;
    class Inventory_Base;
    class KitBase;
    class WoodenCrate;
	class Man;
    class InventoryEquipment;
	class SurvivorBase: Man
	{
		attachments[] += {"Kdu_link"};
		class InventoryEquipment
		{
			playerSlots[] += {"Slot_Kdu_link"};
		};
	};
	class SurvivorMale_Base: SurvivorBase
	{
		model = "MyMaintenanceMod\data\character\askal_bodyparts.p3d";
		simpleHiddenSelections[] += {"kdu_link"};
	};
	class SurvivorFemale_Base: SurvivorBase
	{
		model = "MyMaintenanceMod\data\character\askal_bodyparts.p3d";
		simpleHiddenSelections[] += {"kdu_link"};
	};
    //==============================================
    //============ Mesa de Manutenção ==============
    //==============================================

    class MaintenanceUnitKit : WoodenCrate
    {
        scope = 2;
        displayName = "Kit de Unidade de Manutenção";
        descriptionShort = "Kit para montar uma Unidade de Manutenção.";
        hiddenSelectionsTextures[]= {"MyMaintenanceMod\data\MaintenceUnit\materials\MaintenanceKit_co.paa"};
        itemSize[] = {3,3};
        class Cargo {};
    };
    class MaintenanceUnit_Hologram: Inventory_Base
    {
        scope = 2;
        displayName = "HOLOGRAM Unidade de Manutenção";
        descriptionShort = "HOLOGRAM para posicionar a Unidade de Manutenção.";
        model = "\MyMaintenanceMod\data\MaintenceUnit\MaintenceUnit_Holo.p3d";
        placement = "ForceSlopeOnTerrain";
        physLayer = "item_large";
        overrideDrawArea = "5.0";
    };
    class MaintenanceUnit : Container_Base
    {
        scope = 2;
        displayName = "Unidade de Manutenção";
        descriptionShort = "Mantém sua base com manutenção automática.";
        model = "\MyMaintenanceMod\data\MaintenceUnit\MaintenanceUnit.p3d";
        weight = 10000;
        itemSize[] = {3,3};
        physLayer = "item_large";
        isContainer = 1;
        canBePlacedAnywhere = 1;
        isMeleeWeapon = 0;
        carriable = 1;
        heavyItem = 1;
        inventorySlot[] = {};
        attachments[] = {"Upgrade1","Upgrade2","Upgrade3","Upgrade4"};
        hiddenSelections[] = {"zbytek"};
        class Cargo
        {
            itemsCargoSize[] = {5, 5};
            openable = 0;
            allowOwnedCargoManipulation = 1;
        };
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 5000;
                    transferToAttachmentsCoef = 0.06;
                    healthLevels[] = {{1.01,{""}},{0.7,{""}},{0.5,{""}},{0.3,{""}},{0.01,{""}}};
                };
            };
            class GlobalArmor
            {
                class Projectile
                {
                    class Health
                    {
                        damage = 1;
                    };
                    class Blood
                    {
                        damage = 0;
                    };
                    class Shock
                    {
                        damage = 0;
                    };
                };
                class Melee
                {
                    class Health
                    {
                        damage = 0;
                    };
                    class Blood
                    {
                        damage = 0;
                    };
                    class Shock
                    {
                        damage = 0;
                    };
                };
                class FragGrenade
                {
                    class Health
                    {
                        damage = 8;
                    };
                    class Blood
                    {
                        damage = 0;
                    };
                    class Shock
                    {
                        damage = 0;
                    };
                };
            };
        };
    };


    //==============================================
    //=== Upgrades para a Unidade de Manutenção ====
    //==============================================
    class Maintenance_UpgradeBase: Inventory_Base
    {
        scope = 0; // Base class, não deve ser instanciada diretamente.
        displayName = "Upgrade Base";
        descriptionShort = "Base para upgrades da Unidade de Manutenção.";
        model="\MyMaintenanceMod\data\upgradeKeys\UpgradeKeys.p3d";
        itemSize[] = {2,2};
        rotationFlags=16;
        weight = 100;
        hiddenSelections[] = {"disk"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 1000;
                    healthLevels[] = {{1.01,{""}},{0.7,{""}},{0.5,{""}},{0.3,{""}},{0.01,{""}}};
                };
            };
        };
    };

    class M_UpgradeTo_Lvl2: Maintenance_UpgradeBase
    {
        scope = 2;
        displayName = "Upgrade Para: | Lvl 2 |";
        descriptionShort = "Aumenta a eficiência da Unidade de Manutenção.";
        weight = 200;
        inventorySlot[]= {"Upgrade1"};
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\upgradeKeys\data\DiskUp2_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 1000;
                    healthLevels[] = {{1.01,{""}},{0.7,{""}},{0.5,{""}},{0.3,{""}},{0.01,{""}}};
                };
            };
        };
    };

    class M_UpgradeTo_Lvl3: Maintenance_UpgradeBase
    {
        scope = 2;
        displayName = "Upgrade Para: | Lvl 3 |";
        descriptionShort = "Aumenta ainda mais a eficiência da Unidade de Manutenção.";
        weight = 300;
        inventorySlot[]= {"Upgrade2"};
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\upgradeKeys\data\DiskUp3_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 1000;
                    healthLevels[] = {{1.01,{""}},{0.7,{""}},{0.5,{""}},{0.3,{""}},{0.01,{""}}};
                };
            };
        };
    };
    
    
    class Clothing;
    class DogTag_Base: Clothing
    {
 		scope = 0;
        displayName = "Interface Tática";
        descriptionShort = "Dispositivo padrão de comunicações de elite. Todo operador confiável nasce com um. Se você não tem, ou não é confiável... ou não nasceu.";
        model = "MyMaintenanceMod\data\kdu\kdu_g.p3d";
        inventorySlot[] = {"Kdu_link"};
        canBeSplit=0;
        itemInfo[] = {"Clothing"};
 		simulation = "clothing";
		vehicleClass = "Clothing";   
        rotationFlags = 16;
        weight = 250;
        itemSize[] = {2,2};
        varWetMax = 0;
        absorbency = 0;
        hiddenSelections[] = {"camo","lcd"};
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\kdu\data\fcs_kdu_co.paa", "\MyMaintenanceMod\data\kdu\data\fcs_kdu_screen_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 100;
                    healthLevels[] = {
                        {1.0, {"MyMaintenanceMod\data\kdu\data\fcs_kdu.rvmat","MyMaintenanceMod\data\kdu\data\fcs_kdu.rvmat"}},
                        {0.7, {"MyMaintenanceMod\data\kdu\data\fcs_kdu.rvmat","MyMaintenanceMod\data\kdu\data\fcs_kdu.rvmat"}},
                        {0.5, {"MyMaintenanceMod\data\kdu\data\fcs_kdu_damage.rvmat","MyMaintenanceMod\data\kdu\data\fcs_kdu_damage.rvmat"}},
                        {0.3, {"MyMaintenanceMod\data\kdu\data\fcs_kdu_damage.rvmat","MyMaintenanceMod\data\kdu\data\fcs_kdu_damage.rvmat"}},
                        {0.0, {"MyMaintenanceMod\data\kdu\data\fcs_kdu_destruct.rvmat","MyMaintenanceMod\data\kdu\data\fcs_kdu_destruct.rvmat"}}
                    };
                };
            };
        };
        class ClothingTypes
        {
            male = "\MyMaintenanceMod\data\kdu\kdu.p3d";
            female = "\MyMaintenanceMod\data\kdu\kdu.p3d";
        };
        class AnimEvents
        {
            class SoundWeapon
            {
                class pickUpItem
                {
                    soundSet = "Shirt_pickup_SoundSet";
                    id = 797;
                };
                class drop
                {
                    soundset = "Shirt_drop_SoundSet";
                    id = 898;
                };
            };
        };
    };
    // A DogTag registrada, valiosa para o raid.
    class DogTag_Registered: DogTag_Base
    {
        scope=2;
        displayName="TypeZero Sync: ";
        descriptionShort="";
        model = "MyMaintenanceMod\data\kdu\kdu_g.p3d";
        inventorySlot[]={"Kdu_link", "JammerDogTag"}; // Só esta pode ir no Jammer.
    };

    // A DogTag de fresh spawn, sem valor para raid.
    class DogTag_Unregistered: DogTag_Base
    {
        scope=2;
        displayName="Sincronizando";
        descriptionShort="Aguardando sincronização neural.";
        inventorySlot[] = {"Kdu_link"};
    };

    //==============================================
    //== DogTags de Sobrevivência (Rastreio de Vida) ==
    //==============================================

    // Nível 1 (1 hora de vida)
    class DogTag_R_TypeOne: DogTag_Registered
    {
        scope=2;
        displayName="TypeOne Sync: ";
        descriptionShort="Esta identidade pertence a um sobrevivente experiente. Tempo de vida: 1 hora.";
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\kdu\data\fcs_kdu_co.paa", "\MyMaintenanceMod\data\kdu\data\fcs_kdu_screen_t1_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 200; // Vida aumentada
                };
            };
        };
    };

    // Nível 2 (2 horas de vida)
    class DogTag_R_TypeTwo: DogTag_Registered
    {
        scope=2;
        displayName="TypeTwo Sync: ";
        descriptionShort="Esta identidade pertence a um veterano endurecido. Tempo de vida: 2 horas.";
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\kdu\data\fcs_kdu_co.paa", "\MyMaintenanceMod\data\kdu\data\fcs_kdu_screen_t2_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 300;
                };
            };
        };
    };

    // Nível 3 (3 horas de vida)
    class DogTag_R_TypeThree: DogTag_Registered
    {
        scope=2;
        displayName="TypeThree Sync: ";
        descriptionShort="A identidade de um especialista em sobrevivência. Tempo de vida: 3 horas.";
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\kdu\data\fcs_kdu_co.paa", "\MyMaintenanceMod\data\kdu\data\fcs_kdu_screen_t3_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 400;
                };
            };
        };
    };

    // Nível 4 (4 horas de vida)
    class DogTag_R_TypeFour: DogTag_Registered
    {
        scope=2;
        displayName="TypeFour Sync: ";
        descriptionShort="A identidade de uma lenda viva. Tempo de vida: 4 horas.";
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\kdu\data\fcs_kdu_co.paa", "\MyMaintenanceMod\data\kdu\data\fcs_kdu_screen_t4_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 500;
                };
            };
        };
    };

    // Nível 5 (5 horas de vida)
    class DogTag_R_TypeFive: DogTag_Registered
    {
        scope=2;
        displayName="TypeFive Sync: ";
        descriptionShort="A identidade de alguém que se tornou parte desta terra. Quase um mito. Tempo de vida: 5 horas.";
        hiddenSelectionsTextures[]= {"\MyMaintenanceMod\data\kdu\data\fcs_kdu_co.paa", "\MyMaintenanceMod\data\kdu\data\fcs_kdu_screen_t5_co.paa"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 600;
                };
            };
        };
    };

    class Jammer: Container_Base
    {
        scope=2;
        displayName="Sinalizador Jammer";
        descriptionShort="Um dispositivo eletrônico que parece ser capaz de interferir com certos sinais quando uma fonte de identificação é fornecida.";
        model="\MyMaintenanceMod\data\jammer\Jammer.p3d";
        weight=350;
        itemSize[]={2,3};
        attachments[]={"JammerDogTag"};
		hiddenSelections[] = {"jammer_light"};
		hiddenSelectionsMaterials[] = {"MyMaintenanceMod\data\jammer\materials\jammer_light_red.rvmat"};
        class AnimationSources
        {
            class blade
            {
                source = "user";
                animPeriod = 0.01;
                initPhase = 1;
            };
            class blades_Rotation
            {
                source = "user";
                animPeriod = 1;
                initPhase = 0;
            };
            class antena
            {
                source = "user";
                animPeriod = 0.01;
                initPhase = 1;
            };
        };
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 500; // Vida total do item.
                    transferToAttachmentsCoef = 0.10;
                    healthLevels[] = 
                    {
                    {1.0, {}}, // Pristine
                    {0.7, {}}, // Worn
                    {0.5, {}}, // Damaged
                    {0.3, {}}, // Badly Damaged
                    {0.0, {}}  // Ruined
                    };
                };
            };
        };
    };

    //==============================================
    //============ Event Beacon ====================
    //==============================================
    class EventBeacon: Inventory_Base
    {
        scope = 2;
        displayName = "Event Beacon";
        descriptionShort = "Beacon invisível para detectar eventos militares.";
        model = "\MyMaintenanceMod\data\Askal_Debug.p3d"; // Modelo temporário - será invisível
        weight = 0;
        itemSize[] = {1,1};
        canBeDigged = 0;
        varQuantityInit = 0;
        varQuantityMin = 0;
        varQuantityMax = 0;
        absorbency = 0;
        stackedUnit = "";
        quantityBar = 0;
        varQuantityDestroyOnMin = 0;
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 1;
                    healthLevels[] = {{1.0,{"",{},{}}},{0.7,{"",{},{}}},{0.5,{"",{},{}}},{0.3,{"",{},{}}},{0.0,{"",{},{}}}};
                };
            };
        };
    };
};
class CfgNonAIVehicles
{
	class ProxyKdu_link
	{
		scope = 2;
		simulation = "ProxyInventory";
		inventorySlot = "Kdu_link";
		autocenter = 0;
		animated = 0;
		model = "";
		shadow = 1;
		reversed = 1;
	};
	class ProxyProxyKdu_linkDZ: ProxyKdu_link
	{
		model = "\MyMaintenanceMod\data\character\kdu_link.p3d";
	};
};

class RscMapControl
{
	scaleMin = 0.05;
	scaleMax = 0.85;
	scaleDefault = 0.85;
	ptsPerSquareSea = 8;
	ptsPerSquareTxt = 10;
	ptsPerSquareCLn = 10;
	ptsPerSquareExp = 10;
	ptsPerSquareCost = 10;
	ptsPerSquareFor = 99;
	ptsPerSquareForEdge = 99;
	ptsPerSquareRoad = 4;
	ptsPerSquareObj = 15;
	maxSatelliteAlpha = 1;
	alphaFadeStartScale = 1;
	alphaFadeEndScale = 1;
	userMapPath = "dz\gear\navigation\data\usermap";
	maxUserMapAlpha = 0.2;
	alphaUserMapFadeStartScale = 0.5;
	alphaUserMapFadeEndScale = 0.8;
	showCountourInterval = 1;
	colorLevels[] = {0.65,0.6,0.45,0.3};
	colorSea[] = {0.2,0.5,0.7,1};
	colorForest[] = {0.36,0.78,0.08,0};
	colorRocks[] = {0.5,0.5,0.5,0.2};
	colorCountlines[] = {0.85,0.8,0.65,0.1};
	colorMainCountlines[] = {0.45,0.4,0.25,0};
	colorCountlinesWater[] = {0.25,0.4,0.5,0.3};
	colorMainCountlinesWater[] = {0.25,0.4,0.5,0.9};
	colorPowerLines[] = {0.1,0.1,0.1,1};
	colorRailWay[] = {0.8,0.2,0,1};
	colorForestBorder[] = {0.4,0.8,0,0};
	colorRocksBorder[] = {0.5,0.5,0.5,0};
	colorOutside[] = {1,1,1,1};
	colorTracks[] = {0.78,0.66,0.34,1};
	colorRoads[] = {0.69,0.43,0.23,1};
	colorMainRoads[] = {0.53,0.35,0,1};
	colorTracksFill[] = {0.96,0.91,0.6,1};
	colorRoadsFill[] = {0.92,0.73,0.41,1};
	colorMainRoadsFill[] = {0.84,0.61,0.21,1};
	colorGrid[] = {0,0,0,0};
	colorGridMap[] = {0,0,0,0};
	fontNames = "gui/fonts/sdf_MetronBook24";
	sizeExNames = 0.03;
	colorNames[] = {1,1,1,1};
	fontGrid = "gui/fonts/sdf_MetronBook24";
	sizeExGrid = 0.02;
	fontLevel = "gui/fonts/sdf_MetronBook24";
	sizeExLevel = 0.01;
	colorMountPoint[] = {0.45,0.4,0.25,0};
	mapPointDensity = 0.12;
	text = "";
	fontLabel = "gui/fonts/sdf_MetronBook24";
	fontInfo = "gui/fonts/sdf_MetronBook24";
	class Legend
	{
		x = 0.05;
		y = 0.85;
		w = 0.4;
		h = 0.1;
		font = "gui/fonts/sdf_MetronBook24";
		sizeEx = 0.02;
		colorBackground[] = {1,1,1,0};
		color[] = {0,0,0,0};
	};
};
