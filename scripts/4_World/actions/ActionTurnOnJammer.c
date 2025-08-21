class ActionTurnOnJammer : ActionInteractBase
{
    void ActionTurnOnJammer()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_Text = "Ligar Jammer";
    }

    override void CreateConditionComponents()
    {
        // A condição agora é sobre o alvo (target), não sobre o item nas mãos.
        m_ConditionTarget = new CCTNonRuined(UAMaxDistances.DEFAULT);
        m_ConditionItem = new CCINone; // Não requer item nas mãos.
    }

    // --- CONDIÇÃO SIMPLIFICADA ---
    // A ação agora só verifica se o alvo é um Jammer e se ele está desligado.
    // Isso garante que a opção "Ligar Jammer" sempre apareça.
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        Jammer jammer = Jammer.Cast(target.GetObject());
        if (!jammer || jammer.IsActive())
        {
            return false;
        }
        return true;
    }

    // --- LÓGICA MOVIDA PARA O SERVIDOR ---
    // --- LÓGICA MOVIDA PARA O SERVIDOR (COM DEBUG AVANÇADO) ---
    override void OnExecuteServer(ActionData action_data)
    {
        Jammer jammer = Jammer.Cast(action_data.m_Target.GetObject());
        PlayerBase player = action_data.m_Player;

        // --- INÍCIO: VERIFICAÇÃO DE JAMMERS PRÓXIMOS ---
        float checkRadius = MaintenanceConfigManager.GetInstance().GetMinDistanceBetweenUnits();
        vector jammerPos = jammer.GetPosition();
        array<Object> nearbyObjects = new array<Object>();
        GetGame().GetObjectsAtPosition(jammerPos, checkRadius, nearbyObjects, null);

        foreach (Object obj : nearbyObjects)
        {
            Jammer otherJammer = Jammer.Cast(obj);
            // Verifica se o objeto é um Jammer, se não é o próprio jammer que estamos ativando, e se está ativo.
            if (otherJammer && otherJammer != jammer && otherJammer.IsActive() && (Math.AbsFloat(otherJammer.GetPosition()[1] - jammerPos[1]) <= checkRadius))
            {
                string msg = string.Format("Falha: Outro Jammer já está ativo em um raio de %1m.", checkRadius);
                NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 5, "Jammer", msg, "set:dayz_gui image:icon_radio");
                MMLogger.Log("[DEBUG] FALHA: Tentativa de ativar Jammer, mas outro já está ativo no raio. Posição: " + otherJammer.GetPosition());
                return; // Interrompe a ação.
            }
        }
        // --- FIM: VERIFICAÇÃO DE JAMMERS PRÓXIMOS ---

        MMLogger.Log("[DEBUG] ----------------------------------------------------");
        MMLogger.Log("[DEBUG] Ação 'Ligar Jammer' executada. Iniciando verificações no servidor.");

        // ETAPA 1: Verificar se uma DogTag está FISICAMENTE anexada.
        DogTag_Registered tag = jammer.GetAttachedDogTag();
        if (!tag)
        {
            MMLogger.Log("[DEBUG] FALHA CRÍTICA: O método jammer.GetAttachedDogTag() não encontrou nenhum item 'DogTag' anexado.");
            NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 5, "Jammer", "Falha: Nenhuma DogTag anexada.", "set:dayz_gui image:icon_radio");
            MMLogger.Log("[DEBUG] ----------------------------------------------------");
            return;
        }

        MMLogger.Log("[DEBUG] SUCESSO: Um item 'DogTag' foi encontrado anexado ao Jammer.");

        // ETAPA 2: Verificar se a DogTag encontrada tem um Dono (OwnerID).
        string targetOwnerID = tag.GetOwnerID();
        if (targetOwnerID == "")
        {
            MMLogger.Log("[DEBUG] FALHA CRÍTICA: A DogTag foi encontrada, mas seu OwnerID está VAZIO.");
            NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 5, "Jammer", "Falha: A DogTag não tem um dono registrado.", "set:dayz_gui image:icon_radio");
            MMLogger.Log("[DEBUG] ----------------------------------------------------");
            return;
        }

        MMLogger.Log("[DEBUG] SUCESSO: A DogTag tem um dono. ID do Dono na DogTag: '" + targetOwnerID + "'");
        MMLogger.Log("[DEBUG] Posição do Jammer: " + jammer.GetPosition());

        // ETAPA 3: Procurar pela base correspondente.
        array<MaintenanceUnit> allUnits = MaintenanceUnit.GetAllUnits();
        MMLogger.Log("[DEBUG] Total de Mesas de Manutenção no servidor: " + allUnits.Count());

        bool foundMatchingOwner = false;
        foreach (MaintenanceUnit unit : allUnits)
        {
            MMLogger.Log("[DEBUG] Verificando Mesa ID " + unit.GetID() + " | Dono: '" + unit.GetOwnerID() + "'");

            // CORREÇÃO: Verifica se o dono da DogTag é o dono OU um membro da unidade.
            if (unit.IsMemberOrOwner(targetOwnerID))
            {
                foundMatchingOwner = true;
                MMLogger.Log("[DEBUG] >> SUCESSO: Permissão encontrada! O dono da DogTag é membro ou dono desta unidade.");
                
                float distance = vector.Distance(jammer.GetPosition(), unit.GetPosition());
                float maxRadius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
                MMLogger.Log("[DEBUG] >> Distância: " + distance + " | Raio Máximo: " + maxRadius);

                if (distance <= maxRadius)
                {
                    MMLogger.Log("[DEBUG] >> SUCESSO: Mesa está no alcance. Ativando Jammer.");
                    jammer.TurnOn();
                    NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 5, "Jammer", "Jammer ativado com sucesso!", "set:dayz_gui image:icon_radio");
                    MMLogger.Log("[DEBUG] ----------------------------------------------------");
                    return;
                }
                else
                {
                    MMLogger.Log("[DEBUG] >> FALHA: A Mesa correspondente está FORA do alcance.");
                }
            }
        }

        MMLogger.Log("[DEBUG] Fim da verificação.");
        if (!foundMatchingOwner) {
            MMLogger.Log("[DEBUG] Resultado: Nenhuma Mesa de Manutenção encontrada onde o dono da DogTag seja membro ou proprietário.");
        } else {
            MMLogger.Log("[DEBUG] Resultado: Uma ou mais Mesas foram encontradas, mas nenhuma estava perto o suficiente.");
        }
        NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 5, "Jammer", "Falha: Nenhuma base alvo no alcance.", "set:dayz_gui image:icon_radio");
        MMLogger.Log("[DEBUG] ----------------------------------------------------");
    }
}