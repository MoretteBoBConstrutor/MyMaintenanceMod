modded class MissionGameplay
{
    // --- Variáveis existentes para o Menu de Manutenção ---
    ref MaintenanceStatusMenu iMaintenanceStatusMenu;

    // --- Novas variáveis para o HUD de Coordenadas ---
    private Widget m_CoordenadasHUDRoot; // Apenas a referência ao widget raiz é necessária.
    private ref CoordenadasRootPanel m_CoordenadasHUDController; // Controlador do HUD

    // --- Novas variáveis para o GPS ---
    private Widget m_GPSHUDRoot; // Referência ao widget raiz do GPS
    private ref GPSRootPanel m_GPSHUDController;

    ref HeliCrashMapMenu m_HeliCrashMapMenu; // Controlador do mapa de crashes (ref para manter vivo)
    
    // --- Debounce para tecla L ---
    private bool m_CrashMapKeyPressed = false; // Flag para evitar múltiplas detecções

	override void OnInit()
	{
		super.OnInit();
        // CORREÇÃO: O RPC deve ser registrado na instância 'this' da MissionGameplay para garantir que a função seja chamada corretamente.
        GetRPCManager().AddRPC("MyMaintenanceMod", "OpenMaintenanceStatusMenu", this, SingeplayerExecutionType.Client);
        GetRPCManager().AddRPC("MyMaintenanceMod", "ReceiveHeliCrashes", this, SingeplayerExecutionType.Client);
        GetRPCManager().AddRPC("MyMaintenanceMod", "ReceiveMilitaryEvents", this, SingeplayerExecutionType.Client);
	}

    void OpenMaintenanceStatusMenu(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        if (type == CallType.Client)
        {
            Param1<MaintenanceStatusMenuData> data;
            if (!ctx.Read(data))
                return;

            // Se o nosso menu já estiver aberto, fecha-o antes de abrir um novo para evitar duplicatas.
            if (iMaintenanceStatusMenu)
                iMaintenanceStatusMenu.Close();

            // CORREÇÃO: Cria uma NOVA instância do menu e a exibe.
            iMaintenanceStatusMenu = new MaintenanceStatusMenu();
            GetGame().GetUIManager().ShowScriptedMenu(iMaintenanceStatusMenu, null);
            Print("[MissionServer] OpenMaintenanceStatusMenu — exibindo o menu de status de manutenção para " + sender.GetName());

            // Agora que o menu foi inicializado e seus widgets existem, preenchemos com os dados.
            iMaintenanceStatusMenu.SetData(data.param1);
            Print("[MissionServer] OpenMaintenanceStatusMenu — dados preenchidos no menu de status de manutenção.");
        }
    }

    void ReceiveHeliCrashes(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        if (type == CallType.Client)
        {
            Param1<HeliCrashMapData> data;
            if (!ctx.Read(data))
            {
                Print("[MissionGameplay] ReceiveHeliCrashes — ERRO: Falha ao ler dados do RPC");
                return;
            }

            Print("[MissionGameplay] ReceiveHeliCrashes — Recebidos dados com " + data.param1.GetCrashCount() + " crashes do servidor");
            
            // Sincroniza dados no HeliCrashTracker local
            HeliCrashTracker.SyncFromServer(data.param1);
            Print("[MissionGameplay] ReceiveHeliCrashes — " + data.param1.GetCrashCount() + " crashes sincronizados com HeliCrashTracker");
        }
    }

    void ReceiveMilitaryEvents(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        if (type == CallType.Client)
        {
            Param1<ref array<vector>> data;
            if (!ctx.Read(data))
            {
                Print("[MissionGameplay] ReceiveMilitaryEvents — ERRO: Falha ao ler dados do RPC");
                return;
            }

            Print("[MissionGameplay] ReceiveMilitaryEvents — Recebidos dados com " + data.param1.Count() + " eventos militares do servidor");
            
            // Sincroniza dados no MilitaryEventTracker local
            MilitaryEventTracker.SyncFromServer(data.param1);
            Print("[MissionGameplay] ReceiveMilitaryEvents — " + data.param1.Count() + " eventos militares sincronizados com MilitaryEventTracker");
        }
    }

    // --- Novo: Inicialização do HUD de Coordenadas ---
    override void OnMissionStart()
    {
        super.OnMissionStart();

        if (GetGame().IsClient())
        {
            if (!m_CoordenadasHUDRoot)
            {
                // Criar o widget do layout
                m_CoordenadasHUDRoot = GetGame().GetWorkspace().CreateWidgets("MyMaintenanceMod/GUI/layouts/CoordenadasRootPanel.layout");
                if (!m_CoordenadasHUDRoot)
                {
                    Print("[MyMaintenanceMod] ERRO: Não foi possível carregar o layout 'CoordenadasRootPanel.layout'. Verifique o caminho.");
                }
                else
                {
                    // Criar e anexar manualmente o script controlador
                    m_CoordenadasHUDController = new CoordenadasRootPanel();
                    m_CoordenadasHUDController.OnWidgetScriptInit(m_CoordenadasHUDRoot);
                    Print("[MyMaintenanceMod] HUD de coordenadas inicializado com sucesso.");
                }
            }

            // --- Inicialização do GPS ---
            if (!m_GPSHUDRoot)
            {
                // Criar o widget do layout GPS
                m_GPSHUDRoot = GetGame().GetWorkspace().CreateWidgets("MyMaintenanceMod/GUI/layouts/GPSRootPanel.layout");
                if (!m_GPSHUDRoot)
                {
                    Print("[MyMaintenanceMod] ERRO: Não foi possível carregar o layout 'GPSRootPanel.layout'. Verifique o caminho.");
                }
                else
                {
                    // Criar e anexar manualmente o script controlador do GPS
                    m_GPSHUDController = new GPSRootPanel();
                    m_GPSHUDController.OnWidgetScriptInit(m_GPSHUDRoot);
                    Print("[MyMaintenanceMod] GPS inicializado com sucesso.");
                }
            }
        }
    }

    override void OnMissionFinish()
    {
        if (m_CoordenadasHUDController)
        {
            delete m_CoordenadasHUDController;
            m_CoordenadasHUDController = null;
        }
        
        if (m_CoordenadasHUDRoot)
        {
            delete m_CoordenadasHUDRoot;
            m_CoordenadasHUDRoot = null;
        }

        if (m_GPSHUDController)
        {
            delete m_GPSHUDController;
            m_GPSHUDController = null;
        }
        
        if (m_GPSHUDRoot)
        {
            delete m_GPSHUDRoot;
            m_GPSHUDRoot = null;
        }

        // Limpar instância do HeliCrashMapMenu
        if (m_HeliCrashMapMenu)
        {
            m_HeliCrashMapMenu.Close();
            m_HeliCrashMapMenu = null;
        }

        super.OnMissionFinish();
    }

    // --- Sistema de Input para GPS e Coordenadas ---
    override void OnUpdate(float timeslice) 
    {
        super.OnUpdate(timeslice);
        
        if (GetGame().IsClient())
        {
            Input input = GetGame().GetInput();

            // Detectar tecla J para toggle do GPS
            if (input.LocalPress("UAToggleGPS", false)) 
            {
                if (m_GPSHUDController)
                {
                    m_GPSHUDController.ToggleGPS();
                    Print("[MissionGameplay] Toggle GPS acionado via tecla J");
                }
            }

            // Detectar tecla K para toggle das Coordenadas
            if (input.LocalPress("UAToggleCoords", false)) 
            {
                if (m_CoordenadasHUDController)
                {
                    m_CoordenadasHUDController.ToggleCoords();
                    Print("[MissionGameplay] Toggle Coordenadas acionado via tecla K");
                }
            }

            // Detectar tecla L para abrir Mapa de Crashes (com debounce)
            if (input.LocalPress("UAToggleCrashMap", false) && !m_CrashMapKeyPressed) 
            {
                // Ativar flag de debounce
                m_CrashMapKeyPressed = true;
                
                // Verificar se jogador tem acesso (igual ao RPC do MaintenanceStatusMenu)
                PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
                if (!player)
                {
                    // Reset flag se não há player
                    GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.ResetCrashMapKeyFlag, 500, false);
                    return;
                }

                EntityAI attachment = player.GetInventory().FindAttachment(InventorySlots.GetSlotIdFromString("Kdu_link"));
                if (!attachment || (!attachment.IsKindOf("DogTag_R_TypeFive") && !attachment.IsKindOf("DogTag_R_TypeFour")))
                {
                    // Mostrar mensagem de acesso negado
                    Print("[MissionGameplay] Acesso negado ao mapa de crashes - DogTag_R_TypeFour ou DogTag_R_TypeFive não encontrado");

                    // Reset flag após erro
                    GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.ResetCrashMapKeyFlag, 500, false);
                    return;
                }

                // Abrir o menu com delay para evitar race condition
                GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.OpenCrashMapDelayed, 100, false);
                Print("[MissionGameplay] Solicitação de mapa de crashes agendada");
            }

            // Detectar ESC para fechar menus
            if (GetUApi().GetInputByID(UAUIBack).LocalPress())
            {
                // Verificar se é o mapa de crashes
                HeliCrashMapMenu crashMapMenu;
                if (Class.CastTo(crashMapMenu, GetGame().GetUIManager().GetMenu()))
                {
                    crashMapMenu.Close();
                    m_HeliCrashMapMenu = null;
                    Print("[MissionGameplay] Mapa de crashes fechado via ESC");
                }
                else
                {
                    // Verificar se é o menu de manutenção da mesa
                    MaintenanceStatusMenu maintenanceMenu;
                    if (Class.CastTo(maintenanceMenu, GetGame().GetUIManager().GetMenu()))
                    {
                        maintenanceMenu.Close();
                        iMaintenanceStatusMenu = null;
                        Print("[MissionGameplay] Menu de manutenção fechado via ESC");
                    }
                }
            }
        }
    }
    
    // --- Funções auxiliares para debounce da tecla L ---
    private void OpenCrashMapDelayed()
    {
        // Se o menu já estiver aberto, fecha antes de abrir novo (igual ao MaintenanceStatusMenu)
        if (m_HeliCrashMapMenu)
            m_HeliCrashMapMenu.Close();
        
        // Criar nova instância e manter referência (igual ao MaintenanceStatusMenu)
        m_HeliCrashMapMenu = new HeliCrashMapMenu();
        GetGame().GetUIManager().ShowScriptedMenu(m_HeliCrashMapMenu, null);
        Print("[MissionGameplay] Menu de mapa de crashes aberto com sucesso");
        
        // Reset flag após abrir o menu
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.ResetCrashMapKeyFlag, 1000, false);
    }
    
    private void ResetCrashMapKeyFlag()
    {
        m_CrashMapKeyPressed = false;
        Print("[MissionGameplay] Flag de tecla L resetada - pronto para nova detecção");
    }
};
