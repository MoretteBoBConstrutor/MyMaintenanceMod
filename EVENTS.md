# Askal - DayZ Mod

## 📋 Descrição

Mod para DayZ que adiciona sistema de Manutenção de Base(Proteção), Raid por Jammer e KDU (Dog Tag), e Rastreador de eventos. 

## 🎯 Rastreamento de Eventos Dinâmicos

Para conseguir rastrear os **Eventos Dinâmicos do DayZ** (não necessário para Helicrash), você irá precisar adicionar no arquivo **cfgeventgroups.xml** dentro da pasta do seu mapa a seguinte linha em cada evento que deseja rastrear:

```xml
<child type="EventBeacon" x="0" z="0" a="0"/>
```

### 📝 Exemplo de Implementação

Aqui está um exemplo de como implementar o EventBeacon em um grupo de evento:

```xml
<group name="Traffic_Bor">
    <child type="Land_wreck_truck01_aban1_orange_DE" deloot="1" lootmax="2" lootmin="2" x="0" z="0" a="351.878"/>
    <child type="StaticObj_Wreck_Decal_Small1_DE" spawnsecondary="false" x="-6.162" z="6.138" a="295.027"/> 
    <child type="StaticObj_Wreck_Decal_Small1_DE" spawnsecondary="false" x="0.445" z="-0.610" a="0"/>
    <child type="Land_Wreck_sed01_aban2_police_DE" deloot="1" lootmax="4" lootmin="2" x="-6.071" z="6.459" a="292.448"/>
    <child type="EventBeacon" x="0" z="0" a="0"/>
</group>
```

### ⚠️ Importante

- O `EventBeacon` deve ser adicionado como o **último child** do grupo de evento
- As coordenadas `x="0" z="0" a="0"` são padrão para o beacon
- Este beacon é necessário apenas para eventos que você deseja rastrear ativamente
- **Helicrash events** não necessitam desta configuração

### 📂 Localização do Arquivo

O arquivo `cfgeventgroups.xml` geralmente está localizado em:
```
SEU_MAPA\cfgeventgroups.xml
```

### 🔧 Como Aplicar

1. Abra o arquivo `cfgeventgroups.xml` do seu mapa
2. Localize o grupo de evento que deseja rastrear
3. Adicione a linha `<child type="EventBeacon" x="0" z="0" a="0"/>` antes de fechar o grupo `</group>`
4. Salve o arquivo
5. Reinicie o servidor [Se for server de Teste recomendo WIPE]

---
