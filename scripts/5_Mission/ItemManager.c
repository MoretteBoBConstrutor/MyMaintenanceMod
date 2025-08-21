/**
 * @file ItemManager.c
 * @brief Modifica a lógica da UI do inventário para solicitar dados da DogTag sob demanda.
 */
modded class ItemManager
{
    // Construtor: chamado quando a UI do inventário é criada.
    void ItemManager(Widget root)
    {
        // Registra a nossa função 'OnDogTagInfoUpdate' para ser chamada quando o evento global da DogTag for disparado.
        DogTag_Base.s_OnDogTagInfoUpdate.Insert(OnDogTagInfoUpdate);
    }

    // Destrutor: chamado quando a UI do inventário é destruída.
    void ~ItemManager()
    {
        // É uma boa prática remover o registro para evitar erros.
        if (DogTag_Base.s_OnDogTagInfoUpdate)
        {
            DogTag_Base.s_OnDogTagInfoUpdate.Remove(OnDogTagInfoUpdate);
        }
    }

    /**
     * @brief Prepara o tooltip quando o mouse passa sobre um item no inventário.
     * Esta função é o nosso "gatilho" para a requisição de dados.
     */
    override void PrepareTooltip(EntityAI item, int x = 0, int y = 0)
    {
        // Chama a função original primeiro para manter o comportamento padrão.
        super.PrepareTooltip(item, x, y);

        // Tenta converter o item para a nossa classe DogTag.
        DogTag_Base dogTag = DogTag_Base.Cast(item);

        // Se a conversão for bem-sucedida e não estivermos arrastando um item.
        if (dogTag && !IsDragging())
        {
            // Pede os dados completos da tag ao servidor.
            dogTag.RequestNickname();
            dogTag.RequestDeathTimestamp(); // <-- Pede a nova informação da data da morte.
#ifdef EXPANSIONMODAI
            dogTag.RequestFactionName();    // <-- Pede a informação da facção.
#endif
        }
    }

    /**
     * @brief Função de callback que é chamada pelo evento da DogTag.
     * @param item A instância da DogTag que recebeu os dados.
     */
    private void OnDogTagInfoUpdate(EntityAI item)
    {
        // Se o item que recebeu a atualização é o mesmo que está sob o mouse,
        // forçamos a atualização do tooltip para exibir o novo nome.
        if (m_HoveredItem == item)
        {
            InspectMenuNew.UpdateItemInfo(m_TooltipWidget, item);
        }
    }
}