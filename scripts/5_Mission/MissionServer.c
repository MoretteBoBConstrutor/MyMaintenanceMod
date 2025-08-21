modded class MissionServer
{
    // OnInit é o ponto de entrada padrão e mais seguro para inicializar os sistemas do mod.
    override void OnInit()
    {
        super.OnInit();
        GetRPCManager().AddRPC("MyMaintenanceMod", "RequestMaintenanceStatus", this, SingeplayerExecutionType.Server);
        GetRPCManager().AddRPC("MyMaintenanceMod", "SyncHeliCrashes", this, SingeplayerExecutionType.Server);
        GetRPCManager().AddRPC("MyMaintenanceMod", "SyncMilitaryEvents", this, SingeplayerExecutionType.Server);
        Print("[MissionServer] OnInit — inicializando sistemas do mod");

        // Garante que o singleton do ConfigManager seja criado e o arquivo de config seja carregado.
        // Fazemos isso primeiro, pois outros sistemas podem depender dele.
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(MaintenanceConfigManager.GetInstance);
        MMLogger.Log("Config Manager inicializado.");

        // Carrega o banco de dados de unidades.
        MaintenanceUnitManager.GetInstance().LoadData();
        MMLogger.Log("Unit Database Manager inicializado.");

        // Carrega o banco de dados de jogadores para o Rastreio de Vida.
        MMS_PlayerDatabaseManager.GetInstance().LoadData();
        MMLogger.Log("Player Database Manager inicializado.");

        // Carrega dados persistidos de HeliCrashes
        HeliCrashTracker.LoadPersistedData();
        MMLogger.Log("HeliCrash Database Manager inicializado.");

        // Carrega dados persistidos de eventos militares
        MilitaryEventTracker.LoadPersistedData();
        MMLogger.Log("Military Event Database Manager inicializado.");

        // Inicializa o EventBeaconConfig
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(EventBeaconConfig.GetInstance, 100, false);
        Print("[MissionServer] EventBeacon Config Manager inicializado.");

        // Inicializa o MilitaryEventTracker
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MilitaryEventTracker.GetInstance, 100, false);
        Print("[MissionServer] Military Event Tracker inicializado.");

        // Inicia o timer de limpeza de DogTags
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CleanupDogTags, 300000, true); // 300000 ms = 5 minutos
        
        // Primeira validação após 5 minutos (sem repetição), depois agenda validações a cada 10 minutos
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FirstValidation, 300000, false); // 300000 ms = 5 minutos, sem repetir
    }

    void RequestMaintenanceStatus(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        if (type == CallType.Server)
        {
            Param1<MaintenanceUnit> data;
            if (!ctx.Read(data))
                return;

            MaintenanceUnit unit = data.param1;
            if (!unit || !sender)
                return;

            // Criamos o nosso pacote de dados
            MaintenanceStatusMenuData menuData = new MaintenanceStatusMenuData();

            // --- Reutilizamos a lógica da sua função ShowStatusTo para preencher o pacote ---
            menuData.m_Level = unit.GetCurrentLevel().ToString();

            // Dono e Membros - Busca os nomes em vez de apenas enviar os IDs
            if (unit.HasOwner())
            {
                // Pega a lista de jogadores online UMA VEZ para otimizar a busca.
                array<Man> allPlayers = new array<Man>();
                GetGame().GetPlayers(allPlayers);
                bool ownerFound = false;

                // Procura pelo nome do Dono
                string ownerID = unit.GetOwnerID();
                foreach (Man p : allPlayers)
                {
                    PlayerBase onlinePlayer = PlayerBase.Cast(p);
                    if (onlinePlayer && onlinePlayer.GetIdentity() && onlinePlayer.GetIdentity().GetPlainId() == ownerID)
                    {
                        menuData.m_Owner = onlinePlayer.GetIdentity().GetName();
                        ownerFound = true;
                        break;
                    }
                }
                if (!ownerFound)
                {
                    menuData.m_Owner = ownerID + " (Offline)";
                }

                // Constrói a lista de Membros com nomes
                array<string> memberIDs = unit.GetMembers();
                if (memberIDs)
                {
                    foreach (string memberID : memberIDs)
                    {
                        bool memberFound = false;
                        foreach (Man playerOnline : allPlayers)
                        {
                            PlayerBase pBase = PlayerBase.Cast(playerOnline);
                            if (pBase && pBase.GetIdentity() && pBase.GetIdentity().GetPlainId() == memberID)
                            {
                                menuData.m_Members.Insert(pBase.GetIdentity().GetName());
                                memberFound = true;
                                break;
                            }
                        }
                        if (!memberFound)
                        {
                            menuData.m_Members.Insert(memberID + " (Offline)");
                        }
                    }
                }
            }

            // Limites de Construção
            int currentContainerCount, currentBaseBuildingCount;
            unit.CountDeployablesInRadius(unit.GetPosition(), MaintenanceConfigManager.GetInstance().GetMaintenanceRadius(), currentContainerCount, currentBaseBuildingCount);
            int maxContainers = MaintenanceConfigManager.GetInstance().GetMaxContainersForLevel(unit.GetCurrentLevel());
            int maxBaseBuildingParts = MaintenanceConfigManager.GetInstance().GetMaxBaseBuildingForLevel(unit.GetCurrentLevel());
            menuData.m_ConstructionLimit = string.Format("%1/%2", currentBaseBuildingCount, maxBaseBuildingParts);
            menuData.m_ContainerLimit = string.Format("%1/%2", currentContainerCount, maxContainers);

            // Status e Autonomia
            if (unit.HasEnoughResources())
            {
                menuData.m_Status = "Ativo";
                
                // Lógica de cálculo de tempo copiada de ShowStatusTo
                int totalResources = unit.GetResourceCount();
                int resourcesPerCycle = MaintenanceConfigManager.GetInstance().GetResourcesPerCycle();
                int remainingCycles = 0;
                if (resourcesPerCycle > 0)
                    remainingCycles = totalResources / resourcesPerCycle;
                
                float totalTimeRemainingSec = remainingCycles * MaintenanceConfigManager.GetInstance().GetCheckInterval();
                int totalTimeRemainingMin = Math.Ceil(totalTimeRemainingSec / 60.0);

                string timeString;
                if (totalTimeRemainingMin >= 1440) // Mais de 24 horas
                {
                    int days = totalTimeRemainingMin / 1440;
                    int hours = (totalTimeRemainingMin - (days * 1440)) / 60;
                    timeString = days.ToString() + "d " + hours.ToString() + "h";
                }
                else if (totalTimeRemainingMin > 120) // Mais de 2 horas
                {
                    timeString = Math.Ceil(totalTimeRemainingMin / 60.0).ToString() + " h";
                }
                else
                {
                    timeString = totalTimeRemainingMin.ToString() + " m";
                }
                menuData.m_TimeLeft = timeString;
            }
            else
            {
                menuData.m_Status = "Inativo";
                menuData.m_TimeLeft = "N/A";
            }

            // Enviamos o pacote de dados de volta para o cliente que pediu
            GetRPCManager().SendRPC("MyMaintenanceMod", "OpenMaintenanceStatusMenu", new Param1<MaintenanceStatusMenuData>(menuData), true, sender);
            Print("[MissionServer] RequestMaintenanceStatus — status de manutenção enviado para " + sender.GetName());
        }
    }

    void SyncHeliCrashes(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        if (type == CallType.Server)
        {
            Print("[MissionServer] SyncHeliCrashes solicitado por " + sender.GetName());
            
            // Cria dados de crashes para enviar ao cliente
            HeliCrashMapData crashData = new HeliCrashMapData();
            
            // Obtém crashes ativos + persistidos do servidor
            array<vector> allCrashPositions = HeliCrashTracker.GetCrashPositions();
            if (allCrashPositions)
            {
                Print("[MissionServer] HeliCrashTracker retornou " + allCrashPositions.Count() + " posições");
                foreach (vector crashPos : allCrashPositions)
                {
                    crashData.AddCrashPosition(crashPos);
                }
            }
            else
            {
                Print("[MissionServer] HeliCrashTracker retornou array nulo ou vazio");
            }
            
            // Envia dados para o cliente
            GetRPCManager().SendRPC("MyMaintenanceMod", "ReceiveHeliCrashes", new Param1<HeliCrashMapData>(crashData), true, sender);
            Print("[MissionServer] SyncHeliCrashes — " + crashData.GetCrashCount() + " crashes (incluindo persistidos) enviados para " + sender.GetName());
        }
    }

    void CleanupDogTags()
    {
        // Copiamos a array para evitar problemas se a original for modificada durante a iteração (quando tag.Delete() é chamado).
        array<DogTag_Base> dogTagsToCheck = new array<DogTag_Base>;
        dogTagsToCheck.Copy(DogTag_Base.s_AllDogTags);

        int deleted = 0;
        foreach (DogTag_Base tag : dogTagsToCheck)
        {
            // A verificação 'tag != null' é implícita, pois a lista só contém DogTags.
            if (tag.IsExpired() && !tag.IsProtectedAttachment())
            {
                tag.Delete();
                deleted++;
            }
        }
        if (deleted > 0)
            MMLogger.Log("DogTags expiradas removidas: " + deleted);
    }

    /**
     * @brief Primeira validação após 5 minutos - executa uma vez e agenda validações recorrentes
     */
    void FirstValidation()
    {
        Print("[MissionServer] FirstValidation - executando primeira validação após inicialização");
        
        // Executa a primeira validação
        ValidateCrashes();
        
        // Agenda validações recorrentes a cada 10 minutos (600000 ms)
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ValidateCrashes, 600000, true);
        Print("[MissionServer] FirstValidation - validações recorrentes agendadas para cada 10 minutos");
    }

    /**
     * @brief Valida crashes persistidos periodicamente
     * Reimplementado com detecção correta e raio de 100m para teste
     */
    void ValidateCrashes()
    {
        if (!GetGame().IsServer()) return;
        
        Print("[MissionServer] ValidateCrashes iniciado - verificando crashes persistidos");
        HeliCrashTracker.ValidatePersistedCrashes();
        Print("[MissionServer] ValidateCrashes concluído");
        
        // Valida também eventos militares
        Print("[MissionServer] ValidateEvents iniciado - verificando eventos militares");
        MilitaryEventTracker.ValidateEvents();
        Print("[MissionServer] ValidateEvents concluído");
    }

#ifdef EXPANSIONMODAI
    /**
     * @brief Função auxiliar para obter a facção do jogador.
     * Esta lógica é baseada no sistema do Expansion AI e só será compilada se EXPANSIONMODAI estiver definido.
     * Ela assume que as classes 'eAIGroup' e 'eAIFaction' estão disponíveis através do mod de facções.
     */
    eAIFaction GetPlayerFaction(PlayerBase player)
    {
        if (!player)
            return null;

        eAIGroup group = player.GetGroup();
        if (!group)
            return null;

        return group.GetFaction();
    }

    /**
     * @brief Função com nome único para evitar conflitos, responsável por atualizar a DogTag com a facção do jogador.
     */
    void MDT_UpdateFactionOnDogTag(PlayerBase player)
    {
        if (!player)
            return;

        // Reutiliza a função existente para obter a tag do jogador de forma consistente.
        DogTag_Base dogtag = player.GetDogTag();
        if (!dogtag)
            return;

        eAIFaction faction = GetPlayerFaction(player);
        if (faction)
        {
            string factionName = faction.GetName();
            dogtag.SetFactionName(factionName);
            MMLogger.Log("[MDT Faction Sync] Faction '" + factionName + "' set for player " + player.GetIdentity().GetName());
        }
    }
#endif

    // Chamado para cada jogador que se conecta ao servidor.
    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);

        // PASSO 1: Garante que o jogador receba uma DogTag ao conectar.
        EquipPlayerDogTag(player);

        DogTag_Base tag = player.GetDogTag();
        if (tag)
        {
            // Se a tag for registrada, atualiza o nome caso o jogador tenha mudado na Steam.
            if (tag.IsKindOf("DogTag_Registered"))
            {
                tag.SetNickname(identity.GetName());
            }
            // Se a tag for não registrada, inicia o timer de upgrade.
            else if (tag.IsKindOf("DogTag_Unregistered"))
                player.StartDogTagUpgradeTimer();
        }

        // Define o status de equipe (dono OU membro) do jogador ao conectar, garantindo que a UI esteja correta.
        string playerID = identity.GetPlainId();
        // Usamos a verificação abrangente para garantir que tanto donos quanto membros tenham seu status sincronizado.
        if (MaintenanceUnitManager.GetInstance().IsPlayerInAnyTeam(playerID))
        {
            player.SetHasMaintenanceUnit(true);
        }

        // --- NOVO: Lógica de Rastreio de Vida ao conectar ---
        if (GetGame().IsServer())
        {
            // A fonte de verdade mais confiável é a tag que o jogador está usando.
            // Se a tag NÃO for do tipo "Unregistered", significa que o jogador está
            // na trilha de sobrevivência e seu timer de contagem deve ser iniciado/continuado.
            DogTag_Base currentTag = player.GetDogTag();
            if (currentTag && !currentTag.IsKindOf("DogTag_Unregistered"))
            {
                // --- INÍCIO DA LÓGICA DE AUTOCORREÇÃO ---
                MMS_PlayerData pData = MMS_PlayerDatabaseManager.GetInstance().GetOrCreatePlayerData(playerID);
                int databaseLevel = pData.m_SurvivalLevel;
                int itemLevel = -1;

                // Mapeia o tipo da tag para um nível numérico. A ordem é do maior para o menor.
                if (currentTag.IsKindOf("DogTag_R_TypeFive"))
                    itemLevel = 5;
                else if (currentTag.IsKindOf("DogTag_R_TypeFour"))
                    itemLevel = 4;
                else if (currentTag.IsKindOf("DogTag_R_TypeThree"))
                    itemLevel = 3;
                else if (currentTag.IsKindOf("DogTag_R_TypeTwo"))
                    itemLevel = 2;
                else if (currentTag.IsKindOf("DogTag_R_TypeOne"))
                    itemLevel = 1;
                else if (currentTag.IsKindOf("DogTag_Registered"))
                    itemLevel = 0;

                // Se o nível do item foi identificado e é diferente do nível no banco de dados, corrige.
                if (itemLevel != -1 && itemLevel != databaseLevel)
                {
                    MMLogger.Log("[Survival][Auto-Correction] Inconsistência encontrada para " + identity.GetName() + ". Nível do item: " + itemLevel + ", Nível do DB: " + databaseLevel + ". Corrigindo DB.");
                    pData.m_SurvivalLevel = itemLevel; // Corrige o banco de dados para refletir o item real.
                }
                // --- FIM DA LÓGICA DE AUTOCORREÇÃO ---

                player.StartSurvivalTracking();
            }
        }

#ifdef EXPANSIONMODAI
        // Após a lógica padrão, chama nossa função customizada para sincronizar a facção com a DogTag.
        // Usamos CallLater para dar um pequeno tempo para o jogador e seus anexos serem totalmente inicializados, evitando race conditions.
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.MDT_UpdateFactionOnDogTag, 500, false, player);
#endif
    }

    override void EquipCharacter(MenuDefaultCharacterData char_data)
    {
        EquipPlayerDogTag(m_player);
        super.EquipCharacter(char_data);
    }

    // Função centralizada para criar a DogTag, inspirada no mod da Steam.
    protected void EquipPlayerDogTag(PlayerBase player)
    {
        if (!player)
            return;

        // Se o jogador não tiver nenhuma DogTag, cria uma não registrada e inicia o timer.
        if (!player.GetDogTag())
        {
            MMLogger.Log("Jogador " + player.GetIdentity().GetName() + " não tem DogTag. Criando uma nova.");
            player.GivePlayerDogTag();        // Chama a função que você já criou em PlayerBase.
            player.StartDogTagUpgradeTimer(); // Inicia o timer para o upgrade
        }
    }

    override void OnMissionFinish()
    {
        super.OnMissionFinish();

        // Salva ambos os sistemas ao desligar o servidor.
        MMLogger.Log("Missão finalizada, salvando dados...");
        MaintenanceUnitManager.GetInstance().SaveData();
        MMS_PlayerDatabaseManager.GetInstance().SaveData();
        HeliCrashDatabaseManager.GetInstance().SaveData();
        MilitaryEventDatabaseManager.GetInstance().SaveData();
    }

    // Método RPC para sincronizar eventos militares com clientes
    void SyncMilitaryEvents(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        if (type == CallType.Server)
        {
            if (!sender)
                return;

            PlayerBase player = PlayerBase.Cast(sender.GetPlayer());
            if (!player)
                return;

            // Valida eventos antes de sincronizar (remove órfãos)
            MilitaryEventTracker.ValidateEvents();

            // Obter eventos militares do tracker
            array<vector> eventPositions = MilitaryEventTracker.GetEventPositions();
            
            Print("[MissionServer] SyncMilitaryEvents solicitado por " + sender.GetName());
            Print("[MissionServer] MilitaryEventTracker retornou " + eventPositions.Count() + " posições");

            // Enviar dados para o cliente
            GetRPCManager().SendRPC("MyMaintenanceMod", "ReceiveMilitaryEvents", new Param1<ref array<vector>>(eventPositions), true, sender);
            Print("[MissionServer] SyncMilitaryEvents — " + eventPositions.Count() + " eventos militares enviados para " + sender.GetName());
        }
    }

    // Chamado para cada jogador que se desconecta do servidor.
    override void InvokeOnDisconnect(PlayerBase player)
    {
        super.InvokeOnDisconnect(player);

        // Salva o banco de dados de jogadores para garantir que o tempo de sobrevivência
        // de quem deslogou seja persistido.
        if (GetGame().IsServer())
        {
            MMS_PlayerDatabaseManager.GetInstance().SaveData();
        }
    }
}
