/**
 * @class EventBeacon
 * @brief Beacon invisível para detectar spawns de eventos militares
 */
class EventBeacon extends ItemBase
{
    void EventBeacon()
    {
        Print("[EventBeacon] Construtor chamado - Beacon criado");
    }
    
    /**
     * @brief Método chamado UMA vez na criação do objeto (igual ao HeliCrash)
     * Previne registros duplicados na inicialização do servidor
     */
    override void EEOnCECreate()
    {
        Print("[EventBeacon] EEOnCECreate() chamado - criação única");
        //super.EEOnCECreate();
        
        if (GetGame().IsServer())
        {
            Print("[EventBeacon] Executando no servidor");
            // Carrega configuração
            EventBeaconConfig config = EventBeaconConfig.GetInstance();
            
            if (!config.m_EnableEventMonitoring)
            {
                if (config.m_EnableDebugLogs)
                    Print("[EventBeacon] Sistema desabilitado - beacon ignorado");
                return;
            }
            
            vector beaconPos = GetPosition();
            
            Print("[EventBeacon] Evento militar detectado na posição: " + beaconPos);
            
            // Registra evento no sistema de rastreamento para aparecer no mapa
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RegisterMilitaryEvent, 500, false);
            
            // NÃO fazemos auto-destruição - deixamos o sistema de evento gerenciar
            // O evento tem lifetime=1800s (30min) e cleanupradius=1000m
        }
        else
        {
            Print("[EventBeacon] Executando no cliente - ignorando");
        }
    }
    
    /*
    // COMENTADO PARA TESTE - EEInit pode ser chamado múltiplas vezes
    override void EEInit()
    {
        Print("[EventBeacon] EEInit() chamado - início");
        super.EEInit();
        
        if (GetGame().IsServer())
        {
            Print("[EventBeacon] Executando no servidor");
            // Carrega configuração
            EventBeaconConfig config = EventBeaconConfig.GetInstance();
            
            if (!config.m_EnableEventMonitoring)
            {
                if (config.m_EnableDebugLogs)
                    Print("[EventBeacon] Sistema desabilitado - beacon ignorado");
                return;
            }
            
            vector beaconPos = GetPosition();
            
            Print("[EventBeacon] Evento militar detectado na posição: " + beaconPos);
            
            // Registra evento no sistema de rastreamento para aparecer no mapa
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RegisterMilitaryEvent, 500, false);
            
            // NÃO fazemos auto-destruição - deixamos o sistema de evento gerenciar
            // O evento tem lifetime=1800s (30min) e cleanupradius=1000m
        }
        else
        {
            Print("[EventBeacon] Executando no cliente - ignorando");
        }
    }
    */
    
    /**
     * @brief Registra evento militar no sistema de mapa
     */
    void RegisterMilitaryEvent()
    {
        // Registra este evento no sistema de rastreamento para aparecer no mapa
        // Similar ao HeliCrashTracker, mas para eventos militares
        MilitaryEventTracker.RegisterEvent(GetPosition());
        
        EventBeaconConfig config = EventBeaconConfig.GetInstance();
        if (config.m_EnableDebugLogs)
        {
            Print("[EventBeacon] Evento militar registrado no mapa na posição: " + GetPosition());
        }
    }
    
    override void EEDelete(EntityAI parent)
    {
        // Quando o evento for limpo pelo sistema, remove do tracker
        if (GetGame().IsServer())
        {
            MilitaryEventTracker.UnregisterEvent(GetPosition());
            
            EventBeaconConfig config = EventBeaconConfig.GetInstance();
            if (config.m_EnableDebugLogs)
            {
                Print("[EventBeacon] Evento militar removido do mapa: " + GetPosition());
            }
        }
        
        super.EEDelete(parent);
    }

    override bool CanPutIntoHands(EntityAI parent)
    {
        return false;
    }

    // Impede colocar a unidade no inventário de outro item se houver itens
    override bool CanPutInCargo(EntityAI parent)
    {
        return false;
    }
}
