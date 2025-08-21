class GPSRootPanel : ScriptedWidgetEventHandler
{
    // --- Widgets ---
    private Widget      m_RootWidget;           // O widget principal que contém todo o GPS
    private Widget      m_GPSContainer;         // Container do mapa (com clipchildren)
    private MapWidget   m_MapWidget;            // O widget nativo do mapa
    private ImageWidget m_PlayerIndicator;      // O indicador fixo do jogador

    // --- Estado ---
    private bool        m_ShouldDisplay;        // Flag que controla se o GPS deve ser exibido
    private bool        m_IsInitialized;        // Flag para prevenir a execução antes da inicialização
    private bool        m_IsManuallyHidden;     // Flag para controle manual do usuário

    // --- Construtor e Destrutor ---
    void GPSRootPanel()
    {
        // Usar CallLater como o MiniMap que funciona
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.OnGPSUpdate, 200, true);
    }

    void ~GPSRootPanel()
    {
        // Remove o CallLater ao destruir o objeto
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.OnGPSUpdate);
    }

    void OnWidgetScriptInit(Widget w)
    {
        m_RootWidget = w;
        
        // Esconde o GPS inicialmente e define o estado inicial
        m_RootWidget.Show(false);
        m_ShouldDisplay = false;
        m_IsManuallyHidden = false;

        // Encontra os widgets do GPS
        m_GPSContainer = Widget.Cast(m_RootWidget.FindAnyWidget("GPSContainer"));
        m_MapWidget = MapWidget.Cast(m_RootWidget.FindAnyWidget("MapWidget"));
        m_PlayerIndicator = ImageWidget.Cast(m_RootWidget.FindAnyWidget("PlayerIndicator"));

        if (!m_GPSContainer)
        {
            Print("[GPSRootPanel] ERRO: Nao foi possivel encontrar o widget GPSContainer no layout!");
            return;
        }

        if (!m_MapWidget)
        {
            Print("[GPSRootPanel] ERRO: Nao foi possivel encontrar o widget MapWidget no layout!");
            return;
        }

        if (!m_PlayerIndicator)
        {
            Print("[GPSRootPanel] ERRO: Nao foi possivel encontrar o widget PlayerIndicator no layout!");
            return;
        }

        // Configurar o mapa para múltiplos mapas
        ConfigureMapForMultipleMaps();

        // Marca o script como totalmente inicializado
        m_IsInitialized = true;
        Print("[GPSRootPanel] GPS inicializado com sucesso para multi-mapas!");
    }

    private void ConfigureMapForMultipleMaps()
    {
        Print("[GPSRootPanel] DEBUG: Configurando MapWidget para multi-mapas");
        
        // Configurar o mapa nativo do DayZ
        m_MapWidget.SetMapPos(Vector(7680, 0, 7680)); // Centro padrão
        m_MapWidget.SetScale(0.1); // Zoom inicial padrão
        
        Print("[GPSRootPanel] DEBUG: MapWidget configurado - Centro: 7680,7680 | Escala: 0.1");
    }

    private void CheckEligibility()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player && player.IsAlive())
        {
            EntityAI attachment = player.GetInventory().FindAttachment(InventorySlots.GetSlotIdFromString("Kdu_link"));
            if (attachment)
            {
                // Nova lógica: TypeThree ou superior (3, 4, 5) para GPS
                if (attachment.IsKindOf("DogTag_R_TypeFive") || attachment.IsKindOf("DogTag_R_TypeFour") || attachment.IsKindOf("DogTag_R_TypeThree"))
                {
                    m_ShouldDisplay = true;
                    
                    // Só mostra se não estiver manualmente escondido
                    if (!m_IsManuallyHidden)
                    {
                        m_RootWidget.Show(true);
                    }
                    return;
                }
            }
        }

        // Se qualquer verificação falhar, o GPS é desativado e escondido
        m_ShouldDisplay = false;
        m_RootWidget.Show(false);
        m_IsManuallyHidden = false; // Reset do controle manual
    }

    void OnGPSUpdate()
    {
        // Primeiro verifica se deve exibir o GPS (DogTag check)
        CheckEligibility();
        
        if (!m_IsInitialized || !m_ShouldDisplay || !m_MapWidget)
            return;

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;

        vector map_pos, camera_pos;
        string worldName;
        float scale = 0.1;
        float camera_x, camera_y;
        float shift_x = 0.0, shift_y = 0.0;
        int multiplier;

        GetGame().GetWorldName(worldName);
        worldName.ToLower();
        
        camera_pos = GetGame().GetCurrentCameraPosition();
        camera_x = camera_pos[0];
        camera_y = camera_pos[2];
        
        multiplier = Math.Round(scale * 10);
        
        // Configurações específicas para diferentes mapas
        if (worldName.Contains("chernarus"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }           
        }
        else if (worldName.Contains("enoch"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("namalsk"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("deerisle"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("rostow"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("valning"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("chiemsee"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("takistanplus"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        else if (worldName.Contains("banov"))
        {
            if (scale >= 0.1)
            {   
                shift_x = 0.0;
                shift_y = 0.0;
            }       
        }
        
        camera_x = camera_x + (shift_x * multiplier);
        camera_y = camera_y - (shift_y * multiplier);
        map_pos = Vector(camera_x, 0, camera_y);

        // Aplicar configurações
        m_MapWidget.SetMapPos(map_pos);
        m_MapWidget.SetScale(scale);
        
        m_MapWidget.ClearUserMarks();
        m_MapWidget.AddUserMark(player.GetPosition(), "Me", ARGB(255,255,255,0), "MyMaintenanceMod\\GUI\\images\\playermarker.paa");

        // DEBUG reduzido
        static int debugCounter = 0;
        debugCounter++;
        if (debugCounter % 25 == 0) // Log a cada 5 segundos
        {
            Print("[GPS DEBUG] World: " + worldName + " | Pos: " + map_pos);
        }
    }

    void ToggleGPS()
    {
        if (!m_ShouldDisplay) return; // Só funciona se tiver DogTag
        
        m_IsManuallyHidden = !m_IsManuallyHidden;
        
        if (m_IsManuallyHidden)
        {
            m_RootWidget.Show(false);
            Print("[GPSRootPanel] GPS desativado pelo jogador (tecla J)");
        }
        else
        {
            m_RootWidget.Show(true);
            Print("[GPSRootPanel] GPS ativado pelo jogador (tecla J)");
        }
    }

    void ShowGPS()
    {
        if (m_RootWidget)
        {
            m_RootWidget.Show(true);
            Print("[GPSRootPanel] GPS exibido!");
        }
    }

    void HideGPS()
    {
        if (m_RootWidget)
        {
            m_RootWidget.Show(false);
            Print("[GPSRootPanel] GPS escondido!");
        }
    }
};