// Estrutura para armazenar os dados de uma única unidade de manutenção.
class MaintenanceUnitData
{
    int m_UnitID;
    string m_OwnerSteamID;
    vector m_Position;
    vector m_Orientation;
    ref array<string> m_Members;

    void MaintenanceUnitData(int id, string owner, vector pos, vector ori)
    {
        m_UnitID = id;
        m_OwnerSteamID = owner;
        m_Position = pos;
        m_Orientation = ori;
        m_Members = new array<string>;
    }
}

class MaintenanceUnitDatabase
{
    ref array<ref MaintenanceUnitData> m_AllUnitsData;
    int m_NextUnitID; // Contador para garantir IDs únicos

    void MaintenanceUnitDatabase()
    {
        m_AllUnitsData = new array<ref MaintenanceUnitData>;
        m_NextUnitID = 1;
    }
}
