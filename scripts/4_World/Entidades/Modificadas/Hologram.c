modded class Hologram
{
    // Avalia colisão do holograma com o ambiente e outros objetos
    override void EvaluateCollision(ItemBase action_item = null)
    {
        ItemBase item_in_hands = ItemBase.Cast(m_Player.GetHumanInventory().GetEntityInHands());
        ItemBase mu_colision = ColisionMaintenceUnit();

        // Permite posicionar o MaintenanceUnitKit sem colisão
        if (item_in_hands && item_in_hands.IsInherited(MaintenanceUnitKit))
        {
            SetIsColliding(false);
            return;
        }
        // Permite posicionar se colidir com outro MaintenanceUnit
        if (item_in_hands && item_in_hands.IsInherited(Inventory_Base) && mu_colision)
        {
            SetIsColliding(false);
            return;
        }
        super.EvaluateCollision(action_item);
    }

    // Verifica colisão com pisos e retorna o objeto colidido, se houver
    ItemBase ColisionMaintenceUnit()
    {
        vector center, relative_ofset, absolute_ofset = "0 0.01 0", orientation, edge_length, min_max[2];
        ref array<Object> excluded_objects = new array<Object>;
        ref array<Object> collided_objects = new array<Object>;

        m_Projection.GetCollisionBox(min_max);
        relative_ofset[1] = (min_max[1][1] - min_max[0][1]) / 2.4;
        center = m_Projection.GetPosition() + relative_ofset + absolute_ofset;
        edge_length = GetCollisionBoxSize(min_max);
        excluded_objects.Insert(m_Projection);

        if (GetGame().IsBoxColliding(center, orientation, edge_length, excluded_objects, collided_objects))
        {
            // Retorna o primeiro objeto MaintenanceUnit colidido
            for (int i = 0; i < collided_objects.Count(); i++)
            {
                Object obj = collided_objects[i];
                if (obj.IsInherited(MaintenanceUnit))
                    return MaintenanceUnit.Cast(obj);
            }
        }
        return null;
    }

    // Define o tipo de holograma baseado no kit em mãos
    override string ProjectionBasedOnParent()
    {
        MaintenanceUnitKit Main_Kit = MaintenanceUnitKit.Cast(m_Player.GetHumanInventory().GetEntityInHands());
        if (Main_Kit)
            return Main_Kit.RequiredHologram();
        return super.ProjectionBasedOnParent();
    }

    // Permite posicionar o próprio kit como entidade
    override EntityAI PlaceEntity(EntityAI entity_for_placing)
    {
        if (entity_for_placing.IsInherited(MaintenanceUnitKit))
        {
            return entity_for_placing;
        }
        return super.PlaceEntity(entity_for_placing);
    }
}