class MaintenanceUnit : Container_Base
{
    private static ref array<MaintenanceUnit> m_AllUnits; // Lista estática de todas as unidades no servidor

    // ID único desta instância, carregado do nosso banco de dados JSON.
    private int m_UnitID;

    // Nível atual da unidade, determinado pelos attachments.
    private int m_CurrentLevel;

    float lastMaintenanceCheckTime = 0;
    // Agora usamos int para sincronização precisa do SteamID
    protected int m_OwnerID_Part1; 
    protected int m_OwnerID_Part2;
    // Cache para o ID reconstruído no cliente e servidor, garantindo consistência.
    private string m_CachedOwnerID; 
    // Membros da unidade (apenas no servidor)
    private ref array<string> m_Members;

    void MaintenanceUnit()
    {
        m_UnitID = -1; // ID inválido até ser carregado.
        m_CurrentLevel = 1; // Começa no nível 1 por padrão.
        m_OwnerID_Part1 = 0;
        m_OwnerID_Part2 = 0;
        m_CachedOwnerID = ""; // Inicializa o cache

        m_Members = new array<string>; // Inicializa a lista de membros
        // Inicializa a lista estática apenas no servidor, se ainda não existir.
        if (GetGame().IsServer() && !m_AllUnits)
            m_AllUnits = new array<MaintenanceUnit>;

        // Registra as duas partes do ID para serem sincronizadas automaticamente pela rede
        RegisterNetSyncVariableInt("m_OwnerID_Part1");
        RegisterNetSyncVariableInt("m_OwnerID_Part2");
    }

    // Inicializa timers para manutenção e atualização visual da luz
    override void EEInit()
    {
        super.EEInit();

        // Garante que o nível seja calculado assim que a unidade é carregada.
        UpdateLevelFromAttachments();

        if (GetGame().IsServer())
        {
            // Tenta encontrar seus dados no banco de dados do gerenciador.
            MaintenanceUnitData data = MaintenanceUnitManager.GetInstance().FindUnitDataByPos(GetPosition());
            if (data)
            {
                // Sucesso! Carrega os dados.
                m_UnitID = data.m_UnitID;
                SetOwnerID(data.m_OwnerSteamID); // Define o dono e sincroniza com clientes.
                LoadMembers(data.m_Members);
                LogActivity("EEInit: Unidade inicializada com ID: " + m_UnitID);

                // VERIFICAÇÃO CRÍTICA: Se a unidade estiver arruinada, não inicia os timers.
                if (IsRuined())
                {
                    LogActivity("EEInit: Unidade " + m_UnitID + " carregada em estado arruinado. Timers não serão iniciados.");
                    return; // Impede que a unidade "zumbi" se torne ativa.
                }

                m_AllUnits.Insert(this);
                // Agenda o ciclo de manutenção
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CheckMaintenance, MaintenanceConfigManager.GetInstance().GetCheckInterval() * 1000, true);
                // Agenda a atualização periódica de estados (luz e nível)
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.PeriodicUpdate, 5000, true);
            }
            else
            {
                // Esta unidade não existe no nosso banco de dados (é um "fantasma").
                // Isso pode acontecer se o arquivo de storage do servidor for restaurado sem o nosso JSON.
                // A melhor abordagem é removê-la para evitar inconsistências.
                LogActivity("EEInit: Unidade fantasma encontrada em " + GetPosition() + ". Removendo.");
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Delete, 1000, false);
                return;
            }
        }
    }

    // Chamado em todos os clientes quando as NetSyncVariables são atualizadas pelo servidor.
    // Esta é a forma mais robusta de garantir que o cliente tenha o ID mais recente.
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();

        // Reconstrói o ID a partir das partes sincronizadas e o armazena no cache.
        if (m_OwnerID_Part1 == 0)
        {
            m_CachedOwnerID = "";
        }
        else
        {
            string part1_str = string.Format("%1", m_OwnerID_Part1);
            string part2_str = string.Format("%1", m_OwnerID_Part2);

            // Garante que a segunda parte tenha 9 dígitos, preenchendo com zeros à esquerda.
            while (part2_str.Length() < 9)
            {
                part2_str = "0" + part2_str;
            }
            m_CachedOwnerID = part1_str + part2_str;
        }
    }
    
    override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);

		if (GetGame().IsServer())
		{
            // Remove da lista estática em memória no servidor.
            if (m_AllUnits)
            {
                int index = m_AllUnits.Find(this);
                if (index != -1)
                {
                    m_AllUnits.Remove(index);
                }
            }

            string ownerID = m_CachedOwnerID;

            // Remove do banco de dados JSON para garantir a persistência da remoção
			if (m_UnitID != -1)
			{
				MaintenanceUnitManager.GetInstance().RemoveUnit(m_UnitID);
			}

            // Se havia um dono, encontra o jogador (se online) e atualiza seu status.
            if (ownerID != "")
            {
                array<Man> players = new array<Man>();
                GetGame().GetPlayers(players);
                foreach(Man p : players)
                {
                    PlayerBase player = PlayerBase.Cast(p);
                    if (player && player.GetIdentity() && player.GetIdentity().GetPlainId() == ownerID)
                    {
                        player.SetHasMaintenanceUnit(false);
                        break; // Encontrou o jogador, para o loop.
                    }
                }
            }

            // --- NOVO: Atualiza o status de todos os membros ---
            if (m_Members && m_Members.Count() > 0)
            {
                array<Man> allPlayers = new array<Man>();
                GetGame().GetPlayers(allPlayers);
                foreach (string memberID : m_Members)
                {
                    foreach(Man pp : allPlayers)
                    {
                        PlayerBase memberPlayer = PlayerBase.Cast(pp);
                        if (memberPlayer && memberPlayer.GetIdentity() && memberPlayer.GetIdentity().GetPlainId() == memberID)
                        {
                            memberPlayer.SetHasMaintenanceUnit(false);
                            break; // Encontrou este membro, vai para o próximo ID
                        }
                    }
                }
            }
		}
	}

    int GetUnitID()
    {
        return m_UnitID;
    }

    int GetCurrentLevel()
    {
        return m_CurrentLevel;
    }

    static array<MaintenanceUnit> GetAllUnits()
    {
        if (!m_AllUnits)
            m_AllUnits = new array<MaintenanceUnit>;
            
        return m_AllUnits;
    }

    // Função interna para definir o ID do proprietário a partir de uma string
    private void SetOwnerID(string id)
    {
        if (id == "")
        {
            m_OwnerID_Part1 = 0;
            m_OwnerID_Part2 = 0;
        }
        else if (id.Length() == 17) // Garante que é um Steam64ID válido
        {
            string part1_str = id.Substring(0, 8);
            string part2_str = id.Substring(8, 9);
            int part1 = part1_str.ToInt();
            int part2 = part2_str.ToInt();
            m_OwnerID_Part1 = part1;
            m_OwnerID_Part2 = part2;
        }
        // Atualiza o cache no servidor imediatamente.
        m_CachedOwnerID = id;

        // Se a unidade já tem um ID válido, atualiza o banco de dados.
        if (m_UnitID != -1)
            MaintenanceUnitManager.GetInstance().UpdateOwner(m_UnitID, id);

        // Marca a entidade como 'suja' para que as NetSyncVariables sejam sincronizadas com os clientes.
        SetSynchDirty();
    }

    // Define o proprietário da unidade
    void SetOwner(PlayerBase player)
    {
        // Esta lógica deve rodar apenas no servidor
        if (!GetGame().IsServer()) return;

        // Verificação de robustez no servidor: A ação só deve ser possível em unidades sem dono.
        // A ActionCondition já faz isso no cliente, mas uma checagem no servidor previne exploits.
        if (HasOwner()) 
        {
            LogActivity("Tentativa de reivindicar unidade que já possui dono: " + GetOwnerID() + " por " + player.GetIdentity().GetPlainId());
            return;
        }

        // NOVA VERIFICAÇÃO ABRANGENTE: Impede que um jogador que já é dono OU membro de uma equipe reivindique outra unidade.
        if (MaintenanceUnitManager.GetInstance().IsPlayerInAnyTeam(player.GetIdentity().GetPlainId()))
        {
            LogActivity("Tentativa de reivindicar por " + player.GetIdentity().GetPlainId() + ", que já pertence a uma equipe.");
            // Envia uma notificação para o jogador explicando o motivo da falha.
            NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Ação Falhou", "Você já faz parte de uma equipe de manutenção.", "set:dayz_gui image:icon_cancel");
            return;
        }

        if (player && player.GetIdentity())
        {
            // Usar GetPlainId() para obter o Steam64ID (17 dígitos), que é compatível com a nossa lógica de salvamento.
            string newOwnerId = player.GetIdentity().GetPlainId();
            SetOwnerID(newOwnerId);
            player.SetHasMaintenanceUnit(true);
            LogActivity("Propriedade da unidade definida para: " + player.GetIdentity().GetName() + " (Steam64ID: " + newOwnerId + ")");
        }
        else
        {
            LogActivity("ERRO: SetOwner chamado, mas o 'player' ou sua 'identity' é nula!");
        }
    }

    // Método público para previsão no lado do cliente, para atualizar a UI instantaneamente.
    void Client_SetCachedOwnerID(string id)
    {
        // A verificação de runtime é uma boa prática, mas a diretiva de pré-processador é essencial.
        if (!GetGame().IsClient())
            return;

        m_CachedOwnerID = id;

#ifdef CLIENT
        // Este bloco de código SÓ EXISTE na versão do cliente do jogo.
        // O compilador do servidor o ignora completamente, resolvendo o erro de compilação.
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player) 
        {
            ActionManagerClient am = ActionManagerClient.Cast(player.GetActionManager());
            if (am)
                am.UpdateActions();
        }
#endif
    }

    // Remove o proprietário da unidade
    void AbandonOwnership(PlayerBase player)
    {
        // Esta lógica deve rodar apenas no servidor
        if (!GetGame().IsServer())
            return;

        // Apenas o dono atual pode abandonar a propriedade
        if (HasOwner() && GetOwnerID() == player.GetIdentity().GetPlainId()) // A comparação também deve usar o Steam64ID
        {
            string formerOwnerID = GetOwnerID();

            // --- NOVO: Notifica todos os membros online antes de removê-los ---
            array<Man> allPlayers = new array<Man>();
            GetGame().GetPlayers(allPlayers);
            foreach (string memberID : m_Members)
            {
                foreach(Man p : allPlayers)
                {
                    PlayerBase memberPlayer = PlayerBase.Cast(p);
                    if (memberPlayer && memberPlayer.GetIdentity() && memberPlayer.GetIdentity().GetPlainId() == memberID)
                    {
                        memberPlayer.SetHasMaintenanceUnit(false);
                        break; 
                    }
                }
            }

            SetOwnerID(""); // Limpa o ID

            m_Members.Clear();
            MaintenanceUnitManager.GetInstance().UpdateMembers(m_UnitID, m_Members);
            player.SetHasMaintenanceUnit(false);
            LogActivity("Propriedade da unidade abandonada por: " + player.GetIdentity().GetName() + " (Steam64ID: " + player.GetIdentity().GetPlainId() + ")");
        }
    }

    // Retorna o ID do proprietário
    string GetOwnerID()
    {
        return m_CachedOwnerID;
    }

    // Verifica se a unidade tem um proprietário
    bool HasOwner()
    {
        return m_CachedOwnerID != "";
    }

    // --- Lógica de Membros ---

    /**
     * @brief Carrega a lista de membros a partir do banco de dados.
     * @param members A lista de IDs de membros vinda do JSON.
     */
    void LoadMembers(array<string> members)
    {
        if (GetGame().IsServer() && members)
        {
            m_Members = members;
            LogActivity("Carregados " + m_Members.Count() + " membros para a unidade " + m_UnitID);
        }
    }

    /**
     * @brief Adiciona um novo membro à unidade. Apenas no servidor.
     * @param memberID O Steam64ID do jogador a ser adicionado.
     */
    void AddMember(string memberID)
    {
        if (!GetGame().IsServer() || memberID == "") return;

        if (m_Members.Find(memberID) == -1) // Evita duplicatas
        {
            m_Members.Insert(memberID);
            MaintenanceUnitManager.GetInstance().UpdateMembers(m_UnitID, m_Members);
            LogActivity("Membro " + memberID + " adicionado à unidade " + m_UnitID);

            // --- NOVO: Atualiza o status do jogador adicionado ---
            array<Man> players = new array<Man>();
            GetGame().GetPlayers(players);
            foreach(Man p : players)
            {
                PlayerBase player = PlayerBase.Cast(p);
                if (player && player.GetIdentity() && player.GetIdentity().GetPlainId() == memberID)
                {
                    player.SetHasMaintenanceUnit(true);
                    LogActivity("Status 'HasMaintenanceUnit' definido como TRUE para o novo membro: " + memberID);
                    break;
                }
            }
        }
    }

    /**
     * @brief Remove um membro da unidade. Apenas no servidor.
     * @param memberID O Steam64ID do jogador a ser removido.
     */
    void RemoveMember(string memberID)
    {
        if (!GetGame().IsServer() || memberID == "") return;

        int index = m_Members.Find(memberID);
        if (index != -1)
        {
            m_Members.Remove(index);
            MaintenanceUnitManager.GetInstance().UpdateMembers(m_UnitID, m_Members);
            LogActivity("Membro " + memberID + " removido da unidade " + m_UnitID);

            // --- NOVO: Atualiza o status do jogador removido ---
            array<Man> players = new array<Man>();
            GetGame().GetPlayers(players);
            foreach(Man p : players)
            {
                PlayerBase player = PlayerBase.Cast(p);
                if (player && player.GetIdentity() && player.GetIdentity().GetPlainId() == memberID)
                {
                    player.SetHasMaintenanceUnit(false);
                    LogActivity("Status 'HasMaintenanceUnit' definido como FALSE para o membro removido: " + memberID);
                    break;
                }
            }
        }
    }

    /**
     * @brief Retorna a lista de membros. Apenas no servidor.
     * @return A lista de IDs dos membros.
     */
    array<string> GetMembers()
    {
        return m_Members;
    }

    /**
     * @brief Verifica se um jogador é membro desta unidade.
     * @param playerID O Steam64ID do jogador.
     * @return true se for membro, false caso contrário.
     */
    bool IsMember(string playerID)
    {
        if (playerID == "") return false;
        return m_Members.Find(playerID) != -1;
    }

    /**
     * @brief Verifica se um jogador é o proprietário OU um membro.
     * @param playerID O Steam64ID do jogador.
     * @return true se for dono ou membro, false caso contrário.
     */
    bool IsMemberOrOwner(string playerID)
    {
        if (playerID == "") return false;
        return GetOwnerID() == playerID || IsMember(playerID);
    }

    /**
     * @brief Verifica se há um Jammer ativo dentro de um raio de proximidade,
     * impedindo ações de gerenciamento de propriedade.
     * @return true se um Jammer ativo for encontrado, false caso contrário.
     */
    bool IsJammed()
    {
        // O raio de bloqueio do Jammer agora usa o mesmo raio da área de manutenção, definido no config.json.
        float blockRadius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();

        vector position = GetPosition();
        array<Object> nearbyObjects = new array<Object>;
        
        GetGame().GetObjectsAtPosition3D(position, blockRadius, nearbyObjects, null);

        for (int i = 0; i < nearbyObjects.Count(); i++)
        {
            Jammer jammer = Jammer.Cast(nearbyObjects.Get(i));
            
            // Se o objeto for um Jammer e estiver ativo, a área está "jammada".
            //if (jammer && jammer.IsActive())
            if (jammer && jammer.IsActive() && (Math.AbsFloat(jammer.GetPosition()[1] - position[1]) <= blockRadius))
                return true;
        }

        return false;
    }

    // Loga atividades da unidade no console
    void LogActivity(string message)
    {
        // O logger adiciona um timestamp. Nós adicionamos a posição para contexto.
        MMLogger.Log(message + " at position: " + GetPosition().ToString());
    }

    // Função chamada periodicamente para atualizar estados visuais e lógicos.
    void PeriodicUpdate()
    {
        // 1. Atualiza o nível com base nos attachments.
        // Isso substitui os eventos de anexo que não são confiáveis.
        UpdateLevelFromAttachments();

        // 2. Atualiza o material da luz com base nos recursos.
        string slotName = "zbytek"; // Nome da selection no .p3d
        string pathGreen = "MyMaintenanceMod\\data\\MaintenceUnit\\materials\\mat_light_green.rvmat";
        string pathRed   = "MyMaintenanceMod\\data\\MaintenceUnit\\materials\\mat_light_red.rvmat";

        int index = GetHiddenSelectionIndex(slotName);

        if (index < 0) {
            return;
        }

        // Verde se há recursos suficientes, vermelho caso contrário
        if (HasEnoughResources()) {
            SetObjectMaterial(index, pathGreen);
        } else {
            SetObjectMaterial(index, pathRed);
        }
    }

    // --- Lógica de Nível da Unidade ---

    // Verifica os attachments e atualiza o nível da unidade.
    void UpdateLevelFromAttachments()
    {
        int previousLevel = m_CurrentLevel;
        int highestAchievableLevel = 1;

        array<ref MaintenanceLevel> levelDefs = MaintenanceConfigManager.GetInstance().GetLevelDefinitions();
        if (!levelDefs || levelDefs.Count() == 0) 
        {
            // Se não há definições, reseta para o nível 1 e sai.
            if (m_CurrentLevel != 1)
            {
                m_CurrentLevel = 1;
            }
            return;
        }

        // 1. Pega a lista de attachments atuais UMA VEZ para otimizar.
        array<string> currentAttachmentTypes = new array<string>;
        for (int i = 0; i < GetInventory().GetAttachmentSlotsCount(); i++)
        {
            int slotId = GetInventory().GetAttachmentSlotId(i);
            EntityAI attachedItem = GetInventory().FindAttachment(slotId);
            if (attachedItem)
            {
                currentAttachmentTypes.Insert(attachedItem.GetType());
            }
        }

        // 2. Itera por TODAS as definições de nível para encontrar o mais alto que podemos alcançar.
        for (int j = 0; j < levelDefs.Count(); j++)
        {
            MaintenanceLevel levelDef = levelDefs.Get(j);
            bool canAchieveThisLevel = true;
            if (levelDef.requiredAttachments && levelDef.requiredAttachments.Count() > 0)
            {
                for (int k = 0; k < levelDef.requiredAttachments.Count(); k++)
                {
                    string requiredType = levelDef.requiredAttachments.Get(k);
                    // Se um dos attachments necessários não for encontrado, não podemos alcançar este nível.
                    if (currentAttachmentTypes.Find(requiredType) == -1)
                    {
                        canAchieveThisLevel = false;
                        break; // Para de verificar os requirements para ESTE nível.
                    }
                }
            }
            
            if (canAchieveThisLevel)
            {
                highestAchievableLevel = levelDef.level;
            }
        }

        // 3. Atribui o nível mais alto encontrado.
        m_CurrentLevel = highestAchievableLevel;

        if (m_CurrentLevel != previousLevel)
            LogActivity("Nível da unidade atualizado de " + previousLevel + " para: " + m_CurrentLevel);
    }

    // Executa o ciclo de manutenção: consome recursos, protege ou danifica estruturas
    void CheckMaintenance()
    {
        if (!this || !this.IsAlive())
            return;

        lastMaintenanceCheckTime = GetGame().GetTime();
        array<Object> nearbyObjects = new array<Object>();
        GetGame().GetObjectsAtPosition(GetPosition(), MaintenanceConfigManager.GetInstance().GetMaintenanceRadius(), nearbyObjects, null);

        string resourceType = MaintenanceConfigManager.GetInstance().GetConsumedItemType();
        if (HasEnoughResources())
        {
            int resourcesToConsume = MaintenanceConfigManager.GetInstance().GetResourcesPerCycle();
            LogActivity("Ativa - Consumindo " + resourcesToConsume + " de " + resourceType + " para proteger a área.");
            ConsumeResources(resourcesToConsume);
            ProtectNearbyStructures(nearbyObjects);
        }
        else
        {
            LogActivity("AVISO: Sem " + resourceType + ". Causando dano às estruturas na área.");
            DamageNearbyStructures(nearbyObjects);
        }
        PreventDespawnNearbyItems(nearbyObjects);
    }

    // Impede o despawn de itens próximos, renovando o tempo de vida
    void PreventDespawnNearbyItems(array<Object> nearbyObjects)
    {
        float heightLimit = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
        foreach (Object obj : nearbyObjects)
        {
            if (Math.AbsFloat(obj.GetPosition()[1] - GetPosition()[1]) <= heightLimit)
            {
                EntityAI entity = EntityAI.Cast(obj);
                if (entity && entity != this)
                {
                    entity.SetLifetime(3888000); // 45 dias em segundos (valor alto)
                }
            }
        }
    }

    // Verifica se uma estrutura é afetada pela unidade
    bool IsStructureAffected(BaseBuildingBase building)
    {
        if (!building) return false;

        array<string> affectedTypes = MaintenanceConfigManager.GetInstance().GetAffectedStructureTypes();
        foreach (string typeName : affectedTypes)
        {
            if (building.IsKindOf(typeName))
            {
                return true;
            }
        }
        return false;
    }

    // Repara todas as zonas de dano de uma entidade
    void RepairEntity(EntityAI entity)
    {
        if (!entity) return;

        array<string> damageZones = new array<string>();
        entity.GetDamageZones(damageZones);

        // Verificação explícita para garantir que o array não é nulo e tem itens.
        if (damageZones && damageZones.Count() > 0)
        {
            foreach (string zone_name : damageZones)
            {
                float max_health = entity.GetMaxHealth(zone_name, "Health");
                if (max_health > 0)
                {
                    entity.SetHealth(zone_name, "Health", max_health);
                }
            }
        }
    }

    // Aplica dano a todas as zonas de dano de uma entidade
    void DamageEntity(EntityAI entity, float damage)
    {
        if (!entity) return;

        array<string> damageZones = new array<string>();
        entity.GetDamageZones(damageZones);

        // Verificação explícita para garantir que o array não é nulo e tem itens.
        if (damageZones && damageZones.Count() > 0)
        {
            foreach (string zone_name : damageZones)
            {
                float max_health = entity.GetMaxHealth(zone_name, "Health");
                if (max_health > 0)
                {
                    float newHealth = Math.Max(0, entity.GetHealth(zone_name, "Health") - damage);
                    entity.SetHealth(zone_name, "Health", newHealth);
                }
            }
        }
    }

    // Repara estruturas e veículos próximos, se configurado
    void ProtectNearbyStructures(array<Object> nearbyObjects)
    {
        bool shouldProtectVehicles = MaintenanceConfigManager.GetInstance().GetProtectVehicles();
        float heightLimit = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
        foreach (Object obj : nearbyObjects)
        {
            // Adiciona verificação de altura.
            if (Math.AbsFloat(obj.GetPosition()[1] - GetPosition()[1]) <= heightLimit)
            {
                BaseBuildingBase building = BaseBuildingBase.Cast(obj);
                CarScript car = CarScript.Cast(obj);

                if (IsStructureAffected(building))
                {
                    RepairEntity(building);
                }
                else if (shouldProtectVehicles && car)
                {
                    RepairEntity(car);
                }
            }
        }
    }

    // Aplica dano a estruturas e veículos próximos, se configurado
    void DamageNearbyStructures(array<Object> nearbyObjects)
    {
        bool shouldProtectVehicles = MaintenanceConfigManager.GetInstance().GetProtectVehicles();
        // Pega os dois valores de dano da configuração uma única vez para otimizar
        float structureDamage = MaintenanceConfigManager.GetInstance().GetDamagePerCycle();
        float vehicleDamage = MaintenanceConfigManager.GetInstance().GetVehicleDamagePerCycle();
        float heightLimit = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
        foreach (Object obj : nearbyObjects)
        {
            if (Math.AbsFloat(obj.GetPosition()[1] - GetPosition()[1]) <= heightLimit)
            {
                BaseBuildingBase building = BaseBuildingBase.Cast(obj);
                CarScript car = CarScript.Cast(obj);

                if (IsStructureAffected(building))
                {
                    // Aplica o dano de estrutura
                    DamageEntity(building, structureDamage);
                }
                else if (shouldProtectVehicles && car)
                {
                    // Aplica o dano de veículo
                    DamageEntity(car, vehicleDamage);
                }
            }
        }
    }

    // Verifica se há recursos suficientes para o próximo ciclo
    bool HasEnoughResources()
    {
        return GetResourceCount() >= MaintenanceConfigManager.GetInstance().GetResourcesPerCycle();
    }

    // Conta a quantidade total de recursos disponíveis no inventário
    int GetResourceCount()
    {
        int resourceCount = 0;
        array<EntityAI> items = new array<EntityAI>();
        GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        string resourceType = MaintenanceConfigManager.GetInstance().GetConsumedItemType();

        foreach (EntityAI item : items)
        {
            if (item && item.IsKindOf(resourceType))
            {
                resourceCount += item.GetQuantity();
            }
        }

        return resourceCount;
    }

    // Consome a quantidade necessária de recursos do inventário
    void ConsumeResources(int amount)
    {
        int remaining = amount;
        array<EntityAI> items = new array<EntityAI>();
        GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        string resourceType = MaintenanceConfigManager.GetInstance().GetConsumedItemType();

        foreach (EntityAI item : items)
        {
            if (item && item.IsKindOf(resourceType))
            {
                int qty = item.GetQuantity();
                if (qty >= remaining)
                {
                    item.SetQuantity(qty - remaining);
                    remaining = 0;
                    return;
                }
                else
                {
                    remaining -= qty;
                    GetGame().ObjectDelete(item);
                }
            }
        }

    }

    // --- FUNÇÕES AUXILIARES PARA CONTAGEM (COPIADO DE ACTIONDEPLOYOBJECT PARA CONSISTÊNCIA) ---

	/**
	 * @brief Conta os containers e partes de construção de base deployados em um raio.
	 */
	void CountDeployablesInRadius(vector center, float radius, out int containerCount, out int baseBuildingCount)
    {
        containerCount = 0;
        baseBuildingCount = 0;
        array<Object> nearbyObjects = new array<Object>();
		GetGame().GetObjectsAtPosition(center, radius, nearbyObjects, null);

        for (int i = 0; i < nearbyObjects.Count(); i++)
        {
            Object obj = nearbyObjects[i];
            // Adiciona verificação de altura para contar apenas objetos no mesmo "nível".
            if (Math.AbsFloat(obj.GetPosition()[1] - center[1]) <= radius)
            {
                // Primeiro, verifica se é um container limitado.
                // A própria MaintenanceUnit herda de Container_Base, então precisamos excluí-la explicitamente da contagem.
                if (IsLimitedContainer(obj) && !obj.IsInherited(MaintenanceUnit))
                {
                    containerCount++;
                }
                // Se não for um container, verifica se é uma parte de construção de base.
                // A própria MaintenanceUnit não deve contar para o limite de construção.
                else if (IsBaseBuildingPart(obj) && !obj.IsInherited(MaintenanceUnit))
                {
                    baseBuildingCount++;
                }
            }
        }
    }

	/**
	 * @brief Verifica se um objeto é um container cujo deploy deve ser limitado (barris, caixas, tendas).
	 */
    bool IsLimitedContainer(Object obj)
    {
        return obj.IsInherited(TentBase) || obj.IsInherited(Container_Base);
    }

	/**
	 * @brief Verifica se um objeto é uma parte de construção de base (parede, piso, etc.).
	 */
    bool IsBaseBuildingPart(Object obj)
    {
        return obj.IsInherited(BaseBuildingBase);
    }

    // Mostra o status da unidade para o jogador
    void ShowStatusTo(PlayerBase player)
    {
        string status;
        string msg;
        string levelInfo = "Nível da Unidade: " + GetCurrentLevel().ToString();

        // --- INÍCIO: Lógica para listar Dono e Membros ---
        string ownerInfo = "";
        string membersInfo = "";

        if (HasOwner())
        {
            array<Man> allPlayers = new array<Man>();
            GetGame().GetPlayers(allPlayers);
            bool ownerFound = false;

            // Procura pelo nome do Dono
            foreach (Man p : allPlayers)
            {
                PlayerBase onlinePlayer = PlayerBase.Cast(p);
                if (onlinePlayer && onlinePlayer.GetIdentity() && onlinePlayer.GetIdentity().GetPlainId() == GetOwnerID())
                {
                    ownerInfo = "Dono: " + onlinePlayer.GetIdentity().GetName() + "\n";
                    ownerFound = true;
                    break;
                }
            }
            if (!ownerFound)
            {
                ownerInfo = "Dono: " + GetOwnerID() + " (Offline)\n";
            }

            // Constrói a lista de Membros
            if (m_Members && m_Members.Count() > 0)
            {
                membersInfo = "Membros:\n";
                foreach (string memberID : m_Members)
                {
                    bool memberFound = false;
                    // Itera pelos jogadores online para encontrar o nome deste membro
                    foreach (Man playerOnline : allPlayers)
                    {
                        PlayerBase pBase = PlayerBase.Cast(playerOnline);
                        if (pBase && pBase.GetIdentity() && pBase.GetIdentity().GetPlainId() == memberID)
                        {
                            membersInfo = membersInfo + " - " + pBase.GetIdentity().GetName() + "\n";
                            memberFound = true;
                            break; // Encontrou, vai para o próximo membro
                        }
                    }

                    // Se o membro não foi encontrado online, mostra seu ID
                    if (!memberFound)
                    {
                        membersInfo = membersInfo + " - " + memberID + " (Offline)\n";
                    }
                }
            }
        }
        // --- FIM: Lógica para listar Dono e Membros ---


        // --- INÍCIO: Lógica para exibir limites de construção ---
        string limitsInfo = "";
        if (MaintenanceConfigManager.GetInstance().GetLimitDeployablesByLevel())
        {
            int currentContainerCount, currentBaseBuildingCount;
            float radius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
            CountDeployablesInRadius(GetPosition(), radius, currentContainerCount, currentBaseBuildingCount);

            int unitLevel = GetCurrentLevel();
            int maxContainers = MaintenanceConfigManager.GetInstance().GetMaxContainersForLevel(unitLevel);
            int maxBaseBuildingParts = MaintenanceConfigManager.GetInstance().GetMaxBaseBuildingForLevel(unitLevel);

            string constructionLimit = string.Format("Construção: %1/%2", currentBaseBuildingCount, maxBaseBuildingParts);
            string containerLimit = string.Format("Containers: %1/%2", currentContainerCount, maxContainers);

            limitsInfo = "\n" + constructionLimit + "\n" + containerLimit;
        }
        // --- FIM: Lógica para exibir limites de construção ---

        if (HasEnoughResources())
        {
            status = "Ativo";
            int totalResources = GetResourceCount();
            int resourcesPerCycle = MaintenanceConfigManager.GetInstance().GetResourcesPerCycle();
            int remainingCycles = totalResources / resourcesPerCycle;
            float totalTimeRemainingSec = remainingCycles * MaintenanceConfigManager.GetInstance().GetCheckInterval();
            // Converte para minutos para facilitar a formatação
            int totalTimeRemainingMin = Math.Ceil(totalTimeRemainingSec / 60.0);

            // Lógica para exibir a unidade de tempo mais apropriada
            string timeString;
            if (totalTimeRemainingMin >= 1440) // Mais de 24 horas (1440 min)
            {
                int days = totalTimeRemainingMin / 1440;
                // O operador % (módulo) não existe no Enforce Script. Usamos a matemática equivalente.
                int hours = (totalTimeRemainingMin - (days * 1440)) / 60;
                timeString = days.ToString() + "d " + hours.ToString() + "h";
            }
            else if (totalTimeRemainingMin > 120) // Mais de 2 horas (120 min)
            {
                timeString = Math.Ceil(totalTimeRemainingMin / 60.0).ToString() + " horas";
            }
            else
            {
                timeString = totalTimeRemainingMin.ToString() + " min";
            }
            msg = levelInfo + "\nStatus: " + status + "\nAutonomia restante: " + timeString + "\n" + ownerInfo + membersInfo + limitsInfo;
        }
        else
        {
            string resourceType = MaintenanceConfigManager.GetInstance().GetConsumedItemType();
            status = "Inativo (Sem " + resourceType + "s)";
            msg = levelInfo + "\nStatus: " + status + "\nReabasteça com " + resourceType + "s para reativar." + "\n" + ownerInfo + membersInfo + limitsInfo;
        }
        NotificationSystem.SendNotificationToPlayerExtended(player, 10.0, "Unidade de Manutenção", msg, "set:dayz_gui image:icon_gear");
    }

    // Permite exibir o inventário da unidade
    override bool CanDisplayCargo()
    {
        // Permite exibir o inventário
        return true;
    }

    // Permite receber apenas o item de recurso no inventário
    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        string resourceType = MaintenanceConfigManager.GetInstance().GetConsumedItemType();
        if (item && item.IsKindOf(resourceType))
        {
            return true;
        }
        return false;
    }

    // Permite carregar apenas o item de recurso no inventário
    override bool CanLoadItemIntoCargo(EntityAI item)
    {
        string resourceType = MaintenanceConfigManager.GetInstance().GetConsumedItemType();
        if (item && item.IsKindOf(resourceType))
        {
            return true;
        }
        return false;
    }

    // Impede colocar a unidade nas mãos se houver itens
    override bool CanPutIntoHands(EntityAI parent)
    {
        return false;
    }

    // Impede colocar a unidade no inventário de outro item se houver itens
    override bool CanPutInCargo(EntityAI parent)
    {
        return false;
    }

    // Impede que outros jogadores adicionem attachments se a unidade tiver dono.
    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        // Requisito: Permitir que qualquer jogador adicione attachments.
        return true;
    }

    override bool CanReleaseAttachment(EntityAI attachment)
    {
       return false; // Impede que outros jogadores removam attachments.
    }

    // Adiciona ação customizada de ver status
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionViewMaintenanceStatus);
        AddAction(ActionClaimMaintenanceUnit);
        AddAction(ActionAbandonOwnership);
    }
}
