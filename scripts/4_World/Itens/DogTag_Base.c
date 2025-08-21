class DogTag_Base extends Clothing
{
    
    // Mantém uma lista de todas as instâncias de DogTag no servidor para evitar buscas globais.
    static ref array<DogTag_Base> s_AllDogTags = new array<DogTag_Base>;
    // --- FIM OTIMIZAÇÃO DE LIMPEZA ---
    // Versionamento para os dados salvos. Incremente este número sempre que a estrutura de OnStoreSave/Load mudar.
    private static const int DOGTAG_DATA_VERSION = 1;
    static ref ScriptInvoker s_OnDogTagInfoUpdate = new ScriptInvoker();
    private static const int RPC_MDT_REQUEST_NICKNAME = 11001;
    private static const int RPC_MDT_RECEIVE_NICKNAME = 11002;
    private static const int RPC_MDT_REQUEST_DEATH_TIMESTAMP = 11003;
    private static const int RPC_MDT_RECEIVE_DEATH_TIMESTAMP = 11004;
#ifdef EXPANSIONMODAI
    private static const int RPC_MDT_REQUEST_FACTION_NAME = 11005;
    private static const int RPC_MDT_RECEIVE_FACTION_NAME = 11006;
#endif
    
    // --- VARIÁVEL PARA EXPIRAÇÃO ---
    // Timestamp de quando a tag foi solta no mundo (não anexada). 0 = não está no timer.
    int m_DropTimestamp;
    //static const int DOGTAG_EXPIRE_MS = 5 * 60 * 1000; // 5 minutos
    static const int DOGTAG_EXPIRE_MS = 2 * 24 * 60 * 60 * 1000; // 2 dias (descomente para produção)

    // Usamos duas partes int para sincronizar o SteamID64 de forma confiável pela rede.
    int m_OwnerID_Part1; 
    int m_OwnerID_Part2;
    
    // Cache local do ID reconstruído para acesso rápido. Não é sincronizado diretamente.
    private string m_CachedOwnerID; 

    // --- NOVAS VARIÁVEIS PARA O NOME DO JOGADOR ---
    // Armazena o nome do jogador. Fica apenas no servidor e é enviado sob demanda.
    string m_Nickname; 
    // Sinalizador sincronizado que informa aos clientes que esta tag TEM um nome para ser requisitado.
    bool m_HasNickname;
    
    // --- NOVAS VARIÁVEIS PARA DATA DA MORTE ---
    string m_DeathTimestamp; // Armazena a data da morte (apenas no servidor e clientes que pedem)
    bool m_HasDeathTimestamp; // Sinalizador sincronizado que informa que há uma data de morte
#ifdef EXPANSIONMODAI
    // --- VARIÁVEIS PARA INTEGRAÇÃO COM FACÇÕES ---
    string m_FactionName;       // Armazena o nome da facção. Apenas no servidor.
    bool   m_HasFactionName;    // Sinalizador sincronizado que informa aos clientes que esta tag TEM uma facção.
#endif
    
    void DogTag_Base()
    {
        // Inicializa as variáveis.
        m_OwnerID_Part1 = 0;
        m_OwnerID_Part2 = 0;
        m_CachedOwnerID = "";
        m_Nickname = "";
        m_HasNickname = false;
        m_DeathTimestamp = "";
        m_HasDeathTimestamp = false;
#ifdef EXPANSIONMODAI
        m_FactionName = "";
        m_HasFactionName = false;
#endif

        m_DropTimestamp = 0;
        
        // Registra as variáveis para serem sincronizadas do servidor para os clientes.
        RegisterNetSyncVariableInt("m_OwnerID_Part1");
        RegisterNetSyncVariableInt("m_OwnerID_Part2");
        RegisterNetSyncVariableBool("m_HasNickname"); // <-- Apenas o sinalizador é sincronizado
        RegisterNetSyncVariableBool("m_HasDeathTimestamp"); // <-- Sincroniza o sinalizador da data da morte
#ifdef EXPANSIONMODAI
        RegisterNetSyncVariableBool("m_HasFactionName");
#endif
        
        // Log de criação
        if (GetGame() && GetGame().IsServer()) {
            // Adiciona a si mesma à lista estática para o cleanup.
            s_AllDogTags.Insert(this);
        }
    }

    // --- INÍCIO PASSO 3: Lógica de Comunicação Cliente-Servidor (RPC) ---

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        switch (rpc_type)
        {
            // O cliente está PEDINDO o nome.
            case RPC_MDT_REQUEST_NICKNAME:
                // Esta parte só executa no SERVIDOR.
                if (GetGame().IsServer())
                {
                    // Prepara um RPC de resposta.
                    ScriptRPC rpc = new ScriptRPC();
                    // Escreve o nome (que só o servidor conhece) no pacote.
                    rpc.Write(m_Nickname);
                    // Envia o pacote de volta, mas APENAS para o jogador que pediu (sender).
                    rpc.Send(this, RPC_MDT_RECEIVE_NICKNAME, true, sender);
                }
                break;

            // O cliente está RECEBENDO a resposta do servidor.
            case RPC_MDT_RECEIVE_NICKNAME:
                // Esta parte só executa no CLIENTE.
                if (GetGame().IsClient())
                {
                    // Lê o nome do pacote recebido.
                    ctx.Read(m_Nickname);
                    // PASSO 4: Dispara o evento para notificar a UI que os dados foram atualizados.
                    s_OnDogTagInfoUpdate.Invoke(this);
                }
                break;

            // O cliente está PEDINDO a data da morte.
            case RPC_MDT_REQUEST_DEATH_TIMESTAMP:
                if (GetGame().IsServer())
                {
                    ScriptRPC rpc_ts = new ScriptRPC();
                    rpc_ts.Write(m_DeathTimestamp);
                    rpc_ts.Send(this, RPC_MDT_RECEIVE_DEATH_TIMESTAMP, true, sender);
                }
                break;

            // O cliente está RECEBENDO a data da morte.
            case RPC_MDT_RECEIVE_DEATH_TIMESTAMP:
                if (GetGame().IsClient())
                {
                    ctx.Read(m_DeathTimestamp);
                    s_OnDogTagInfoUpdate.Invoke(this); // Notifica a UI para redesenhar o tooltip
                }
                break;

#ifdef EXPANSIONMODAI
            // O cliente está PEDINDO o nome da facção.
            case RPC_MDT_REQUEST_FACTION_NAME:
                if (GetGame().IsServer())
                {
                    ScriptRPC rpc_faction = new ScriptRPC();
                    rpc_faction.Write(m_FactionName);
                    rpc_faction.Send(this, RPC_MDT_RECEIVE_FACTION_NAME, true, sender);
                }
                break;

            // O cliente está RECEBENDO o nome da facção.
            case RPC_MDT_RECEIVE_FACTION_NAME:
                if (GetGame().IsClient())
                {
                    ctx.Read(m_FactionName);
                    s_OnDogTagInfoUpdate.Invoke(this);
                }
                break;
#endif
        }
    }

    // Função chamada pelo ItemManager (Passo 2) no lado do cliente.
    void RequestNickname()
    {
        // A requisição só faz sentido no cliente.
        if (GetGame().IsClient())
        {
            // Otimização: Se já temos o nome, ou se o servidor já nos disse que não há nome, não pedimos de novo.
            if (m_Nickname != "" || !m_HasNickname)
                return;

            // Envia um RPC vazio para o servidor, apenas para sinalizar o pedido.
            GetGame().RPCSingleParam(this, RPC_MDT_REQUEST_NICKNAME, null, true);
        }
    }

    // Função para o cliente pedir a data da morte ao servidor.
    void RequestDeathTimestamp()
    {
        if (GetGame().IsClient())
        {
            // Otimização: Se já temos a data, ou se o servidor já nos disse que não há data, não pedimos de novo.
            if (m_DeathTimestamp != "" || !m_HasDeathTimestamp)
                return;

            GetGame().RPCSingleParam(this, RPC_MDT_REQUEST_DEATH_TIMESTAMP, null, true);
        }
    }

#ifdef EXPANSIONMODAI
    void RequestFactionName()
    {
        if (GetGame().IsClient())
        {
            if (m_FactionName != "" || !m_HasFactionName)
                return;

            GetGame().RPCSingleParam(this, RPC_MDT_REQUEST_FACTION_NAME, null, true);
        }
    }
#endif
    // Chamado em todos os clientes quando as variáveis sincronizadas são atualizadas pelo servidor.
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        RebuildCachedID();
    }


   // Função central para reconstruir o ID em cache a partir das partes.
    private void RebuildCachedID()
    {
        if (m_OwnerID_Part1 == 0 && m_OwnerID_Part2 == 0)
        {
            m_CachedOwnerID = "";
        }
        else
        {
            string part1_str = string.Format("%1", m_OwnerID_Part1);
            string part2_str = string.Format("%1", m_OwnerID_Part2);

            while (part2_str.Length() < 9)
            {
                part2_str = "0" + part2_str;
            }
            m_CachedOwnerID = part1_str + part2_str;
        }
    }

    // Define o proprietário. Chamado apenas no servidor.
    void SetOwnerID(string id)
    {
        if (id == "" || id.Length() != 17) 
        {
            m_OwnerID_Part1 = 0;
            m_OwnerID_Part2 = 0;
        }
        else
        {
            // Divide o ID em duas partes para armazenamento.
            m_OwnerID_Part1 = id.Substring(0, 8).ToInt();
            m_OwnerID_Part2 = id.Substring(8, 9).ToInt();
        }
        
        // Após definir as partes, reconstrói o cache no servidor imediatamente.
        RebuildCachedID();

        // Marca a entidade como 'suja' para forçar a sincronização.
        SetSynchDirty();
    }

    // Define o nome do jogador. Chamado apenas no servidor.
    void SetNickname(string name)
    {
        // Esta função só deve ser executada no servidor.
        if (!GetGame().IsServer())
            return;

        m_Nickname = name;
        // Se o nome não for vazio, ativamos o sinalizador.
        m_HasNickname = (name != "");

        // Marca a entidade como 'suja' para forçar a sincronização do m_HasNickname.
        SetSynchDirty();
    }

#ifdef EXPANSIONMODAI
    void SetFactionName(string name)
    {
        if (!GetGame().IsServer())
            return;

        m_FactionName = name;
        m_HasFactionName = (name != "");
        SetSynchDirty();
    }
#endif
    // Define a data da morte. Chamado a partir do PlayerBase no evento EEKilled.
    void SetDeathTimestamp(string timestamp)
    {
        // Esta função só deve ser executada no servidor.
        if (!GetGame().IsServer())
            return;

        m_DeathTimestamp = timestamp;
        m_HasDeathTimestamp = (timestamp != "");
        
        // Atualiza a descrição no servidor e marca para sincronização.
        SetSynchDirty();
    }

     // Retorna o SteamID64 do proprietário a partir do cache.
    string GetOwnerID()
    {
        return m_CachedOwnerID;
    }

    /**
     * @brief Retorna o nome do jogador armazenado na tag.
     * @return O nome do jogador, ou uma string vazia se não houver.
     */
    string GetNickname()
    {
        return m_Nickname;
    }

#ifdef EXPANSIONMODAI
    string GetFactionName()
    {
        return m_FactionName;

    }
#endif

    // Salva as variáveis no banco de dados do servidor.
    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);

        // 1. Salva a versão atual dos dados.
        ctx.Write(DOGTAG_DATA_VERSION);

        // 2. Salva os dados da versão 1.
        ctx.Write(m_OwnerID_Part1);
        ctx.Write(m_OwnerID_Part2);
        ctx.Write(m_Nickname);
        ctx.Write(m_DeathTimestamp); // Salva a data da morte
#ifdef EXPANSIONMODAI
        ctx.Write(m_FactionName);
#endif

        // Salva o timestamp de criação
        ctx.Write(m_DropTimestamp);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;
        
        int data_version = 0;
        if (!ctx.Read(data_version))
        {
            // Se não conseguirmos ler a versão, os dados estão corrompidos.
            return false;
        }

        // Carrega os dados com base na versão lida, com verificações de segurança.
        if (data_version >= 1)
        {
            // Se a leitura de uma variável falhar, atribuímos um valor padrão para
            // evitar corrupção de dados em memória e garantir que o resto do carregamento continue.
            if (!ctx.Read(m_OwnerID_Part1))
                m_OwnerID_Part1 = 0;
            
            if (!ctx.Read(m_OwnerID_Part2))
                m_OwnerID_Part2 = 0;

            if (!ctx.Read(m_Nickname))
                m_Nickname = "";

            if (!ctx.Read(m_DeathTimestamp))
                m_DeathTimestamp = "";

#ifdef EXPANSIONMODAI
            if (!ctx.Read(m_FactionName))
                m_FactionName = "";
#endif

            if (!ctx.Read(m_DropTimestamp))
                m_DropTimestamp = 0;
        }

        RebuildCachedID();
        return true;
    }
    // --- FIM DA CORREÇÃO DE PERSISTÊNCIA ---

    // Chamado após o OnStoreLoad, ideal para configurar estados derivados.
    override void AfterStoreLoad()
    {
        super.AfterStoreLoad();

        // Após carregar do banco de dados, garantimos que os sinalizadores estejam corretos.
        m_HasNickname = (m_Nickname != "");
        m_HasDeathTimestamp = (m_DeathTimestamp != "");
#ifdef EXPANSIONMODAI
        m_HasFactionName = (m_FactionName != "");
#endif

        // Força a sincronização das variáveis de rede após o carregamento.
        // Isso garante que o cliente receba o estado atualizado dos sinalizadores (m_HasNickname, etc).
        SetSynchDirty();
    }
    // --- FIM DA LÓGICA DE PERSISTÊNCIA ---

    // --- INÍCIO OTIMIZAÇÃO DE LIMPEZA ---
    override void EEDelete(EntityAI parent)
    {
        super.EEDelete(parent);

        if (GetGame() && GetGame().IsServer())
        {
            // Remove a si mesma da lista estática ao ser deletada.
            int index = s_AllDogTags.Find(this);
            if (index > -1)
                s_AllDogTags.Remove(index);
        }
    }
    // --- FIM OTIMIZAÇÃO DE LIMPEZA ---

    // --- LÓGICA DE EXPIRAÇÃO ---

    // Chamado no servidor quando o item é anexado a um parent.
    override void OnWasAttached(EntityAI parent, int slot_id)
    {
        super.OnWasAttached(parent, slot_id);

        // Se a tag for anexada a um jogador ou a um Jammer, ela está "segura".
        // Resetamos o timer de expiração para garantir que ela não seja deletada.
        if (parent.IsInherited(PlayerBase) || parent.IsInherited(Jammer)) // Jammer é a classe, não o tipo de item.
        {
            MMLogger.Log("[DogTag] Anexada a um protetor. Timer de expiração resetado.");
            m_DropTimestamp = 0;
        }
    }

    // Chamado no servidor quando o item é desanexado de um parent.
    override void OnWasDetached(EntityAI parent, int slot_id)
    {
        super.OnWasDetached(parent, slot_id);

        // Se a tag foi removida de um jogador (ex: morte) ou de um Jammer,
        // ela agora está "solta" no mundo. Iniciamos o timer de expiração.
        if (parent.IsInherited(PlayerBase) || parent.IsInherited(Jammer)) // Jammer é a classe, não o tipo de item.
        {
            MMLogger.Log("[DogTag] Desanexada de um protetor. Timer de expiração iniciado.");
            m_DropTimestamp = GetGame().GetTime();
        }
    }

/*     override bool CanDetachAttachment(EntityAI parent)
    {
        int slot_id = GetInventory().GetSlotId();
        string slot_name = InventorySlots.GetSlotName(slot_id);
        if (slot_name == "Kdu_link")
        {
            PlayerBase player = PlayerBase.Cast(parent);
            if (parent.GetHierarchyRoot())
            {
                player = PlayerBase.Cast(parent.GetHierarchyRoot());
            }
            if (player && player.IsAlive())
            {
                // Bloqueia a remoção do slot Armband enquanto o jogador está vivo
                return false;
            }
        }
        return super.CanDetachAttachment(parent);
    }
 */
    bool IsExpired()
    {
        // Se m_DropTimestamp é 0, significa que a tag está segura (anexada)
        // ou nunca foi desanexada, então ela não pode expirar.
        if (m_DropTimestamp == 0)
            return false;

        // Calcula se o tempo desde que foi solta no mundo excedeu o limite.
        int now = GetGame().GetTime();
        bool expired = (now - m_DropTimestamp) > DOGTAG_EXPIRE_MS;
        return expired;
    }

    // Considera "protegida" se estiver anexada a um Player ou a um Jammer
    bool IsProtectedAttachment()
    {
        EntityAI parent = GetHierarchyParent();
        if (!parent)
            return false;

        // Protege se estiver anexada a um Player
        if (PlayerBase.Cast(parent))
            return true;

        // Protege se estiver anexada a um Jammer
        if (parent.IsKindOf("Jammer"))
            return true;

        return false;
    }

    // --- INÍCIO PASSO 4: Lógica de Exibição na UI ---

    /**
     * @brief Sobrescreve o nome do item exibido na interface do usuário (inventário, tooltip).
     * @param output A string de saída que será exibida como nome.
     * @return true se o nome foi sobrescrito, false para usar o nome padrão do config.
     */
    override bool NameOverride(out string output)
    {
        // Se o nickname já foi recebido do servidor e não está vazio...
        if (m_Nickname != "")
        {
            // Formata o nome para "Dog Tag (NomeDoJogador)".
            output = string.Format("%1 (%2)", ConfigGetString("displayName"), m_Nickname);
            return true; // Informa ao jogo que o nome foi modificado.
        }

        return false; // Usa o nome padrão "Dog Tag".
    }
};
