/**
 * @class HeliCrashMapMenu
 * @brief Menu para o mapa de HeliCrashes usando UIScriptedMenu
 */
class HeliCrashMapMenu extends UIScriptedMenu
{
    // --- Widgets ---
    private Widget      m_CrashMapContainer;    // Container do mapa (delimitador)
    private MapWidget   m_CrashMapWidget;       // Widget do mapa
    private TextWidget  m_CrashMapTitle;        // Título
    private TextWidget  m_CrashMapInfo;         // Info
    private TextWidget  m_CrashMapCounter;      // Contador de crashes
    private ButtonWidget m_CloseButton;         // Botão fechar



    override Widget Init()
    {
        // Carrega o layout do mapa de crashes
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("MyMaintenanceMod/GUI/layouts/HeliCrashMapMenu.layout");

        // Vincula as variáveis aos widgets do layout pelos seus nomes
        m_CrashMapContainer = Widget.Cast(layoutRoot.FindAnyWidget("CrashMapContainer"));
        m_CrashMapWidget = MapWidget.Cast(layoutRoot.FindAnyWidget("CrashMapWidget"));
        m_CrashMapTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("CrashMapTitle"));
        m_CrashMapInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("CrashMapInfo"));
        m_CrashMapCounter = TextWidget.Cast(layoutRoot.FindAnyWidget("CrashMapCounter"));
        m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CrashMapCloseButton"));

        if (!m_CrashMapWidget)
        {
            Print("[HeliCrashMapMenu] ERRO: MapWidget não encontrado!");
            return layoutRoot;
        }

        if (!m_CrashMapContainer)
        {
            Print("[HeliCrashMapMenu] AVISO: Container do mapa não encontrado - mapa pode sair dos limites!");
        }

        // Configurar mapa
        ConfigureCrashMap();

        Print("[HeliCrashMapMenu] Menu de mapa de crashes inicializado!");

        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);

        // Esconde a HUD do jogo para focar no mapa
        GetGame().GetMission().GetHud().Show(false);

        // Solicitar sincronização de crashes do servidor primeiro
        GetRPCManager().SendRPC("MyMaintenanceMod", "SyncHeliCrashes", new Param1<bool>(true), true, null);
        
        // Solicitar sincronização de eventos militares do servidor
        GetRPCManager().SendRPC("MyMaintenanceMod", "SyncMilitaryEvents", new Param1<bool>(true), true, null);
        
        // Atualizar markers quando abrir (com delay maior para garantir sincronização)
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.UpdateCrashMarkers, 1000, false);

        Print("[HeliCrashMapMenu] Mapa de crashes aberto");
    }

    override void OnHide()
    {
        super.OnHide();
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor(false);

        // Mostra a HUD do jogo novamente ao fechar
        GetGame().GetMission().GetHud().Show(true);

        Print("[HeliCrashMapMenu] Mapa de crashes fechado");
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);

        if (w == m_CloseButton)
        {
            Close();
            return true;
        }

        return false;
    }

    private void ConfigureCrashMap()
    {
        // Configurar mapa para visão geral
        m_CrashMapWidget.SetMapPos(Vector(7680, 0, 7680)); // Centro do mapa
        m_CrashMapWidget.SetScale(0.05); // Zoom bem aberto para ver todo o mapa
        
        Print("[HeliCrashMapMenu] Mapa configurado para visão geral");
    }

    /**
     * Verifica o nível de acesso do jogador baseado no DogTag
     * @return int Nível de acesso (2-5) ou 0 se não tiver acesso
     */
    private int GetPlayerAccessLevel()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
        {
            Print("[HeliCrashMapMenu] Jogador não encontrado");
            return 0;
        }

        Print("[HeliCrashMapMenu] Player encontrado: " + (player != null));

        // Verificar DogTag no attachment Kdu_link
        EntityAI attachment = player.GetInventory().FindAttachment(InventorySlots.GetSlotIdFromString("Kdu_link"));
        
        Print("[HeliCrashMapMenu] Attachment encontrado: " + (attachment != null));
        if (attachment) 
        {
            Print("[HeliCrashMapMenu] Tipo do attachment: " + attachment.GetType());
        }

        if (attachment)
        {
            if (attachment.IsKindOf("DogTag_R_TypeFour"))  
            {
                Print("[HeliCrashMapMenu] DogTag TypeFour detectado!");
                return 4;
            }
            if (attachment.IsKindOf("DogTag_R_TypeFive"))  
            {
                Print("[HeliCrashMapMenu] DogTag TypeFive detectado!");
                return 5;
            }
        }

        Print("[HeliCrashMapMenu] Nenhum DogTag encontrado no Kdu_link - acesso negado");
        return 0;
    }

    private void UpdateCrashMarkers()
    {
        if (!m_CrashMapWidget)
        {
            Print("[HeliCrashMapMenu] ERRO: MapWidget não está disponível para atualizar markers");
            return;
        }

        // Verificar nível de acesso do jogador
        int accessLevel = GetPlayerAccessLevel();

        // Limpar markers existentes
        m_CrashMapWidget.ClearUserMarks();
        Print("[HeliCrashMapMenu] Markers do mapa limpos");

        // ADICIONADO: Marcador da posição do jogador
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            vector playerPos = player.GetPosition();
            m_CrashMapWidget.AddUserMark(playerPos, "VOCÊ ESTÁ AQUI", ARGB(255,0,255,0), "MyMaintenanceMod\\GUI\\images\\playermarker.paa");
            Print("[HeliCrashMapMenu] Marcador do jogador adicionado na posição: " + playerPos);
        }

        // Obter posições dos crashes sincronizadas do servidor
        array<vector> crashPositions = HeliCrashTracker.GetCrashPositions();
        
        // Obter posições dos eventos militares
        array<vector> militaryEvents = MilitaryEventTracker.GetEventPositions();
        
        int totalMarkers = 0;
        
        // Adicionar markers de helicrashes (só TypeFive pode ver)
        if (crashPositions && crashPositions.Count() > 0 && accessLevel >= 5)
        {
            Print("[HeliCrashMapMenu] Recebidas " + crashPositions.Count() + " posições de helicrashes");
            
            foreach (vector crashPos : crashPositions)
            {
                string crashLabel = "Emergência " + (totalMarkers + 1);
                m_CrashMapWidget.AddUserMark(crashPos, crashLabel, ARGB(255,255,50,50), "MyMaintenanceMod\\GUI\\images\\crashmarker.paa");
                totalMarkers++;
            }
        }
        
        // Adicionar markers de eventos militares (TypeFour+ pode ver)
        if (militaryEvents && militaryEvents.Count() > 0 && accessLevel >= 4)
        {
            Print("[HeliCrashMapMenu] Recebidas " + militaryEvents.Count() + " posições de eventos militares");
            
            foreach (vector eventPos : militaryEvents)
            {
                string eventLabel = "Convoy " + (totalMarkers + 1);
                m_CrashMapWidget.AddUserMark(eventPos, eventLabel, ARGB(255,255,165,0), "MyMaintenanceMod\\GUI\\images\\militarymarker.paa");
                totalMarkers++;
            }
        }

        // Atualizar texto de info baseado no nível de acesso
        if (m_CrashMapInfo)
        {
            string infoText;
            if (accessLevel >= 5)
            {
                infoText = "[TypeFive] Sinais Recebidos:";
            }
            else if (accessLevel >= 4)
            {
                infoText = "[TypeFour] Sinais Recebidos:";
            }
            m_CrashMapInfo.SetText(infoText);
        }

        // Atualizar contador com quantidade total
        if (m_CrashMapCounter)
        {
            m_CrashMapCounter.SetText(totalMarkers.ToString());
            Print("[HeliCrashMapMenu] Contador atualizado: " + totalMarkers + " eventos totais");
        }
        
        if (totalMarkers == 0)
        {
            Print("[HeliCrashMapMenu] Nenhum evento encontrado");
        }
        else
        {
            Print("[HeliCrashMapMenu] " + totalMarkers + " eventos marcados no mapa com sucesso");
        }
    }
}
