class MaintenanceStatusMenu extends UIScriptedMenu
{
    // Declaração dos widgets do layout
    private TextWidget m_LevelValueText;
    private TextWidget m_StatusValueText;
    private TextWidget m_AutonomyValueText;
    private TextWidget m_ConstructionValueText;
    private TextWidget m_ContainerValueText;
    private TextWidget m_OwnerLabel;
    private MultilineTextWidget m_MembersListText;
    private ButtonWidget m_CloseButton;

    override Widget Init()
    {
        // Carrega o nosso novo layout
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("MyMaintenanceMod/GUI/layouts/MaintenanceStatusMenu.layout");

        // Vincula as variáveis aos widgets do layout pelos seus nomes
        m_LevelValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("level_value_text"));
        m_StatusValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("status_value_text"));
        m_AutonomyValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("autonomy_value_text"));
        m_ConstructionValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("construction_value_text"));
        m_ContainerValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("container_value_text"));
        m_OwnerLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("owner_label"));
        m_MembersListText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("members_list_text"));
        m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("close_button"));

        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);

        // Esconde a HUD do jogo para focar no menu
        GetGame().GetMission().GetHud().Show(false);
    }

    override void OnHide()
    {
        super.OnHide();
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor(false);

        // Mostra a HUD do jogo novamente ao fechar o menu
        GetGame().GetMission().GetHud().Show(true);
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

    // Esta função é a chave: ela recebe os dados do servidor e preenche a UI
    void SetData(MaintenanceStatusMenuData data)
    {
        if (!data) return;

        m_LevelValueText.SetText(data.m_Level);
        m_StatusValueText.SetText(data.m_Status);
        m_AutonomyValueText.SetText(data.m_TimeLeft);
        m_ConstructionValueText.SetText(data.m_ConstructionLimit);
        m_ContainerValueText.SetText(data.m_ContainerLimit);
        m_OwnerLabel.SetText("Dono: " + data.m_Owner);

        string membersText = "";
        if (data.m_Members && data.m_Members.Count() > 0)
        {
            foreach (string member : data.m_Members)
            {
                membersText = membersText + "- " + member + "\n";
            }
        }
        else
        {
            membersText = "Nenhum membro na equipe.";
        }
        m_MembersListText.SetText(membersText);
    }
}
