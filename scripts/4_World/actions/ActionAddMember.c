class ActionAddMember : ActionInteractBase
{
    void ActionAddMember()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_Text = "Adicionar Membro";
    }

    override void CreateConditionComponents()
    {
        m_ConditionTarget = new CCTMan(5); // O alvo deve ser um jogador a até 5 metros
        m_ConditionItem = new CCINone;
    }

    override bool HasTarget()
    {
        return true;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        PlayerBase targetPlayer = PlayerBase.Cast(target.GetObject());
        if (!targetPlayer || targetPlayer == player || !player)
            return false;

        // --- VERIFICAÇÃO DE PROXIMIDADE (CLIENT-SIDE) ---
        // A ação só deve aparecer se o jogador estiver perto de uma unidade de manutenção.
        // Esta verificação é feita no cliente e é puramente para a UI.
        float searchRadius = 10.0; // Raio de busca em metros
        array<Object> nearbyObjects = new array<Object>();
        GetGame().GetObjectsAtPosition3D(player.GetPosition(), searchRadius, nearbyObjects, null);

        for (int i = 0; i < nearbyObjects.Count(); i++)
        {
            // Se encontrarmos qualquer objeto que seja uma MaintenanceUnit no raio, a condição é satisfeita.
            if (nearbyObjects[i].IsInherited(MaintenanceUnit))
            {
                // A lógica final (se o jogador é o dono, etc.) é validada no servidor.
                return true;
            }
        }

        // Se o loop terminar e nenhuma unidade for encontrada, a ação não aparece.
        return false;
    }

    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);
        PlayerBase player = action_data.m_Player;
        PlayerBase targetPlayer = PlayerBase.Cast(action_data.m_Target.GetObject());
        
        if (!targetPlayer || !player || !player.GetIdentity() || !targetPlayer.GetIdentity() || targetPlayer == player) return;
        
        string targetID = targetPlayer.GetIdentity().GetPlainId();
        string targetName = targetPlayer.GetIdentity().GetName();
        
        // Verificação global: O jogador alvo já pertence a alguma unidade?
        array<MaintenanceUnit> allUnits = MaintenanceUnit.GetAllUnits();
        if (allUnits)
        {
            foreach (MaintenanceUnit anyUnit : allUnits)
            {
                if (anyUnit.IsMemberOrOwner(targetID))
                {
                    string msg = "O jogador " + targetName + " já pertence a uma unidade de manutenção.";
                    NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Falha ao Adicionar", msg, "set:dayz_gui image:icon_error");
                    return; // Interrompe a ação
                }
            }
        }
        
        MaintenanceUnit unit = GetNearestMaintenanceUnit(player);
        if (unit)
        {
            bool isOwner = unit.GetOwnerID() == player.GetIdentity().GetPlainId();
            bool targetIsClose = vector.Distance(targetPlayer.GetPosition(), unit.GetPosition()) <= 5;
            if (isOwner && !unit.IsJammed() && targetIsClose)
            {
                unit.AddMember(targetID);
                NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Sucesso", "Membro " + targetName + " adicionado.", "set:dayz_gui image:icon_info");
                NotificationSystem.SendNotificationToPlayerExtended(targetPlayer, 5, "Você foi Adicionado", "Você agora é membro da unidade de " + player.GetIdentity().GetName() + ".", "set:dayz_gui image:icon_info");
            }
        }
    }

    // Função auxiliar para encontrar a unidade mais próxima
    MaintenanceUnit GetNearestMaintenanceUnit(PlayerBase player)
    {
        float closestDistSq = 5 * 5; // Raio máximo de busca (5 metros), ao quadrado para eficiência
        MaintenanceUnit closestUnit = null;

        array<MaintenanceUnit> allUnits = MaintenanceUnit.GetAllUnits();
        if (allUnits)
        {
            foreach (MaintenanceUnit unit : allUnits)
            {
                float distSq = vector.DistanceSq(player.GetPosition(), unit.GetPosition());
                if (distSq < closestDistSq && (Math.AbsFloat(unit.GetPosition()[1] - player.GetPosition()[1]) <= 10.0))
                {
                    closestDistSq = distSq;
                    closestUnit = unit;
                }
            }
        }
        return closestUnit;
    }
}