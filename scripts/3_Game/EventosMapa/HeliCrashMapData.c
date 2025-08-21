/**
 * @class HeliCrashMapData
 * @brief Classe para serializar dados de crashes para RPC (igual MaintenanceStatusMenuData)
 */
class HeliCrashMapData
{
    ref array<vector> m_CrashPositions; // Array de posições dos crashes
    
    void HeliCrashMapData()
    {
        m_CrashPositions = new array<vector>();
    }
    
    void AddCrashPosition(vector position)
    {
        m_CrashPositions.Insert(position);
    }
    
    int GetCrashCount()
    {
        return m_CrashPositions.Count();
    }
}
