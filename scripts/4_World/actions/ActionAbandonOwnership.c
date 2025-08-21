class ActionAbandonOwnership : ActionInteractBase
{
    void ActionAbandonOwnership()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_Text = "Abandonar Propriedade";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        MaintenanceUnit unit = MaintenanceUnit.Cast(target.GetObject());
        if (!unit || !player || !player.GetIdentity()) 
            return false;
        
        // --- NOVA CONDIÇÃO ---
        // Impede a ação se houver um Jammer ativo próximo.
        if (unit.IsJammed())
            return false;

        // A condição é: a unidade deve ter um dono, e o jogador deve ser esse dono.
        return unit.HasOwner() && unit.GetOwnerID() == player.GetIdentity().GetPlainId();
    }

    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);
        MaintenanceUnit unit = MaintenanceUnit.Cast(action_data.m_Target.GetObject());
        if (unit)
        {
            unit.AbandonOwnership(action_data.m_Player);
        }
    }

    override void OnStartClient(ActionData action_data)
    {
        super.OnStartClient(action_data);
        
        // Previsão no lado do cliente para uma resposta instantânea da UI
        MaintenanceUnit unit = MaintenanceUnit.Cast(action_data.m_Target.GetObject());
        if (unit)
        {
            unit.Client_SetCachedOwnerID("");
        }
    }
}
