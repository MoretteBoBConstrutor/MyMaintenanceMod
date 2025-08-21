class CraftMaintenanceUnitKit extends RecipeBase
{
    // Inicializa a receita de craft do kit de manutenção
    override void Init()
    {
        m_Name = "Craft Maintenance Unit Kit";
        m_IsInstaRecipe = false;
        m_AnimationLength = 2.0;
        m_Specialty = 0.01;

        // --- Condições dos ingredientes ---
        // Ingrediente 1: Rope
        m_MinDamageIngredient[0] = -1;
        m_MaxDamageIngredient[0] = 3;
        m_MinQuantityIngredient[0] = 1;
        m_MaxQuantityIngredient[0] = -1;
        // Ingrediente 2: Wooden Sticks
        m_MinDamageIngredient[1] = -1;
        m_MaxDamageIngredient[1] = 3;
        m_MinQuantityIngredient[1] = 2; // Requer 2 gravetos
        m_MaxQuantityIngredient[1] = -1;

        // --- Ingredientes ---
        InsertIngredient(0, "Rope");
        m_IngredientAddHealth[0] = 0;
        m_IngredientSetHealth[0] = -1;
        m_IngredientAddQuantity[0] = 0;
        m_IngredientDestroy[0] = true; // Consome a corda inteira

        InsertIngredient(1, "WoodenStick");
        m_IngredientAddHealth[1] = 0;
        m_IngredientSetHealth[1] = -1;
        m_IngredientAddQuantity[1] = -2; // Consome 2 gravetos da pilha
        m_IngredientDestroy[1] = false; // Não destrói a pilha se houver mais gravetos

        // --- Resultado ---
        AddResult("MaintenanceUnitKit");
        m_ResultSetFullQuantity[0] = false;
        m_ResultSetQuantity[0] = 1;
        m_ResultSetHealth[0] = -1;
        m_ResultInheritsHealth[0] = -2; // Saúde do resultado será a média dos ingredientes
        m_ResultToInventory[0] = -2;    // O resultado aparece no chão
    }

    // Verifica se o crafting é permitido conforme as regras do mod
    override bool CanDo(ItemBase ingredients[], PlayerBase player)
    {
        if (!MaintenanceConfigManager.GetInstance().GetEnableKitCrafting())
            return false;
        // Previne o crafting se os itens estiverem como anexos em outro item
        if (ingredients[0].GetInventory().IsAttachment() || ingredients[1].GetInventory().IsAttachment())
            return false;
        return super.CanDo(ingredients, player);
    }
}

class DeCraftMaintenanceUnit extends RecipeBase
{	
	override void Init()
	{
		m_Name = "Dismount";
		m_IsInstaRecipe = false;
		m_AnimationLength = 1;
		m_Specialty = 0.02;
		
		
		// --- Conditions ---
        // Ingrediente 1
		m_MinDamageIngredient[0] = -1;//-1 = disable check
		m_MaxDamageIngredient[0] = -1;//-1 = disable check
		m_MinQuantityIngredient[0] = 1;//-1 = disable check
		m_MaxQuantityIngredient[0] = -1;//-1 = disable check
		
        // Ingrediente 2
		m_MinDamageIngredient[1] = -1;//-1 = disable check
		m_MaxDamageIngredient[1] = -1;//-1 = disable check
		m_MinQuantityIngredient[1] = 1;//-1 = disable check
		m_MaxQuantityIngredient[1] = -1;//-1 = disable check
		
		//----------------------------------------------------------------------------------------------------------------------
		
		//INGREDIENTS
		//ingredient 1
		InsertIngredient(0,"MaintenanceUnit");//you can insert multiple ingredients this way
		
		m_IngredientAddHealth[0] = 0;// 0 = do nothing
		m_IngredientSetHealth[0] = -1; // -1 = do nothing
		m_IngredientAddQuantity[0] = 0;// 0 = do nothing
		m_IngredientDestroy[0] = true;//true = destroy, false = do nothing
		m_IngredientUseSoftSkills[0] = false;// set 'true' to allow modification of the values by softskills on this ingredient
		
		//ingredient 2
		InsertIngredient(1,"Screwdriver");//you can insert multiple ingredients this way
		
		m_IngredientAddHealth[1] = -10;// 0 = do nothing
		m_IngredientSetHealth[1] = -1; // -1 = do nothing
		m_IngredientAddQuantity[1] = 0;// 0 = do nothing
		m_IngredientDestroy[1] = false;// false = do nothing
		m_IngredientUseSoftSkills[1] = true;// set 'true' to allow modification of the values by softskills on this ingredient
		
		//----------------------------------------------------------------------------------------------------------------------
		
		//result1
		AddResult("MaintenanceUnitKit");//add results here

		m_ResultSetFullQuantity[0] = false;
		m_ResultSetQuantity[0] = 1; // Should give 1 kit back
		m_ResultSetHealth[0] = -1;
		m_ResultInheritsHealth[0] = 0; // Inherit health from the MaintenanceUnit
		m_ResultToInventory[0] = -2;   // Spawn the resulting kit on the ground
	
	}

	override bool CanDo(ItemBase ingredients[], PlayerBase player)//final check for recipe's validity
	{
		// 1. Verifica se a desmontagem está habilitada na configuração
		if (!MaintenanceConfigManager.GetInstance().GetEnableKitDecrafting())
			return false;

		// 2. Converte o primeiro ingrediente para a nossa unidade de manutenção
		MaintenanceUnit unit = MaintenanceUnit.Cast(ingredients[0]);
		if (unit)
		{
			// 3. Verifica a propriedade da unidade, se o sistema estiver ativo
			if (MaintenanceConfigManager.GetInstance().GetEnableOwnershipSystem())
			{
				// Se a unidade não tiver dono, não pode ser desmontada
				if (!unit.HasOwner())
				{
					return false;
				}
				// Se tiver dono, verifica se o jogador é o dono
				if (unit.GetOwnerID() != player.GetIdentity().GetPlainId())
				{
					return false;
				}
			}

			// 4. Proíbe a desmontagem se a unidade tiver itens em seu inventário
			if (unit.GetInventory().GetCargo().GetItemCount() > 0)
				return false;

			// 5. Proíbe a desmontagem se houver estruturas de base próximas
			float radius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius(); // ajuste conforme necessário
			array<Object> nearby = new array<Object>();
			GetGame().GetObjectsAtPosition(unit.GetPosition(), radius, nearby, null);
			for (int i = 0; i < nearby.Count(); i++)
			{
				BaseBuildingBase b = BaseBuildingBase.Cast(nearby[i]);
				// Adiciona verificação de altura para ignorar construções muito acima ou abaixo.
				if (b && b != unit && (Math.AbsFloat(b.GetPosition()[1] - unit.GetPosition()[1]) <= radius))
				{
					// Bloqueia o decrafting, notificação será feita no Do
					return false;
				}
			}
		}

		return super.CanDo(ingredients, player);
	}

};

modded class PluginRecipesManager
{
	override void RegisterRecipies()
	{
		super.RegisterRecipies();
        RegisterRecipe(new CraftMaintenanceUnitKit);
    	RegisterRecipe(new DeCraftMaintenanceUnit);
	}
}
