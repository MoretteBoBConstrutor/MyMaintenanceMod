class Jammer extends Container_Base
{
    protected Particle         m_ParticleEfx;
    // --- INÍCIO: LÓGICA DE SOM ---
    protected EffectSound m_JammerSound; // Guarda a referência do som
    // --- FIM: LÓGICA DE SOM ---    

    // --- INÍCIO: CONSTANTES DA LUZ ---
    private const string LIGHT_SELECTION_NAME = "jammer_light";
    private const string LIGHT_MATERIAL_OFF   = "MyMaintenanceMod\\data\\jammer\\materials\\jammer_light_red.rvmat";
    private const string LIGHT_MATERIAL_ON    = "MyMaintenanceMod\\data\\jammer\\materials\\jammer_light_green.rvmat";
    // --- FIM: CONSTANTES DA LUZ ---

    // --- INÍCIO: CONSTANTES DE DANO DA DOGTAG ---
    private const float DOGTAG_DAMAGE_PER_CYCLE = 25.0; // Causa 25 de dano a cada ciclo. Se a vida da tag é 200, ela será destruída em 8 minutos.
    private const int   DOGTAG_DAMAGE_INTERVAL_MS = 60 * 1000; // 1 minuto em milissegundos = 60 * 1000
    // --- FIM: CONSTANTES DA LUZ ---


    // Variáveis para controlar o estado do Jammer
    protected bool m_IsActive;
    protected string m_TargetOwnerID; // SteamID do dono da DogTag anexada
    protected int m_CooldownEndTime; // Timestamp de quando o cooldown para pegar o item termina

    void Jammer()
    {
        m_IsActive = false;
        m_TargetOwnerID = "";
        m_CooldownEndTime = 0; // Inicia sem cooldown
        // Registra a variável para sincronização (para que os clientes saibam se está ligado)
        RegisterNetSyncVariableBool("m_IsActive");
    }

     // Chamado quando o item é inicializado no cliente (ex: ao entrar na bolha de rede)
    override void EEInit()
    {
        super.EEInit();
        // Garante que o estado do som seja verificado assim que o item é carregado no cliente.
        // Isso lida com o caso de o jogador se aproximar de um Jammer que já está ligado.
        UpdateJammerEffects();
    }

    /**
     * @brief Função auxiliar para verificar se o Jammer está em cooldown.
     */
    private bool IsOnCooldown()
    {
        if (m_CooldownEndTime == 0) return false;
        // Com a persistência removida, voltamos a usar GetGame().GetTime() (milissegundos).
        return GetGame().GetTime() < m_CooldownEndTime;
    }

    override bool CanPutInCargo(EntityAI parent)
    {
        if (IsRuined() || FindAttachmentBySlotName("JammerDogTag") != null)
        {
            return false;
        }

        // --- NOVA VERIFICAÇÃO DE COOLDOWN ---
        if (IsOnCooldown())
        {
            PlayerBase player = PlayerBase.Cast(parent.GetHierarchyRootPlayer());
            if (player)
            {
                // Corrigido para calcular segundos a partir de milissegundos.
                int remainingSeconds = ((m_CooldownEndTime - GetGame().GetTime()) / 1000) + 1;
                NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Jammer", "Aguarde o resfriamento... " + remainingSeconds + "s restantes.", "set:dayz_gui image:icon_radio");
            }
            return false;
        }

        return super.CanPutInCargo(parent);
    }

    override bool CanPutIntoHands(EntityAI parent)
    {
        // Reutiliza a mesma lógica de CanPutInCargo para consistência.
        // O 'parent' aqui é o próprio jogador.
        return CanPutInCargo(parent);
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        // Permite apenas itens do tipo "DogTag_Registered"
        if (attachment.IsKindOf("DogTag_Registered"))
        {
            return super.CanReceiveAttachment(attachment, slotId);
        }
        return false;
    }

    override bool CanReleaseAttachment(EntityAI attachment)
    {
        // Se o Jammer estiver ativo (m_IsActive == true), bloqueia a remoção.
        if (IsActive())
        {
            return false;
        }

        // Se o Jammer estiver desligado, permite a remoção chamando a lógica padrão.
        return super.CanReleaseAttachment(attachment);
    }

    // Retorna se o Jammer está ativo
    bool IsActive()
    {
        return m_IsActive;
    }

    // Retorna o ID do alvo que o Jammer está tentando bloquear
    string GetTargetOwnerID()
    {
        return m_TargetOwnerID;
    }

    // Ativa o Jammer (Lógica principal no servidor)
    void TurnOn()
    {
        if (!GetGame().IsServer()) return;

        DogTag_Registered attachedTag = GetAttachedDogTag();
        if (attachedTag && attachedTag.GetOwnerID() != "")
        {
            m_IsActive = true;
            m_TargetOwnerID = attachedTag.GetOwnerID();
            SetSynchDirty(); // Força a sincronização da variável m_IsActive

            // Inicia o ciclo de dano na DogTag, que se repetirá a cada minuto.
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DamageAttachedDogTag, DOGTAG_DAMAGE_INTERVAL_MS, true);

            MMLogger.Log("Jammer ativado. Alvo: " + m_TargetOwnerID);
        }
    }

    void TurnOff()
    {
        if (!GetGame().IsServer()) return;

        // Interrompe o ciclo de dano na DogTag.
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.DamageAttachedDogTag);

        // --- INÍCIO: LÓGICA DE COOLDOWN ---
        // O cooldown agora não é persistente. Usamos GetGame().GetTime() (milissegundos).
        m_CooldownEndTime = GetGame().GetTime() + (60 * 1000); // Adiciona 1 minuto (60000 ms) de cooldown
        // --- FIM: LÓGICA DE COOLDOWN ---

        m_IsActive = false;
        m_TargetOwnerID = "";
        SetSynchDirty(); // Força a sincronização
        MMLogger.Log("Jammer desativado.");
    }

    /**
     * @brief Função chamada periodicamente para aplicar dano à DogTag anexada.
     */
    protected void DamageAttachedDogTag()
    {
        // Verificação de segurança: só deve rodar no servidor e se o Jammer estiver ativo.
        if (!GetGame().IsServer() || !IsActive())
        {
            // Se por algum motivo esta função for chamada com o Jammer inativo, remove o timer.
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.DamageAttachedDogTag);
            return;
        }

        DogTag_Registered tag = GetAttachedDogTag();
        if (tag)
        {
            tag.AddHealth("", "Health", -DOGTAG_DAMAGE_PER_CYCLE);
            MMLogger.Log("Jammer causou " + DOGTAG_DAMAGE_PER_CYCLE + " de dano à DogTag. Vida restante: " + tag.GetHealth("", "Health"));

            // Se a DogTag for destruída, o Jammer se desliga.
            if (tag.IsRuined())
            {
                MMLogger.Log("DogTag foi destruída pelo Jammer. Desligando imediatamente.");
                // Primeiro, desliga o Jammer para parar todos os timers e efeitos.
                TurnOff();
                // Então, deleta a DogTag destruída.
                GetGame().ObjectDelete(tag);
            }
        }
    }

    // Procura e retorna a DogTag anexada
    DogTag_Registered GetAttachedDogTag()
    {
        // O nome do slot "DogTagSlot" será definido no config.cpp
        EntityAI attachment = FindAttachmentBySlotName("JammerDogTag"); 
        if (attachment)
        {
            return DogTag_Registered.Cast(attachment);
        }
        return null;
    }

    /**
     * @brief Chamado no servidor sempre que um anexo é removido deste item.
     * Garante que o Jammer se desligue se a DogTag for removida por qualquer motivo.
     */
    override void OnWasDetached(EntityAI parent, int slot_id)
    {
        super.OnWasDetached(parent, slot_id);

        // Se o Jammer estava ativo e a DogTag foi removida, ele deve ser desligado.
        if (GetGame().IsServer() && IsActive())
        {
            MMLogger.Log("DogTag foi removida do Jammer, desligando...");
            TurnOff();
        }
    }

    // --- INÍCIO: CORREÇÃO DO SOM AO SER DESTRUÍDO ---

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);

        // No servidor, garante que o estado lógico do item seja 'desligado' e que essa mudança seja sincronizada com os clientes.
        if (GetGame().IsServer())
        {
            TurnOff();
        }
        StopJammerSound();
    }

    override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
        if (newLevel == GameConstants.STATE_RUINED)
        {
            m_ParticleEfx = ParticleManager.GetInstance().PlayOnObject(ParticleList.SMOKE_GENERIC_WRECK, this, Vector(0, 0.4, 0));
            EffectSound m_JammerRuinned;
            PlaySoundSet(m_JammerRuinned,"Jammer_Instant_SoundSet", 0, 0, false);

            if (m_ParticleEfx)
            {
                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(StopSmokeParticle, 15000, false); // 5000 ms = 5 segundos
            }
        }
        //SMOKING_HELI_WRECK SMOKE_GENERIC_WRECK SMOKING_CAR_ENGINE
        //GameConstants.STATE_PRISTINE = 1
        //GameConstants.STATE_WORN = 2
        //GameConstants.STATE_DAMAGED = 3
        //GameConstants.STATE_BADLY_DAMAGED = 4
        //GameConstants.STATE_RUINED = 5

        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(UpdateJammerEffects, 1, false);
    }

    override void AfterStoreLoad()
    {
        super.AfterStoreLoad();
        // Garante que os efeitos visuais e sonoros estejam corretos ao carregar o item.
        UpdateJammerEffects();
    }

    // Chamado quando o objeto é destruído
    override void EEDelete(EntityAI parent)
    {
        super.EEDelete(parent);

        if (m_ParticleEfx)
        {
            m_ParticleEfx.Stop();
            m_ParticleEfx = null;
        }

        // Garante que o timer de dano pare no servidor se o item for deletado.
        if (GetGame() && GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.DamageAttachedDogTag);
        }
        // Garante que o som pare no cliente se o item for destruído
        if (GetGame() && GetGame().IsClient())
        {
            StopJammerSound();
        }
    }

    // Chamado nos clientes quando as variáveis de rede são atualizadas
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();

        // Reage à mudança de estado do m_IsActive no lado do cliente, atualizando som e luz.
        UpdateJammerEffects();
    }

    // Atualiza o som e a luz com base no estado do Jammer
    protected void UpdateJammerEffects()
    {
       
        if (IsActive())
        {
            StartJammerSound();
            SetLightState(true); // Liga a luz (verde)
            // Mostra as animações
            SetAnimationPhase( "antena", 0 );
            SetAnimationPhase( "blade", 0 );
        }
        else
        {
            StopJammerSound();
            SetLightState(false); // Desliga a luz (vermelho)
            // Esconde as animações
            SetAnimationPhase( "antena", 1 );
            SetAnimationPhase( "blade", 1 );
        }
    }

    // Controla a cor da luz (apenas no cliente)
    protected void SetLightState(bool isOn)
    {
        int selectionIndex = GetHiddenSelectionIndex(LIGHT_SELECTION_NAME);
        if (selectionIndex == -1) return;

        if (isOn)
            SetObjectMaterial(selectionIndex, LIGHT_MATERIAL_ON);
        else
            SetObjectMaterial(selectionIndex, LIGHT_MATERIAL_OFF);
    }

    // Inicia o som do Jammer (apenas no cliente)
    protected void StartJammerSound()
    {
        // O som só deve tocar no cliente e se ainda não estiver tocando
        if (GetGame().IsClient() && !m_JammerSound)
        {
            // Agora usamos nosso SoundSet customizado definido no config.cpp
            PlaySoundSet(m_JammerSound, "Jammer_Loop_SoundSet", 0, 0, true);
            Print("[Jammer] Som iniciado no cliente para o item: " + this);
        }
    }

    // Para o som do Jammer (apenas no cliente)
    protected void StopJammerSound()
    {
        // O som só deve ser parado no cliente e se estiver tocando
        if (GetGame().IsClient() && m_JammerSound)
        {
            StopSoundSet(m_JammerSound);
            m_JammerSound = null; // Limpa a referência
            Print("[Jammer] Som parado no cliente para o item: " + this);
        }
    }
    // --- FIM: LÓGICA DE SOM ---

    void StopSmokeParticle()
    {
        if (m_ParticleEfx)
        {
            m_ParticleEfx.Stop();
            m_ParticleEfx = null;
        }
    }

    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionTurnOnJammer);
        AddAction(ActionTurnOffJammer);
    }
}


modded class ModItemRegisterCallbacks
{
    override void RegisterTwoHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
    {
        super.RegisterTwoHanded(pType, pBehavior);
        pType.AddItemInHandsProfileIK("Jammer",							"dz/anims/workspaces/player/player_main/weapons/player_main_2h_extinguisher.asi",	pBehavior,					"dz/anims/anm/player/ik/two_handed/BaseRadio.anm"); 
    }
};