modded class BaseBuildingBase
{
    /**
     * @brief Lógica de reparo completo, inspirada na sua função RepairEntity.
     * Itera por todas as zonas de dano e as restaura para a vida máxima.
     */
    private void FullRepair()
    {
        // Array para armazenar os nomes das zonas de dano.
        array<string> damageZones = new array<string>();
        // Pega todas as zonas de dano do objeto.
        GetDamageZones(damageZones);

        // Repara cada zona de dano específica. É isso que restaura as partes destruídas.
        if (damageZones && damageZones.Count() > 0)
        {
            foreach (string zone_name : damageZones)
            {
                // Se a vida da zona não estiver no máximo, restaura.
                if (GetHealth01(zone_name, "Health") < 1.0)
                {
                    SetHealth(zone_name, "Health", GetMaxHealth(zone_name, "Health"));
                }
            }
        }

        // Por segurança, repara também a vida global do objeto.
        if (GetHealth01() < 1.0)
        {
            SetHealth(GetMaxHealth());
        }
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GetGame().IsServer())
        {
            // A lógica agora é simples: perguntamos ao Manager se a proteção está desativada para esta posição.
            // O Manager faz todo o trabalho pesado de encontrar a unidade, o jammer e verificar a propriedade.
            if (!MaintenanceUnitManager.GetInstance().IsProtectionDisabled(GetPosition()))
            {
                 // Se a proteção NÃO estiver desativada, verificamos se a base deveria estar protegida por ter recursos.
                MaintenanceUnit protectingUnit = MaintenanceUnitManager.GetInstance().GetUnitProtectingPosition(GetPosition());
                if (protectingUnit && protectingUnit.HasEnoughResources())
                {
                    MMLogger.Log("[EEHitBy] Proteção ATIVA para " + GetType() + ". Reparando.");
                    FullRepair();
                    return; // Interrompe a execução para não aplicar dano.
                }
            }
            else
            {
                 MMLogger.Log("[EEHitBy] Proteção DESATIVADA para " + GetType() + ". Dano será aplicado.");
            }
        }

        // Se não estiver protegido (sem recursos) ou se a proteção for desativada, o dano é processado normalmente.
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}