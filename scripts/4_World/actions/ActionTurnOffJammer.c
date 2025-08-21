class ActionTurnOffJammer : ActionInteractBase
{
    void ActionTurnOffJammer()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_Text = "Desligar Jammer";
    }

    override void CreateConditionComponents()
    {
        m_ConditionTarget = new CCTNonRuined(UAMaxDistances.DEFAULT);
        m_ConditionItem = new CCINone;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        Jammer jammer = Jammer.Cast(target.GetObject());
        // A única condição para desligar é que seja um Jammer e que ele esteja ativo.
        if (jammer && jammer.IsActive())
        {
            return true;
        }
        return false;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        Jammer jammer = Jammer.Cast(action_data.m_Target.GetObject());
        if (jammer)
        {
            jammer.TurnOff();
            // Feedback para o jogador
            NotificationSystem.SendNotificationToPlayerIdentityExtended(action_data.m_Player.GetIdentity(), 5, "Jammer", "Jammer desativado.", "set:dayz_gui image:icon_radio");
        }
    }
}