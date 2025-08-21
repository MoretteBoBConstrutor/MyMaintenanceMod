class ActionClaimMaintenanceUnit : ActionInteractBase
{
    void ActionClaimMaintenanceUnit()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_Text = "Reivindicar Unidade";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        MaintenanceUnit unit = MaintenanceUnit.Cast(target.GetObject());
        if (!unit || !player) 
            return false;
        
        // Impede a ação se a unidade estiver arruinada.
        if (unit.IsRuined())
            return false;

        // --- NOVA CONDIÇÃO ---
        // Impede a ação se houver um Jammer ativo próximo.
        if (unit.IsJammed())
            return false;

        // Condição 1: A unidade alvo não pode ter dono.
        if (unit.HasOwner())
            return false;

        // Condição 2: O jogador não pode já possuir uma unidade.
        // Esta verificação agora usa a variável sincronizada no próprio jogador, que é muito mais confiável.
        if (player.HasMaintenanceUnit())
            return false;

        return true;
    }

    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);
        MaintenanceUnit unit = MaintenanceUnit.Cast(action_data.m_Target.GetObject());
        if (unit)
        {
            unit.SetOwner(action_data.m_Player);
        }
    }
    
    override void OnStartClient(ActionData action_data)
    {
        super.OnStartClient(action_data);
        
        // Previsão no lado do cliente para uma resposta instantânea da UI
        MaintenanceUnit unit = MaintenanceUnit.Cast(action_data.m_Target.GetObject());
        PlayerBase player = action_data.m_Player;
        
        if (unit && player && player.GetIdentity())
        {
            unit.Client_SetCachedOwnerID(player.GetIdentity().GetPlainId());
        }
    }
}
