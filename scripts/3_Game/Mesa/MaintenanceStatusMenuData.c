// Esta classe atua como um container para enviar todos os dados da unidade para o menu
class MaintenanceStatusMenuData : MenuData
{
    string m_Level;
    string m_Status;
    string m_TimeLeft;
    string m_ConstructionLimit;
    string m_ContainerLimit;
    string m_Owner;
    ref array<string> m_Members;

    void MaintenanceStatusMenuData()
    {
        m_Members = new array<string>;
    }
}
