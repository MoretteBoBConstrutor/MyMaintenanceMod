class ActionViewMaintenanceStatus : ActionInteractBase
{
    // Construtor: define comando e texto da ação
    void ActionViewMaintenanceStatus()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_Text = "Ver status da manutenção";
    }

    // Condição para exibir a ação: só aparece se o alvo for uma MaintenanceUnit
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        MaintenanceUnit unit = MaintenanceUnit.Cast(target.GetObject());
        if (!unit) return false;
        return true;
    }

    // A ação é iniciada no cliente. O cliente deve pedir os dados ao servidor.
    // Usamos OnExecuteClient para ações que acontecem uma vez.
    override void OnExecuteClient(ActionData action_data)
    {
        super.OnExecuteClient(action_data);

        // Envia um RPC do cliente PARA o servidor para solicitar os dados da unidade de manutenção.
        // O último parâmetro é 'null' porque o destino é o servidor.
        GetRPCManager().SendRPC("MyMaintenanceMod", "RequestMaintenanceStatus", new Param1<MaintenanceUnit>(MaintenanceUnit.Cast(action_data.m_Target.GetObject())), true, null);
        Print("[ActionViewMaintenanceStatus] Enviando solicitação de status de manutenção para o servidor pelo Jogador: " + action_data.m_Player.GetIdentity().GetName());
    }
}
