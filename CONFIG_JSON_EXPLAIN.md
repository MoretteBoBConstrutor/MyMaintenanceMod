# 📋 Documentação dos Arquivos de Configuração JSON

Este documento explica todos os arquivos JSON gerados automaticamente pelo MyMaintenanceMod e suas configurações.

## 📂 Localização dos Arquivos

Todos os arquivos são gerados automaticamente na pasta do perfil do servidor:
```
$profile:MaintenanceMod/
├── config.json                    # Configurações principais do mod
├── UnitDatabase.json               # Banco de dados das unidades de manutenção
├── PlayerDatabase.json             # Dados de progressão dos jogadores
├── HeliCrashDatabase.json         # Banco de dados de crashes de helicóptero
├── MilitaryEventDatabase.json     # Banco de dados de eventos militares
├── EventBeaconConfig.json         # Configurações do sistema de eventos
└── activity.log                   # Log de atividades (não é JSON)
```

---

## ⚙️ config.json - Configurações Principais

**Arquivo:** `$profile:MaintenanceMod/config.json`  
**Gerenciado por:** `MaintenanceConfigManager.c`

### 🔧 Estrutura do Arquivo

```json
{
    "maintenanceRadius": 60.0,
    "resourcesPerCycle": 10,
    "checkInterval": 3600.0,
    "damagePerCycle": 10000.0,
    "vehicleDamagePerCycle": 250.0,
    "affectedStructureTypes": ["Fence", "WatchTower"],
    "consumedItemType": "Nail",
    "enableKitCrafting": true,
    "enableKitDecrafting": true,
    "protectVehicles": true,
    "requireMaintenanceUnitForKits": true,
    "restrictedPlacementItems": ["FenceKit", "WatchtowerKit", "ShelterKit"],
    "enableOwnershipSystem": true,
    "minDistanceBetweenUnits": 150,
    "minDistanceBetweenBuildings": 25,
    "limitDeployablesByLevel": true,
    "maxContainersPerLevel": [2, 4, 6],
    "maxBaseBuildingPerLevel": [1, 2, 3],
    "levelDefinitions": [
        {
            "level": 2,
            "requiredAttachments": ["M_UpgradeTo_Lvl2"]
        },
        {
            "level": 3,
            "requiredAttachments": ["M_UpgradeTo_Lvl2", "M_UpgradeTo_Lvl3"]
        }
    ],
    "ManagedFurnitureMap": {
        "ExampleFurnitureKit": "ExampleContainer"
    }
}
```

### 📖 Parâmetros de Configuração

| Parâmetro | Tipo | Padrão | Descrição |
|-----------|------|---------|-----------|
| `maintenanceRadius` | Float | 60.0 | Raio em metros da área de manutenção |
| `resourcesPerCycle` | Integer | 10 | Quantidade de recursos consumidos por ciclo |
| `checkInterval` | Float | 3600.0 | Intervalo em segundos entre ciclos (1 hora) |
| `damagePerCycle` | Float | 10000.0 | Dano aplicado a estruturas por ciclo sem recursos |
| `vehicleDamagePerCycle` | Float | 250.0 | Dano aplicado a veículos por ciclo sem recursos |
| `affectedStructureTypes` | Array | ["Fence", "WatchTower"] | Tipos de estruturas afetadas pela manutenção |
| `consumedItemType` | String | "Nail" | Tipo de item consumido para manutenção |
| `enableKitCrafting` | Boolean | true | Permite criação de kits de construção |
| `enableKitDecrafting` | Boolean | true | Permite desmontagem de kits |
| `protectVehicles` | Boolean | true | Protege veículos na área de manutenção |
| `requireMaintenanceUnitForKits` | Boolean | true | Requer unidade próxima para usar kits |
| `restrictedPlacementItems` | Array | [...] | Lista de itens com colocação restrita |
| `enableOwnershipSystem` | Boolean | true | Ativa sistema de propriedade de unidades |
| `minDistanceBetweenUnits` | Integer | 150 | Distância mínima entre unidades (metros) |
| `minDistanceBetweenBuildings` | Integer | 25 | Distância mínima entre construções (metros) |
| `limitDeployablesByLevel` | Boolean | true | Limita construções por nível da unidade |
| `maxContainersPerLevel` | Array | [2, 4, 6] | Limite de containers por nível |
| `maxBaseBuildingPerLevel` | Array | [1, 2, 3] | Limite de construções por nível |
| `levelDefinitions` | Array | [...] | Definições dos níveis e requisitos |
| `ManagedFurnitureMap` | Object | {...} | Mapeamento de móveis gerenciados |

---

## 🏗️ UnitDatabase.json - Banco de Dados das Unidades

**Arquivo:** `$profile:MaintenanceMod/UnitDatabase.json`  
**Gerenciado por:** `MaintenanceUnitManager.c`

### 🔧 Estrutura do Arquivo

```json
{
    "m_AllUnitsData": [
        {
            "m_UnitID": 1,
            "m_OwnerSteamID": "76561198012345678",
            "m_Position": [1234.5, 45.2, 6789.1],
            "m_Orientation": [0.0, 180.0, 0.0],
            "m_Members": ["76561198087654321", "76561198098765432"]
        }
    ],
    "m_NextUnitID": 2
}
```

### 📖 Estrutura dos Dados

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `m_UnitID` | Integer | ID único da unidade de manutenção |
| `m_OwnerSteamID` | String | Steam64ID do proprietário |
| `m_Position` | Array | Coordenadas X, Y, Z da unidade |
| `m_Orientation` | Array | Orientação da unidade (pitch, yaw, roll) |
| `m_Members` | Array | Lista de Steam64IDs dos membros |
| `m_NextUnitID` | Integer | Próximo ID disponível para novas unidades |

---

## 👤 PlayerDatabase.json - Dados dos Jogadores

**Arquivo:** `$profile:MaintenanceMod/PlayerDatabase.json`  
**Gerenciado por:** `PlayerDatabaseManager.c`

### 🔧 Estrutura do Arquivo

```json
{
    "m_AllPlayerData": [
        {
            "m_PlayerID": "76561198012345678",
            "m_SurvivalTime": 7200.0,
            "m_SurvivalLevel": 2
        }
    ]
}
```

### 📖 Estrutura dos Dados

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `m_PlayerID` | String | Steam64ID do jogador |
| `m_SurvivalTime` | Float | Tempo de sobrevivência acumulado (segundos) |
| `m_SurvivalLevel` | Integer | Nível atual da Dog Tag (0-5) |

### 🏷️ Níveis de Dog Tag

| Nível | Tempo Mínimo | Classe | HP |
|-------|--------------|---------|-----|
| 0 | Fresh Spawn | DogTag_Unregistered | 100 |
| 1 | 1 hora | DogTag_R_TypeOne | 200 |
| 2 | 2 horas | DogTag_R_TypeTwo | 300 |
| 3 | 3 horas | DogTag_R_TypeThree | 400 |
| 4 | 4 horas | DogTag_R_TypeFour | 500 |
| 5 | 5+ horas | DogTag_R_TypeFive | 600 |

---

## 🚁 HeliCrashDatabase.json - Dados de Crashes

**Arquivo:** `$profile:MaintenanceMod/HeliCrashDatabase.json`  
**Gerenciado por:** `HeliCrashDatabaseManager.c`

### 🔧 Estrutura do Arquivo

```json
{
    "m_AllCrashData": [
        {
            "m_CrashID": "crash_1234.5_6789.1_1640995200000",
            "m_Position": [1234.5, 45.2, 6789.1],
            "m_Timestamp": 1640995200000,
            "m_IsActive": true
        }
    ],
    "m_NextCrashID": 2
}
```

### 📖 Estrutura dos Dados

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `m_CrashID` | String | ID único do crash (formato: crash_X_Z_timestamp) |
| `m_Position` | Array | Coordenadas X, Y, Z do crash |
| `m_Timestamp` | Integer | Timestamp de criação (milissegundos) |
| `m_IsActive` | Boolean | Se o crash ainda está ativo no mapa |
| `m_NextCrashID` | Integer | Próximo ID sequencial |

### ⏰ Lógica de Expiração

- Crashes expiram automaticamente após **4 horas**
- Crashes expirados são removidos do database automaticamente
- Sistema verifica expiração a cada ciclo de limpeza

---

## 🪖 MilitaryEventDatabase.json - Eventos Militares

**Arquivo:** `$profile:MaintenanceMod/MilitaryEventDatabase.json`  
**Gerenciado por:** `MilitaryEventDatabaseManager.c`

### 🔧 Estrutura do Arquivo

```json
{
    "m_AllEventData": [
        {
            "m_EventID": "military_1234.5_6789.1_1640995200000",
            "m_EventType": "Contaminated Area",
            "m_Position": [1234.5, 45.2, 6789.1],
            "m_Timestamp": 1640995200000,
            "m_IsActive": true
        }
    ],
    "m_NextEventID": 2
}
```

### 📖 Estrutura dos Dados

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `m_EventID` | String | ID único do evento |
| `m_EventType` | String | Tipo do evento militar |
| `m_Position` | Array | Coordenadas X, Y, Z do evento |
| `m_Timestamp` | Integer | Timestamp de criação (milissegundos) |
| `m_IsActive` | Boolean | Se o evento ainda está ativo |
| `m_NextEventID` | Integer | Próximo ID sequencial |

### 🎯 Tipos de Eventos Militares

- **Contaminated Area** - Área contaminada
- **Artillery Strike** - Bombardeio de artilharia
- **Air Raid** - Ataque aéreo
- **Military Checkpoint** - Checkpoint militar

---

## 📡 EventBeaconConfig.json - Configurações de Eventos

**Arquivo:** `$profile:MaintenanceMod/EventBeaconConfig.json`  
**Gerenciado por:** `EventBeaconConfig.c`

### 🔧 Estrutura do Arquivo

```json
{
    "m_EnableEventMonitoring": true,
    "m_EnableDebugLogs": true
}
```

### 📖 Parâmetros de Configuração

| Parâmetro | Tipo | Padrão | Descrição |
|-----------|------|---------|-----------|
| `m_EnableEventMonitoring` | Boolean | true | Ativa/desativa monitoramento de eventos |
| `m_EnableDebugLogs` | Boolean | true | Ativa/desativa logs detalhados |

---

## 🔧 Gerenciamento e Manutenção

### 🔄 Criação Automática

- Todos os arquivos são criados automaticamente na primeira execução
- Valores padrão são aplicados quando não existe configuração
- Diretório `$profile:MaintenanceMod/` é criado automaticamente

### 💾 Salvamento e Carregamento

- **config.json**: Carregado na inicialização, salvo quando modificado
- **UnitDatabase.json**: Salvo a cada alteração de unidade
- **PlayerDatabase.json**: Salvo periodicamente e na saída do servidor
- **HeliCrashDatabase.json**: Salvo a cada novo crash detectado
- **MilitaryEventDatabase.json**: Salvo a cada novo evento militar
- **EventBeaconConfig.json**: Salvo quando configurações são alteradas

### 🧹 Limpeza Automática

- **Crashes expirados**: Removidos automaticamente após 4 horas
- **Eventos antigos**: Limpeza baseada no tipo de evento
- **Jogadores inativos**: Dados mantidos indefinidamente
- **Unidades órfãs**: Removidas se não existem no mundo

### ⚠️ Backup Recomendado

- Faça backup regular da pasta `$profile:MaintenanceMod/`
- Especialmente importante antes de atualizações do mod
- Configure backups automáticos no servidor

### 🔍 Troubleshooting

#### Arquivo corrompido:
1. Pare o servidor
2. Delete o arquivo corrompido
3. Reinicie o servidor (arquivo será recriado com padrões)

#### Configurações não aplicadas:
1. Verifique sintaxe JSON válida
2. Reinicie o servidor após alterações
3. Consulte logs para erros de carregamento

#### Performance Issues:
- Reduza `maintenanceRadius` se necessário
- Aumente `checkInterval` para menor frequência
- Monitore tamanho dos databases

---

## 📝 Notas Importantes

- **Não edite** databases enquanto servidor estiver rodando
- **Sempre valide** JSON antes de aplicar mudanças
- **Monitore logs** para detectar problemas
- **Teste configurações** em ambiente de desenvolvimento
- **Mantenha backups** regulares dos arquivos
