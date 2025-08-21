class MaintenanceUnitManager
{
    private static ref MaintenanceUnitManager m_Instance;
    private ref MaintenanceUnitDatabase m_Database;
    private const string JSON_PATH = "$profile:MaintenanceMod/UnitDatabase.json";

    // Singleton pattern para garantir uma única instância do gerenciador.
    static MaintenanceUnitManager GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new MaintenanceUnitManager();
        }
        return m_Instance;
    }

    private void MaintenanceUnitManager()
    {
        m_Database = new MaintenanceUnitDatabase();
    }

    // Carrega os dados do arquivo JSON.
    void LoadData()
    {
        if (FileExist(JSON_PATH))
        {
            JsonFileLoader<MaintenanceUnitDatabase>.JsonLoadFile(JSON_PATH, m_Database);
            MMLogger.Log("[Manager] Banco de dados de unidades carregado com " + m_Database.m_AllUnitsData.Count() + " unidades.");
        }
        else
        {
            MMLogger.Log("[Manager] Arquivo de banco de dados não encontrado. Um novo será criado.");
        }
    }

    // Salva os dados no arquivo JSON.
    void SaveData()
    {
        if (!FileExist("$profile:MaintenanceMod/"))
        {
            MakeDirectory("$profile:MaintenanceMod/");
        }
        JsonFileLoader<MaintenanceUnitDatabase>.JsonSaveFile(JSON_PATH, m_Database);
        MMLogger.Log("[Manager] Banco de dados de unidades salvo.");
    }

    // Registra uma nova unidade, atribui um ID e retorna os dados.
    MaintenanceUnitData RegisterNewUnit(vector pos, vector ori)
    {
        int newID = m_Database.m_NextUnitID;
        m_Database.m_NextUnitID++; // Incrementa para o próximo
        
        MaintenanceUnitData newData = new MaintenanceUnitData(newID, "", pos, ori);
        m_Database.m_AllUnitsData.Insert(newData);
        
        MMLogger.Log("[Manager] Nova unidade registrada com ID: " + newID);
        SaveData(); // Salva imediatamente
        return newData;
    }

    // Remove uma unidade do banco de dados pelo seu ID.
    void RemoveUnit(int unitID)
    {
        for (int i = 0; i < m_Database.m_AllUnitsData.Count(); i++)
        {
            if (m_Database.m_AllUnitsData[i].m_UnitID == unitID)
            {
                m_Database.m_AllUnitsData.Remove(i);
                MMLogger.Log("[Manager] Unidade com ID: " + unitID + " removida do banco de dados.");
                SaveData();
                return;
            }
        }
    }

    // Encontra os dados de uma unidade pela sua posição (usado na inicialização).
    MaintenanceUnitData FindUnitDataByPos(vector pos)
    {
        foreach (MaintenanceUnitData data : m_Database.m_AllUnitsData)
        {
            // Usamos uma pequena tolerância para a comparação de vetores.
            if (vector.Distance(data.m_Position, pos) < 0.1)
            {
                return data;
            }
        }
        return null;
    }
    
    // Atualiza o dono de uma unidade específica.
    void UpdateOwner(int unitID, string ownerID)
    {
        foreach (MaintenanceUnitData data : m_Database.m_AllUnitsData)
        {
            if (data.m_UnitID == unitID)
            {
                data.m_OwnerSteamID = ownerID;
                MMLogger.Log("[Manager] Proprietário da unidade ID " + unitID + " atualizado para " + ownerID);
                SaveData();
                return;
            }
        }
    }

    /**
 * @brief Atualiza a lista de membros de uma unidade específica no banco de dados.
 * @param unitID O ID da unidade a ser atualizada.
 * @param members A nova lista completa de membros.
 */
    void UpdateMembers(int unitID, array<string> members)
    {
        if (!m_Database || !members) // Adicionada verificação de 'members' nulo
            return;

        MaintenanceUnitData data = FindUnitDataByID(unitID); // Agora esta função existe
        if (data)
        {
            data.m_Members = members;
            SaveData(); // Nome da função corrigido
            MMLogger.Log("[Manager] Membros da unidade ID " + unitID + " atualizados. Total: " + members.Count());
        }
        else
        {
            MMLogger.Log("[Manager] ERRO: Tentativa de atualizar membros para uma unidade não encontrada no banco de dados: ID " + unitID);
        }
    }

    /**
     * @brief Verifica no banco de dados se um jogador já é dono OU membro de QUALQUER unidade.
     * @param playerID O Steam64ID do jogador a ser verificado.
     * @return true se o jogador já estiver em uma equipe, false caso contrário.
     */
    bool IsPlayerInAnyTeam(string playerID)
    {
        if (playerID == "")
            return false;

        // Acessa a lista de dados de todas as unidades do banco de dados
        array<ref MaintenanceUnitData> allUnitData = m_Database.m_AllUnitsData;

        if (!allUnitData)
            return false;

        // Itera por cada unidade no banco de dados
        foreach (MaintenanceUnitData unitData : allUnitData)
        {
            // 1. Verifica se o jogador é o dono desta unidade
            if (unitData.m_OwnerSteamID == playerID)
            {
                return true; // Encontrado como dono, não precisa procurar mais
            }

            // 2. Verifica se o jogador está na lista de membros desta unidade
            if (unitData.m_Members && unitData.m_Members.Find(playerID) != -1)
            {
                return true; // Encontrado como membro, não precisa procurar mais
            }
        }

        // Se o loop terminar, o jogador não foi encontrado em nenhuma equipe
        return false;
    }

    // Encontra os dados de uma unidade pelo seu ID.
    MaintenanceUnitData FindUnitDataByID(int unitID)
    {
        foreach (MaintenanceUnitData data : m_Database.m_AllUnitsData)
        {
            if (data.m_UnitID == unitID)
            {
                return data;
            }
        }
        return null;
    }

    // Verifica no banco de dados se um jogador já possui uma unidade.
    bool IsPlayerAlreadyOwner(string playerID)
    {
        if (playerID == "") 
            return false;
            
        foreach (MaintenanceUnitData data : m_Database.m_AllUnitsData)
        {
            if (data.m_OwnerSteamID == playerID)
                return true; // Encontrou uma unidade que pertence a este jogador.
        }

        return false; // O jogador não possui nenhuma unidade.
    }

   /**
     * @brief Verifica se a proteção de uma unidade de manutenção está desativada para um atacante específico.
     * A proteção é desativada se o atacante usar uma DogTag (do dono OU de um membro) em um Jammer ativo próximo à unidade.
     * 
     * @param raidingTag A DogTag que o atacante está usando para o raid.
     * @param basePartPosition A posição da parte da base que está sendo atacada.
     * @return true se a proteção estiver desativada (raid permitido), false caso contrário.
     */
    /**
     * @brief Verifica se a proteção de uma base em uma determinada posição está desativada por um Jammer.
     * Esta é a função central que determina se um raid é possível.
     * 
     * @param basePartPosition A posição da parte da base que está sendo atacada.
     * @return true se a proteção estiver desativada (raid permitido), false caso contrário.
     */
    bool IsProtectionDisabled(vector basePartPosition)
    {
        // 1. Encontra a unidade de manutenção que protege a área da peça da base.
        MaintenanceUnit targetUnit = GetUnitProtectingPosition(basePartPosition);
        if (!targetUnit)
        {
            // Se nenhuma unidade protege esta área, não há proteção para desativar.
            return false; 
        }

        // 2. Procura por um Jammer ATIVO no raio de proteção da unidade de manutenção.
        float checkRadius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
        array<Object> nearbyObjects = new array<Object>;
        GetGame().GetObjectsAtPosition3D(targetUnit.GetPosition(), checkRadius, nearbyObjects, null);

        foreach (Object obj : nearbyObjects)
        {
            Jammer activeJammer = Jammer.Cast(obj);
            if (activeJammer && activeJammer.IsActive() && (Math.AbsFloat(activeJammer.GetPosition()[1] - targetUnit.GetPosition()[1]) <= checkRadius))
            {
                // 3. Encontramos um Jammer ativo. Agora, pegamos o ID do alvo da DogTag dentro dele.
                string raidingTagOwnerId = activeJammer.GetTargetOwnerID();

                // 4. A verificação crucial: o dono da DogTag no Jammer é o dono da unidade ou um membro?
                if (targetUnit.IsMemberOrOwner(raidingTagOwnerId))
                {
                    MMLogger.Log("[IsProtectionDisabled] Jammer ativo encontrado com DogTag de membro/dono (" + raidingTagOwnerId + "). Proteção DESATIVADA.");
                    return true; // SIM, a proteção está desativada. Encontramos um Jammer válido.
                }
            }
        }
        
        // 5. Se o loop terminar e não encontrarmos um Jammer válido, a proteção continua ativa.
        MMLogger.Log("[IsProtectionDisabled] Nenhuma condição de Jammer válida encontrada. A proteção continua ATIVA.");
        return false;
    }

    /**
     * @brief Encontra a unidade de manutenção que protege uma determinada posição.
     * @param position A posição a ser verificada.
     * @return A instância da MaintenanceUnit se encontrada, caso contrário null.
     */
    MaintenanceUnit GetUnitProtectingPosition(vector position)
    {
        float protectionRadius = MaintenanceConfigManager.GetInstance().GetMaintenanceRadius();
        foreach (MaintenanceUnit unit : MaintenanceUnit.GetAllUnits())
        {
            if (unit.HasOwner() && vector.Distance(position, unit.GetPosition()) <= protectionRadius && (Math.AbsFloat(unit.GetPosition()[1] - position[1]) <= protectionRadius))
            {
                return unit;
            }
        }
        return null;
    }

};
