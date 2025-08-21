modded class ActionDeployObject
{
	// --- NOVAS FUNÇÕES AUXILIARES ---

	/**
	 * @brief Encontra uma MaintenanceUnit próxima na qual o jogador tem permissão para construir.
	 * @param player O jogador que está tentando construir.
	 * @return A instância da MaintenanceUnit se encontrada e válida, senão null.
	 */
	private MaintenanceUnit GetNearbyMaintenanceUnitFor(PlayerBase player)
	{
		vector position_to_check = player.GetPosition(); // Usa a posição do jogador como referência

		float searchRadius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
		array<Object> nearbyObjects = new array<Object>();
		GetGame().GetObjectsAtPosition(position_to_check, searchRadius, nearbyObjects, null);

		for (int i = 0; i < nearbyObjects.Count(); i++)
		{
			MaintenanceUnit unit = MaintenanceUnit.Cast(nearbyObjects[i]);
			if (unit && (Math.AbsFloat(unit.GetPosition()[1] - position_to_check[1]) <= searchRadius))
			{
				if (!MaintenanceConfigManager.GetInstance().GetEnableOwnershipSystem())
					return unit; // Retorna a unidade se a propriedade estiver desativada

				if (unit.HasOwner() && unit.IsMemberOrOwner(player.GetIdentity().GetPlainId()))
					return unit; // Retorna a unidade se o jogador for o dono OU um membro
			}
		}
		return null; // Nenhuma unidade válida encontrada
	}

	/**
	 * @brief Conta os containers e partes de construção de base deployados em um raio.
	 */
	private void CountDeployablesInRadius(vector center, float radius, out int containerCount, out int baseBuildingCount)
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
				// CORREÇÃO: Adicionada verificação para excluir a própria MaintenanceUnit da contagem de containers.
				if (IsLimitedContainer(obj) && !obj.IsInherited(MaintenanceUnit))
				{
					containerCount++;
				}
				// Se não for um container, verifica se é uma parte de construção de base.
				// A própria MaintenanceUnit não deve contar para este limite.
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
    private bool IsLimitedContainer(Object obj)
    {
        return obj.IsInherited(TentBase) || obj.IsInherited(Container_Base);
    }

	/**
	 * @brief Verifica se um objeto é um kit de construção cujo deploy deve ser limitado.
	 */
    private bool IsLimitedKit(Object obj)
    {
		//Nova Lógica de Compatibilidade
		ItemBase item = ItemBase.Cast(obj);
        if (!item)
            return false;

        // Exclui o kit da própria unidade de manutenção de qualquer restrição.
        if (item.IsInherited(MaintenanceUnitKit))
            return false;

        // Verifica se o item é um kit de construção de base padrão (vanilla).
        bool isStandardKit = item.IsBasebuildingKit();

        // Verifica se o item está na lista de restrição customizada do JSON para compatibilidade com mods.
        bool isCustomRestricted = false;
        array<string> restrictedList = MaintenanceConfigManager.GetInstance().GetRestrictedPlacementItems();
        if (restrictedList && restrictedList.Find(item.GetType()) != -1)
        {
            isCustomRestricted = true;
        }

        // Um kit é limitado se for um kit de construção padrão OU estiver na lista de restrição.
        return isStandardKit || isCustomRestricted;
	}

	/**
	 * @brief Verifica se um objeto é uma parte de construção de base (parede, piso, etc.).
	 */
    private bool IsBaseBuildingPart(Object obj)
    {
        return obj.IsInherited(BaseBuildingBase);
    }

	/**
	 * @brief Verifica se um kit é gerenciado pelo sistema e retorna o tipo do objeto que ele irá gerar.
	 * @param item O kit que o jogador está segurando.
	 * @return O nome da classe do objeto final se o kit for gerenciado, senão uma string vazia.
	 */
	private string GetManagedDeployedType(ItemBase item)
	{
		if (!item)
			return "";
		// Esta função irá consultar o mapa "ManagedFurnitureMap" que você criará no seu JSON
		return MaintenanceConfigManager.GetInstance().GetDeployedTypeForKit(item.GetType());
	}

	// --- LÓGICA DA AÇÃO REFEITA ---

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		string msg; // Variável para mensagens de notificação, declarada uma única vez.

		if (player.GetBrokenLegs() == eBrokenLegs.BROKEN_LEGS)
			return false;

		// Lógica do lado do cliente (visualização do holograma)
		if (!GetGame().IsDedicatedServer())
		{
			if (player.IsPlacingLocal())
			{
				if (player.GetHologramLocal().IsColliding())
					return false;
			
				if (item.CanBePlaced(player, player.GetHologramLocal().GetProjectionEntity().GetPosition()))
					return true;
			}
			return false;
		}

		// Lógica do lado do servidor (regras de construção)
		
		// --- INÍCIO DA NOVA LÓGICA DE COMPATIBILIDADE ---
		string managedDeployedType = GetManagedDeployedType(item);
		bool isDeployingManagedContainer = false;

		if (managedDeployedType != "")
		{
			// Usamos GetGame().IsKindOf() para checar a herança de uma classe pelo seu nome em string.
			// Isso nos permite saber se o item final será um container.
			if (GetGame().IsKindOf(managedDeployedType, "Container_Base"))
			{
				isDeployingManagedContainer = true;
			}
		}
		// --- FIM DA NOVA LÓGICA DE COMPATIBILIDADE ---

		// 1. Restrição de proximidade para a própria MaintenanceUnit
		if (item.IsInherited(MaintenanceUnitKit))
        {
			vector place_pos = player.GetPosition(); // Usa a posição do jogador para a verificação de distância
			float min_dist = MaintenanceConfigManager.GetInstance().GetMinDistanceBetweenUnits();
			array<Object> nearby_units = new array<Object>();
			GetGame().GetObjectsAtPosition(place_pos, min_dist, nearby_units, null);

			foreach (Object obj : nearby_units)
			{
				if (obj.IsInherited(MaintenanceUnit) && (Math.AbsFloat(obj.GetPosition()[1] - place_pos[1]) <= min_dist))
				{
					if (vector.Distance(place_pos, obj.GetPosition()) < min_dist)
					{
						msg = string.Format("Você está muito perto de outra Unidade de Manutenção (mínimo %1m).", min_dist);
						NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Restrição", msg, "set:dayz_gui image:icon_error");
						return false;
					}
				}
			}
		}

		// 2. Lógica de restrição para itens gerenciados (Kits de construção, Containers e Mobília)
		bool isLimitedContainer = IsLimitedContainer(item);
        bool isLimitedKit = IsLimitedKit(item);

		// A condição principal agora inclui os kits de mobília gerenciados
		if (isLimitedContainer || isLimitedKit || isDeployingManagedContainer)
		{
			MaintenanceUnit nearbyUnit = GetNearbyMaintenanceUnitFor(player);

			// CASO 1: O jogador está DENTRO do raio de sua unidade.
			if (nearbyUnit)
			{
				// REGRA DO TERRITÓRIO: Verifica os limites de quantidade.
				if (MaintenanceConfigManager.GetInstance().GetLimitDeployablesByLevel())
	            {
	                int currentContainerCount, currentBaseBuildingCount;
	                float radius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
	                CountDeployablesInRadius(nearbyUnit.GetPosition(), radius, currentContainerCount, currentBaseBuildingCount);
	                int unitLevel = nearbyUnit.GetCurrentLevel();

					// A verificação de container agora inclui os kits de mobília que geram containers
	                if (isLimitedContainer || isDeployingManagedContainer)
	                {
	                    int maxContainers = MaintenanceConfigManager.GetInstance().GetMaxContainersForLevel(unitLevel);
	                    if (currentContainerCount >= maxContainers)
	                    {
	                        msg = string.Format("Limite de containers (%1) atingido para esta área de Nível %2.", maxContainers, unitLevel);
	                        NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Construção Bloqueada", msg, "set:dayz_gui image:icon_error");
	                        return false;
	                    }
	                }
	                else if (isLimitedKit)
	                {
	                    int maxBaseBuildingParts = MaintenanceConfigManager.GetInstance().GetMaxBaseBuildingForLevel(unitLevel);
	                    if (currentBaseBuildingCount >= maxBaseBuildingParts)
	                    {
	                        msg = string.Format("Limite de partes de construção (%1) atingido para esta área de Nível %2.", maxBaseBuildingParts, unitLevel);
	                        NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Construção Bloqueada", msg, "set:dayz_gui image:icon_error");
	                        return false;
	                    }
	                }
	            }
			}
			// CASO 2: O jogador está FORA do raio de sua unidade.
			else
			{
				// REGRA DO MUNDO ABERTO: Proíbe kits de construção de base E kits de mobília gerenciados.
				if (isLimitedKit || managedDeployedType != "")
				{
					string errorMsg = "Você só pode construir isso dentro do raio de sua Unidade de Manutenção.";
					NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Construção Bloqueada", errorMsg, "set:dayz_gui image:icon_error");
					return false;
				}
				// Containers vanilla (tendas, barris) ainda são permitidos.
			}
		}

		return true;
	}
}
