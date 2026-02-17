# Cloud Providers - Integração com Serviços na Nuvem

## Visão Geral

Sistema unificado de integração com serviços na nuvem (Steam, Epic Online Services, Google Play Games, etc.) oferecendo autenticação, salvamento na nuvem e multiplayer através de uma API única e polimórfica.

## Recursos Principais

### 1. Arquitetura de Provedores Polimórfica

Interface única que abstrai diferentes plataformas:

- **SteamProvider**: Integração completa com Steam Cloud, Auth e Networking
- **EOSProvider**: Suporte a Epic Online Services (Save, Friends, Sessions)
- **CustomHTTPProvider**: Backend genérico via REST para infraestrutura própria
- **GooglePlayProvider**: Integração com Google Play Games Services

### 2. Cloud Save Integrado

Sincronização automática de progresso com a nuvem:

```gdscript
# Upload automático de saves
CloudProvider.upload_save("slot_1", SaveServer.get_snapshot_data())

# Download e merge com saves locais
CloudProvider.download_save("slot_1", func(data):
    SaveServer.merge_snapshot_data(data)
)
```

**Recursos:**

- **Resolução de Conflitos**: Baseado em timestamps e checksums SHA-256
- **Sincronização Incremental**: Apenas mudanças são transferidas
- **Integração Transparente**: Funciona com o SaveServer existente

### 3. Single Sign-On (SSO)

Autenticação unificada através das credenciais da plataforma:

```gdscript
# Login automático
CloudProvider.authenticate(func(success, user_info):
    if success:
        print("Logado como: ", user_info.username)
        show_main_menu()
    else:
        show_login_screen()
)
```

**Recursos:**

- **Autenticação Transparente**: Usa credenciais do provedor
- **Profile Persistence**: Sincronização de metadados do jogador
- **Multi-plataforma**: Mesmo código funciona em todas as plataformas

### 4. Lobby Management

Sistema completo de gerenciamento de salas:

```gdscript
# Criar lobby
var lobby = CloudProvider.create_lobby({
    "max_players": 8,
    "game_mode": "deathmatch",
    "map": "arena_01"
})

# Descobrir lobbies disponíveis
CloudProvider.find_lobbies(func(lobbies):
    for lobby in lobbies:
        add_to_lobby_list(lobby)
)
```

**Recursos:**

- **Criação e Descoberta**: API unificada para todas as plataformas
- **Metadados Customizáveis**: Propriedades específicas do jogo
- **Filtros Busca**: Encontre lobbies por critérios específicos

### 5. Relay Networking

Transporte de pacotes sem necessidade de Port Forwarding:

```gdscript
# Conectar via relay
CloudProvider.connect_relay(lobby.relay_address, func(success):
    if success:
        multiplayer.multiplayer_peer = CloudProvider.get_relay_peer()
        start_multiplayer_game()
)
```

**Recursos:**

- **P2P via Relay**: Conexões diretas através de servidores relay
- **NAT Traversal**: Sem necessidade de configuração de roteador
- **Performance**: Latência otimizada para gameplay multiplayer

## Configuração

### Editor SDK Manager

Interface dedicada para gerenciar SDKs externos:

- **Configuração de Caminhos**: Defina localização dos SDKs
- **Validação de Dependências**: Verifica se todos os requisitos estão instalados
- **Build Integration**: Configura automaticamente para compilação

### Configurações do Projeto

Acessível via `Project Settings > Network > Cloud Providers`:

- **Provider Ativo**: Selecione o provedor principal
- **Fallback Options**: Provedores secundários em caso de falha
- **Sync Settings**: Frequência de sincronização automática

## API Completa

### Autenticação

```gdscript
# Verificar status de autenticação
if CloudProvider.is_authenticated():
    var user_info = CloudProvider.get_user_info()
    show_profile(user_info)

# Logout
CloudProvider.sign_out()
```

### Gerenciamento de Saves

```gdscript
# Lista de saves na nuvem
CloudProvider.list_saves(func(save_list):
    for save in save_list:
        add_save_entry(save.name, save.timestamp, save.size)
)

# Download específico
CloudProvider.download_save("slot_1", on_save_loaded)
```

### Multiplayer

```gdscript
# Entrar em lobby existente
CloudProvider.join_lobby(lobby_id, func(success):
    if success:
        start_lobby_scene()
)

# Sair do lobby atual
CloudProvider.leave_current_lobby()
```

## Casos de Uso

### Jogos Multiplayer

- **Matchmaking**: Encontre jogadores automaticamente
- **Cross-platform**: Jogadores de diferentes plataformas jogando juntos
- **Persistência**: Progresso salvo na nuvem entre dispositivos

### Jogos Single-Player com Cloud

- **Progresso Universal**: Continue jogando em qualquer dispositivo
- **Backup Automático**: Proteção contra perda de dados locais
- **Conquistas**: Sincronização com sistemas de conquistas da plataforma

### Aplicações Corporativas

- **Backend Custom**: Use infraestrutura própria com CustomHTTPProvider
- **Autenticação Enterprise**: Integração com sistemas corporativos
- **Analytics**: Envio de dados para serviços de análise

## Provedores Suportados

### Steam

- **Steam Cloud**: Salvamento automático
- **Steam Auth**: Login via Steam
- **Steam Networking**: P2P e relay

### Epic Online Services

- **EOS Save**: Sistema de saves da Epic
- **EOS Auth**: Login via Epic Games
- **EOS Sessions**: Matchmaking e lobbies

### Google Play Games

- **PGS Save**: Saves no Google Drive
- **PGS Auth**: Login via Google
- **PGS Multiplayer**: Integração com Nearby Connections

---

_Sistema enterprise-ready para integração com serviços na nuvem de qualquer escala._
