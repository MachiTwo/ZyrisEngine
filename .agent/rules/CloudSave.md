---
trigger: always_on
---

# Sistema de Cloud Save

## 1. Contexto e Filosofia

O sistema de **Cloud Save** na Zyris Engine é um pipeline nativo para sincronização de progresso entre dispositivos, integrado diretamente ao fluxo do `SaveServer`.

- **Prediction & Fallback:** O sistema prioriza a versão local para carregamento rápido, mas verifica a nuvem em background para resolver conflitos.
- **Abstração de Plataforma:** Uma interface única (`CloudSaveProvider`) abstrai as particularidades de Steam, Google Play, Xbox e PlayStation.
- **Determinismo:** Focado em garantir que o snapshot serializado seja idêntico em todas as plataformas através de checksums SHA-256.

## 2. Configurações Globais (Editor)

Diferente de outras configurações, o Cloud Save requer caminhos para SDKs externos. Estas configurações são gerenciadas via `EditorSDKManager` e armazenadas em `EditorSettings`.

### 2.1 SDK Paths (`export/sdks/`)

- **`steamworks_sdk_path`**: Caminho raiz do Steamworks SDK. Requer presença de `public/steam/steam_api.h`.
- **`gpg_sdk_path`**: Caminho do Google Play Games C++ SDK. Requer `gpg/gpg.h`.
- **`xbox_gdk_path`**: Caminho do Microsoft Game Development Kit.
- **`playstation_sdk_path`**: Caminho do SDK da Sony (acesso restrito via NDA).

## 3. Provedores de Cloud Save

O sistema utiliza provedores especializados que implementam a interface `CloudSaveProvider`.

### 3.1 Custom HTTP (`CloudSaveCustom`)

Provedor genérico baseado em REST API. Útil para implementações independentes de plataforma.

- **Endpoints**: `upload`, `download`, `query`, `delete`.
- **Protocolo**: HTTPS com autenticação via Bearer Token.
- **Headers**:
  - `X-Checksum`: Hash do snapshot.
  - `X-Timestamp`: Data de criação para resolução de conflitos.

### 3.2 Steam Cloud (`CloudSaveSteam`)

- **Backend**: utiliza `SteamRemoteStorage()`.
- **Requisito**: Engine compilada com `STEAM_ENABLED`.

### 3.3 Google Play Games (`CloudSaveGPlay`)

- **Backend**: utiliza o Snapshot API do GPGS.
- **Requisito**: Engine compilada com `GOOGLE_PLAY_GAMES_ENABLED` e `ANDROID_ENABLED`.

### 3.4 Xbox / PlayStation

- **Xbox**: Integração com *Connected Storage*.
- **PlayStation**: Sistema de *Mount Point* para save data sincronizado automaticamente pelo sistema operacional.

## 4. Fluxo de Execução

### 4.1 Ciclo de Salvamento

1. O `SaveServer` finaliza a escrita no disco local.
2. Dispara `_cloud_upload()` de forma assíncrona.
3. O provedor calcula o Delta ou faz o upload completo do `.snapshot`.
4. Feedback de sucesso ou falha é emitido via sinal `cloud_sync_finished`.

### 4.2 Resolução de Conflitos

Quando uma divergência é detectada entre o snapshot local e o remoto:

1. **Timestamp Check**: A versão com o timestamp mais recente é sugerida como prioritária.
2. **User Choice**: Em casos ambíguos, a engine dispara um sinal que pode ser capturado pela UI do jogo para que o usuário escolha qual versão manter.
3. **Merge Diferencial**: Futuras implementações suportarão merge de dicionários JSON se não houver conflitos de chaves.

## 5. Implementação Técnica

### 5.1 Estrutura de Pastas

- `servers/save/`: Orquestração global.
- `modules/cloud_save/`: Implementações dos provedores.
- `editor/gui/editor_sdk_manager.cpp`: Interface de configuração de caminhos.

### 5.2 Flags de Compilação

As plataformas de Cloud Save são incluídas condicionalmente:

- `STEAM_ENABLED`
- `GOOGLE_PLAY_GAMES_ENABLED`
- `XBOX_GDK_ENABLED`
- `PS5_SDK_ENABLED`

---

*Este documento define a especificação oficial do subsistema de Cloud Save na Zyris Engine.*
