class DogTag_Registered extends DogTag_Base
{
    /**
     * @brief Sobrescreve o nome do item exibido na interface do usuário (inventário, tooltip).
     */
    override bool NameOverride(out string output)
    {
        if (m_Nickname != "")
        {
            output = string.Format("%1 %2", ConfigGetString("displayName"), m_Nickname);
            return true;
        }
        // Se não tiver nickname, usa o displayName do config.cpp ("Dog Tag Registrada")
        return false;
    }

    /**
     * @brief Sobrescreve a descrição do item exibida na interface do usuário (inventário, tooltip).
     */
    override bool DescriptionOverride(out string output)
    {
        string ownerName = m_Nickname;
        if (GetGame().IsClient() && ownerName == "" && m_HasNickname)
        {
            ownerName = "Carregando...";
        }
 
        string deathTime = m_DeathTimestamp;
        if (GetGame().IsClient() && deathTime == "" && m_HasDeathTimestamp)
        {
            deathTime = "Disconnected: Carregando...";
        }

#ifdef EXPANSIONMODAI
        string factionName = m_FactionName;
        if (GetGame().IsClient() && factionName == "" && m_HasFactionName)
        {
            factionName = "Carregando...";
        }

        string finalDesc = "";
        bool hasContent = false;

        if (factionName != "")
        {
            finalDesc += "Facção: " + factionName;
            hasContent = true;
        }

        if (ownerName != "")
        {
            if (hasContent) finalDesc += "\n";
            finalDesc += "Sync ID: " + ownerName;
            hasContent = true;
        }

        if (deathTime != "")
        {
            if (hasContent) finalDesc += "\n";
            finalDesc += deathTime;
            hasContent = true;
        }

        if (hasContent)
        {
            output = finalDesc;
            return true;
        }
#else
        // Comportamento padrão sem o mod de facções
        string finalDesc = "";
        if (ownerName != "")
        {
            finalDesc += "Sync ID: " + ownerName;
        }
        if (deathTime != "")
        {
            if (finalDesc != "") finalDesc += "\n";
            finalDesc += deathTime;
        }
        if (finalDesc != "") { output = finalDesc; return true; }
#endif
        return false;
    }
}

class DogTag_R_TypeOne extends DogTag_Registered{};
class DogTag_R_TypeTwo extends DogTag_Registered{};
class DogTag_R_TypeThree extends DogTag_Registered{};
class DogTag_R_TypeFour extends DogTag_Registered{};
class DogTag_R_TypeFive extends DogTag_Registered{};
