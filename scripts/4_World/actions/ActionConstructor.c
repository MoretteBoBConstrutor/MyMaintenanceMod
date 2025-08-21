modded class ActionConstructor
{
    // Registra ações customizadas do mod
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        actions.Insert(ActionViewMaintenanceStatus);
        actions.Insert(ActionClaimMaintenanceUnit);
        actions.Insert(ActionAbandonOwnership);
        actions.Insert(ActionTurnOnJammer);
        actions.Insert(ActionTurnOffJammer);
        actions.Insert(ActionAddMember);
        actions.Insert(ActionRemoveMember);
    }
}

