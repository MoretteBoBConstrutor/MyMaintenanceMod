modded class PlayerBase
{
    // Timers
    private ref Timer m_SurvivalTimer;
    
    private bool m_HasMaintenanceUnit; // Variável para rastrear se o jogador possui uma unidade de manutenção
    static const int DOGTAG_UPGRADE_TIME = 10 * 60 * 1000; // 10 minuto

    void PlayerBase()
    {
        RegisterNetSyncVariableBool("m_HasMaintenanceUnit");
    }

    bool HasMaintenanceUnit()
    {
        return m_HasMaintenanceUnit;
    }

    void SetHasMaintenanceUnit(bool hasUnit)
    {
        if (GetGame().IsServer())
        {
            if (m_HasMaintenanceUnit != hasUnit)
            {
                m_HasMaintenanceUnit = hasUnit;
                SetSynchDirty();
            }
        }
    }

    // Função para buscar a DogTag no slot dedicado. Agora retorna a classe base.
    DogTag_Base GetDogTag()
    {
        EntityAI attachment = FindAttachmentBySlotName("Kdu_link");
        if (attachment)
        {
            return DogTag_Base.Cast(attachment);
        }
        return null;
    }
 
    // Cria e anexa uma DogTag NÃO REGISTRADA ao jogador.
    void GivePlayerDogTag()
    {
        // Cria a tag inicial, que não tem valor.
        DogTag_Unregistered tag = DogTag_Unregistered.Cast(GetInventory().CreateAttachment("DogTag_Unregistered"));
        if (tag)
        {
            MMLogger.Log("DogTag não registrada criada para " + GetIdentity().GetName());
        }
        else 
        {
            MMLogger.Log("ERRO: Falha ao criar DogTag para " + GetIdentity().GetName());
        }
    }

    // Inicia o timer para o upgrade da DogTag.
    void StartDogTagUpgradeTimer()
    {
        // Garante que isso só rode no servidor.
        if (!GetGame().IsServer()) return;

        // Agenda a função 'PerformDogTagUpgrade' para ser chamada após DOGTAG_UPGRADE_TIME.
        // O 'false' no final significa que não vai repetir.
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.PerformDogTagUpgrade, DOGTAG_UPGRADE_TIME, false);
        MMLogger.Log("Timer de upgrade de DogTag iniciado para " + GetIdentity().GetName() + ". Duração: " + (DOGTAG_UPGRADE_TIME / 1000) + "s");
    }

    // Função que executa o upgrade.
    void PerformDogTagUpgrade()
    {
        // Verifica se o jogador ainda está vivo e online.
        if (!IsAlive() || !GetIdentity()) return;

        DogTag_Base currentTag = GetDogTag();
        if (currentTag && currentTag.IsKindOf("DogTag_Unregistered"))
        {
            // **SOLUÇÃO CORRIGIDA:** Armazena a facção da tag antiga ANTES de deletá-la.
            // A facção é definida na tag não registrada pelo MissionServer logo após o login.
#ifdef EXPANSIONMODAI        
            string factionName = currentTag.GetFactionName();
#endif
            MMLogger.Log("Realizando upgrade da DogTag para " + GetIdentity().GetName());
            
            GetGame().ObjectDelete(currentTag);

            DogTag_Registered newTag = DogTag_Registered.Cast(GetInventory().CreateAttachment("DogTag_Registered"));
            if (newTag)
            {
                newTag.SetOwnerID(GetIdentity().GetPlainId());
                newTag.SetNickname(GetIdentity().GetName());
#ifdef EXPANSIONMODAI
                newTag.SetFactionName(factionName); // Usa a facção armazenada
#endif

                MMLogger.Log("[Survival] DogTag de " + GetIdentity().GetName() + " sincronizada para Registrada.");

                // --- INÍCIO DA NOVA LÓGICA ---
                // Reseta os dados de sobrevivência e inicia o rastreio
                MMS_PlayerData pData = MMS_PlayerDatabaseManager.GetInstance().GetOrCreatePlayerData(GetIdentity().GetPlainId());
                pData.m_SurvivalTime = 0;
                pData.m_SurvivalLevel = 0; // Nível 0 = DogTag_Registered base

                StartSurvivalTracking();
                // --- FIM DA NOVA LÓGICA ---
            }
        }
    }

    // --- INÍCIO: NOVAS FUNÇÕES PARA RASTREIO DE VIDA ---

    // Inicia o timer de contagem de tempo de vida
    void StartSurvivalTracking()
    {
        if (GetGame().IsServer())
        {
            if (!m_SurvivalTimer)
            {
                m_SurvivalTimer = new Timer();
            }

            // Roda a cada 60 segundos para atualizar o progresso.
            if (!m_SurvivalTimer.IsRunning())
            {
                m_SurvivalTimer.Run(60, this, "UpdateSurvivalProgress", null, true);
                MMLogger.Log("[Survival] Rastreio de vida iniciado para: " + GetIdentity().GetName());
            }
        }
    }

    // Callback do timer de sobrevivência
    void UpdateSurvivalProgress()
    {
        if (!GetGame().IsServer() || !GetIdentity() || !IsAlive()) return;

        string playerID = GetIdentity().GetPlainId();
        MMS_PlayerData pData = MMS_PlayerDatabaseManager.GetInstance().GetOrCreatePlayerData(playerID);

        pData.m_SurvivalTime += 60; // Adiciona 60 segundos

        // Verifica por upgrades, do maior para o menor
        int currentLevel = pData.m_SurvivalLevel;

        // --- LÓGICA DE TEMPO PARA TESTE ---
        // Para testar, vamos usar minutos em vez de horas. Cada nível levará 1 minuto.
        // Para voltar para produção, comente a linha de minutos e descomente a de horas, e troque a variável nos 'if' abaixo.
        //float survivalMinutes = pData.m_SurvivalTime / 60.0; // TESTE: 1, 2, 3, 4, 5 minutos
        float survivalHours = pData.m_SurvivalTime / 3600.0; // PRODUÇÃO: 1, 2, 3, 4, 5 horas

        int newLevel = -1;

        if (survivalHours >= 5 && survivalHours == 4) newLevel = 5;
        else if (survivalHours >= 4 && survivalHours == 3) newLevel = 4;
        else if (survivalHours >= 3 && survivalHours == 2) newLevel = 3;
        else if (survivalHours >= 2 && survivalHours == 1) newLevel = 2;
        else if (survivalHours >= 1 && survivalHours == 0) newLevel = 1;

        if (newLevel != -1)
        {
            PerformSurvivalUpgrade(newLevel);
        }
    }

    // Executa o upgrade da DogTag para um novo nível
    void PerformSurvivalUpgrade(int newLevel)
    {
        if (!GetGame().IsServer() || !GetIdentity()) return;

        DogTag_Base oldTag = GetDogTag();
        if (!oldTag) return;

        // **SOLUÇÃO:** Armazena todos os dados da tag antiga ANTES de deletá-la.
        string ownerName   = oldTag.GetNickname();
        string ownerID     = oldTag.GetOwnerID();
#ifdef EXPANSIONMODAI
        string factionName = oldTag.GetFactionName();
#endif
        string newTagType;
        switch(newLevel)
        {
            case 1: newTagType = "DogTag_R_TypeOne"; break;
            case 2: newTagType = "DogTag_R_TypeTwo"; break;
            case 3: newTagType = "DogTag_R_TypeThree"; break;
            case 4: newTagType = "DogTag_R_TypeFour"; break;
            case 5: newTagType = "DogTag_R_TypeFive"; break;
        }

        if (newTagType != "")
        {
            GetGame().ObjectDelete(oldTag);
            DogTag_Base newTag = DogTag_Base.Cast(GetInventory().CreateAttachment(newTagType));
            if (newTag)
            {
                // Configura a nova tag usando os dados armazenados.
                newTag.SetOwnerID(ownerID);
                newTag.SetNickname(ownerName);
#ifdef EXPANSIONMODAI
                newTag.SetFactionName(factionName);
#endif
                
                MMS_PlayerData pData = MMS_PlayerDatabaseManager.GetInstance().GetOrCreatePlayerData(ownerID);
                pData.m_SurvivalLevel = newLevel;

                MMLogger.Log("[Survival] " + GetIdentity().GetName() + " foi promovido para DogTag Nível " + newLevel);

                // --- MENSAGEM DE NOTIFICAÇÃO CUSTOMIZÁVEL ---
                string notificationTitle = "Sincronização Melhorada";
                string notificationText = "Experiencia armazenada. Nova melhoria integrada. Tipo" + newLevel + ".";
                NotificationSystem.SendNotificationToPlayerExtended(this, 10, notificationTitle, notificationText, "set:dayz_gui image:icon_info");
            }
        }
    }

    // Bloqueia a remoção da DogTag
    override bool CanReleaseAttachment(EntityAI attachment)
    {
        // Verifica se o anexo é qualquer tipo de DogTag
        if (attachment.IsInherited(DogTag_Base))
        {
            // Se o jogador estiver vivo, impede a remoção
            if (IsAlive())
            {
                return false;
            }
        }
        return super.CanReleaseAttachment(attachment);
    }

    // Este método é chamado no servidor quando o jogador é morto.
    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);

        // --- INÍCIO DA NOVA LÓGICA ---
        // Reseta os dados de sobrevivência ao morrer
        if (GetGame().IsServer() && GetIdentity())
        {
            string playerID = GetIdentity().GetPlainId();
            MMS_PlayerData pData = MMS_PlayerDatabaseManager.GetInstance().FindPlayerData(playerID);
            if (pData)
            {
                pData.m_SurvivalTime = 0;
                pData.m_SurvivalLevel = 0;
                MMLogger.Log("[Survival] Dados de sobrevivência resetados para " + GetIdentity().GetName() + " após a morte.");
            }
        }
        // --- FIM DA NOVA LÓGICA ---

        // Encontra a DogTag (qualquer tipo) no slot do jogador.
        DogTag_Base myDogTag = GetDogTag();
        if (myDogTag)
        {
            // --- MUDANÇA PARA HORA REAL ---
            // Usamos GetYearMonthDay e GetHourMinuteSecond para pegar a hora do sistema (real).
            // Esta será a hora do relógio do servidor.
            int year, month, day, hour, minute, second;
            GetYearMonthDay(year, month, day);
            GetHourMinuteSecond(hour, minute, second);

            // Formata a string de data e hora.
            string deathTime = string.Format("Morreu em: %1/%2 às %3:%4 (UTC)", 
                                                day.ToStringLen(2), month.ToStringLen(2),
                                                hour.ToStringLen(2), minute.ToStringLen(2));
            // Chama a função na DogTag para registrar o momento da morte.
            // A DogTag cuidará da sincronização e persistência.
            myDogTag.SetDeathTimestamp(deathTime);
            MMLogger.Log("[DogTag] Jogador " + GetIdentity().GetName() + " morreu. Timestamp registrado na DogTag.");
        }
    }


    override void SetActions(out TInputActionMap InputActionMap)
	{	
		super.SetActions(InputActionMap);
		AddAction(ActionAddMember, InputActionMap);
        AddAction(ActionRemoveMember, InputActionMap);
	}
}