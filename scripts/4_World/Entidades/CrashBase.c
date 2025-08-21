modded class CrashBase
{
    /**
     * @brief Sobrescreve o método EOnInit, que é chamado quando a entidade (o destroço do helicóptero) é criada no mundo.
     */
    override void EEOnCECreate()
    {
        super.EEOnCECreate();

        // A lógica deve rodar apenas no servidor para garantir consistência e segurança.
        if (GetGame().IsServer())
        {
            // Registra este crash no sistema de rastreamento
            HeliCrashTracker.RegisterCrash(this);
            
            // Usamos CallLater para garantir que a posição do objeto esteja totalmente inicializada
            // e para desassociar nossa lógica do processo de criação do objeto, evitando problemas de timing.
            //GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.NotifyEligiblePlayers, 500, false);
        }
    }

    override void EEDelete(EntityAI parent)
    {
        // Remove este crash do sistema de rastreamento quando for destruído
        if (GetGame().IsServer())
        {
            HeliCrashTracker.UnregisterCrash(this);
        }
        
        super.EEDelete(parent);
    }

    /**
     * @brief Procura por todos os jogadores online que possuem a DogTag_R_TypeFive e envia uma notificação.
     */
    /* void NotifyEligiblePlayers()
    {
        // Se o objeto não for mais válido (pode acontecer em raras condições), a função é interrompida.
        if (!this) return;

        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        vector crashPosition = this.GetPosition();
        // Formata as coordenadas para serem mais legíveis (apenas X e Z, arredondados).
        string formattedCoords = "[" + Math.Round(crashPosition[0]) + ", " + Math.Round(crashPosition[2]) + "]";
        string message = "Sinal de emergência de alto nível detectado em: " + formattedCoords;

        foreach (Man man : players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            // CORREÇÃO CRÍTICA: Primeiro, verificamos se o jogador é válido e está vivo.
            if (player && player.IsAlive())
            {
                EntityAI attachment = player.GetInventory().FindAttachment(InventorySlots.GetSlotIdFromString("Kdu_link"));
                if (attachment && attachment.IsKindOf("DogTag_R_TypeFive"))
                {
                    // Envia a notificação para o jogador elegível.
                    NotificationSystem.SendNotificationToPlayerExtended(player, 15, "Sinal de Emergência Detectado", message, "set:dayz_gui image:icon_radio");
                    Print("[MyMaintenanceMod] Notificação de HeliCrash enviada para " + player.GetIdentity().GetName() + ". Coordenadas: " + crashPosition);
                }
            }
        }
    } */
}