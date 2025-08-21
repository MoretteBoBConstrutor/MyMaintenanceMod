/**
 * @class CoordenadasRootPanel
 * @brief Controlador do HUD de coordenadas. O nome da classe DEVE ser o mesmo da classe do widget raiz no .layout.
 */
class CoordenadasRootPanel extends ScriptedWidgetEventHandler
{
    // --- Widgets ---
    private Widget      m_RootWidget;           // O widget principal que contém todo o HUD. Usado para mostrar/esconder.
    private TextWidget  m_CoordsTextWidget;     // O widget de texto que exibirá as coordenadas.

    // --- Timers de Otimização ---
    private const float SLOW_TICK_RATE = 1.0;   // Frequência do timer lento (1 vez por segundo).
    private const float FAST_TICK_RATE = 0.1;   // Frequência do timer rápido (10 vezes por segundo).
    private float       m_SlowTickTimer;        // Acumulador de tempo para o timer lento.
    private float       m_FastTickTimer;        // Acumulador de tempo para o timer rápido.

    // --- Estado ---
    private bool        m_ShouldDisplay;        // Flag que controla se o HUD deve ser exibido.
    private bool        m_IsInitialized;        // Flag para prevenir a execução do Update() antes da inicialização completa.
    private bool        m_IsManuallyHidden;     // Flag para controle manual do usuário

    // --- Construtor e Destrutor ---
    void CoordenadasRootPanel()
    {
        // CORREÇÃO: Registra o método 'Update' na fila de atualização da GUI.
        // Esta é a forma garantida de ter uma função de atualização executada a cada frame.
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert(this.Update);
    }

    void ~CoordenadasRootPanel()
    {
        // CORREÇÃO: Remove o método da fila de atualização ao destruir o objeto para evitar erros.
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove(this.Update);
    }

    /**
     * @brief Chamado quando o script do widget é inicializado.
     *        Configura as referências dos widgets e o estado inicial.
     */
    void OnWidgetScriptInit(Widget w)
    {
        m_RootWidget = w;
        
        // CORREÇÃO: Esconde o HUD e define o estado inicial ANTES de qualquer outra coisa.
        // Isso garante que o HUD nunca apareça visível por padrão, mesmo que a busca por widgets filhos falhe.
        m_RootWidget.Show(false);
        m_ShouldDisplay = false;
        m_IsManuallyHidden = false;

        // Agora, tenta encontrar o widget de texto.
        m_CoordsTextWidget = TextWidget.Cast(m_RootWidget.FindAnyWidget("CoordenadasTextoWidget"));

        if (!m_CoordsTextWidget)
        {
            // Se a busca falhar, apenas imprime o erro. O HUD já está escondido, então o bug visual não ocorre.
            Print("[CoordenadasRootPanel] ERRO: Não foi possível encontrar o widget 'CoordenadasTextoWidget' no layout!");
            return; // Retorna para garantir que não marcamos como inicializado se o widget de texto não for encontrado.
        }

        // Marca o script como totalmente inicializado somente após encontrar todos os widgets necessários.
        m_IsInitialized = true;
    }

    /**
     * @brief Chamado a cada frame. Gerencia os timers e a lógica de atualização.
     */
    void Update(float timeslice)
    {
        // CORREÇÃO: Não executa a lógica de atualização até que OnWidgetScriptInit tenha sido concluído com sucesso.
        if (!m_IsInitialized)
            return;

        // --- Lógica do Timer Lento ---
        m_SlowTickTimer += timeslice;
        if (m_SlowTickTimer >= SLOW_TICK_RATE)
        {
            m_SlowTickTimer = 0;
            CheckEligibility(); // Verifica se o jogador tem a DogTag
        }

        // Se o jogador não for elegível, não fazemos mais nada.
        if (!m_ShouldDisplay)
        {
            return;
        }

        // --- Lógica do Timer Rápido ---
        m_FastTickTimer += timeslice;
        if (m_FastTickTimer >= FAST_TICK_RATE)
        {
            m_FastTickTimer = 0;
            UpdateCoordinates(); // Atualiza o texto das coordenadas
        }
    }

    /**
     * @brief (SLOW TICK) Verifica se o jogador possui DogTag adequada equipada.
     *        Atualiza a flag m_ShouldDisplay e a visibilidade do HUD.
     */
    private void CheckEligibility()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player && player.IsAlive())
        {
            EntityAI attachment = player.GetInventory().FindAttachment(InventorySlots.GetSlotIdFromString("Kdu_link"));
            if (attachment)
            {
                // Nova lógica: TypeTwo ou superior (2, 3, 4, 5) para Coordenadas
                if (attachment.IsKindOf("DogTag_R_TypeFive") || attachment.IsKindOf("DogTag_R_TypeFour") || attachment.IsKindOf("DogTag_R_TypeThree") ||attachment.IsKindOf("DogTag_R_TypeTwo"))
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

        // Se qualquer verificação falhar, o HUD é desativado e escondido.
        m_ShouldDisplay = false;
        m_RootWidget.Show(false);
        m_IsManuallyHidden = false; // Reset do controle manual
    }

    /**
     * @brief (FAST TICK) Obtém a posição do jogador e atualiza o widget de texto.
     */
    private void UpdateCoordinates()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;

        vector pos = player.GetPosition();
        string coordsText = "X: " + Math.Round(pos[0]) + " | Z: " + Math.Round(pos[2]);
        m_CoordsTextWidget.SetText(coordsText);
    }

    /**
     * @brief Toggle manual das coordenadas pelo jogador
     */
    void ToggleCoords()
    {
        if (!m_ShouldDisplay) return; // Só funciona se tiver DogTag
        
        m_IsManuallyHidden = !m_IsManuallyHidden;
        
        if (m_IsManuallyHidden)
        {
            m_RootWidget.Show(false);
            Print("[CoordenadasRootPanel] Coordenadas desativadas pelo jogador (tecla K)");
        }
        else
        {
            m_RootWidget.Show(true);
            Print("[CoordenadasRootPanel] Coordenadas ativadas pelo jogador (tecla K)");
        }
    }
}