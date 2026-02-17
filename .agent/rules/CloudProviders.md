---
trigger: always_on
---

# Cloud Providers (Serviços Externos)

O módulo **Cloud Providers** é o ponto de integração unificado da Zyris Engine com backends de serviços na nuvem (Steam, Epic Online Services, Google Play Games, etc.). Ele substitui o antigo sistema de cloud-save isolado por uma arquitetura de provedores polimórfica.

## 1. Arquitetura de Provedores (`CloudProvider`)

Uma interface única que abstrai as APIs de diferentes plataformas através de pontes especializadas.

- **CloudProvider (Base):** Classe abstrata que define o contrato para autenticação, salvamento e sessões.
- **Implementações:**
  - `SteamProvider`: Integração com Steam Cloud, Auth e Steam Networking.
  - `EOSProvider`: Suporte a Epic Online Services (Save, Friends, Sessions).
  - `CustomHTTPProvider`: Backend genérico para infraestrutura própria via REST.

## 2. Subsistemas Unificados

### 2.1 Cloud Save

- **Sincronização de Progresso:** Integrado ao `SaveServer`, realiza o upload/download automático de snapshots.
- **Resolução de Conflitos:** Sistema baseado em timestamps e checksums SHA-256 para garantir a integridade dos dados.

### 2.2 Account & Auth

- **Single Sign-On (SSO):** Autenticação transparente usando as credenciais do provedor de plataforma.
- **Profile Persistence:** Sincronização de metadados do jogador (conquistas, estatísticas).

### 2.3 Sessões e Lobbies (Transporte Relay)

- **Lobby Management:** Criação e descoberta de salas através das APIs do provedor.
- **Relay Networking:** Fornece o transporte de pacotes para o `MultiplayerAPI` sem a necessidade de Port Forwarding (P2P via Relay).

## 3. Gestão de SDKs (`EditorSDKManager`)

Interface dedicada no Editor para configurar caminhos de SDKs externos e validar dependências de compilação.

---

*Este documento define a especificidade do módulo de provedores de serviço na Zyris Engine.*
