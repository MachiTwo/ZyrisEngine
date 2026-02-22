# Zyris Engine

[![Zyris Version](https://img.shields.io/badge/Zyris-1.3--AbilitySystem-blue.svg)](https://github.com/godotengine/godot)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE.txt)

> [!TIP]
> **Leia isto em outros idiomas / Read this in other languages:**
> [**English**](README.en.md) | [**Português**](README.md)

Zyris é uma engine open source orientada a sistemas e arquitetura de produção.

Projetada para oferecer controle estrutural, previsibilidade e escalabilidade,
Zyris integra o mesmo ecossistema de engines profissionais como Godot, Unreal Engine e Unity.

---

## Origem Tecnológica

Zyris é um fork da Godot Engine

Isso significa que:

- Projetos feitos na Godot podem ser abertos no Zyris
- Seu conhecimento em GDScript continua válido
- O fluxo de trabalho permanece familiar

Zyris mantém compatibilidade com a base Godot,
mas possui governança própria de commits e roadmap.

As atualizações upstream não são aplicadas automaticamente.
Cada mudança é analisada, filtrada e integrada de forma controlada.

---

## Filosofia

Zyris é uma engine **intencionalmente opinada**. Acreditamos que liberdade absoluta sem direção gera código caótico, retrabalho estrutural e sistemas inconsistentes ao longo do tempo.

Por isso, o Zyris estabelece **convenções claras**, arquiteturas recomendadas e sistemas integrados como padrão. Essas convenções não removem liberdade — elas reduzem a necessidade de reinventar soluções fundamentais.

Você ainda pode desenvolver da forma que preferir, mas, por padrão, o Zyris oferece:

- **Estruturas previsíveis**
- **Sistemas persistentes integrados**
- **Gerenciamento determinístico de estados**
- **Base arquitetural consistente** para IA e gameplay

Nosso objetivo não é limitar criatividade, mas **eliminar complexidade desnecessária**. Zyris evolui como uma engine orientada a produção, focada em coerência sistêmica, longevidade de projeto e maturidade técnica.

### Princípios de Evolução

Cada sistema introduzido no Zyris deve integrar-se nativamente ao núcleo, aderir a princípios determinísticos e priorizar estabilidade de produção.

A engine evolui de forma **curada**. Atualizações externas não são adotadas automaticamente; cada mudança é avaliada sob a ótica de coerência estrutural e maturidade técnica. Nosso foco é construir uma base sólida para projetos escaláveis e preparados para longevidade e evolução contínua.

Todo o crédito para a engine base vai para o [repositório official da Godot Engine](https://github.com/godotengine/godot) e sua incrível comunidade de contribuidores.

## Comunicação Oficial

O desenvolvimento e anúncios oficiais do Zyris são comunicados através de:

🎥 [Machi](https://www.youtube.com/@machiatodev)
🎥 [Alen](https://www.youtube.com/@yatsuragames)

Esses canais são responsáveis por:

- Atualizações da engine
- Explicações técnicas
- Demonstrações de sistemas
- Tutoriais oficiais

## Roadmap

O Zyris está implementando um conjunto abrangente de sistemas. Abaixo está nosso roadmap de desenvolvimento:

### Implementado

- [x] **Virtual Input Devices** - Camada de abstração de entrada multi-plataforma

    Um sistema completo de entrada virtual integrado diretamente ao core da engine, fornecendo controles de toque fluidos para dispositivos móveis e híbridos.

    **Nós de Interface:**

- `VirtualButton` - Botão de toque configurável com feedback visual e mapeamento de ações
- `VirtualJoystick` - Controle de analógico com zonas mortas e sensibilidade customizáveis
- `VirtualDPad` - Pad directional com suporte a 4 e 8 direções
- `VirtualTouchPad` - Área de gestos multi-toque para controle de câmera e viewport

    **Recursos Principais:**

- **Rastreamento de Dispositivo de Entrada** - API `LastInputType` detecta e rastreia automaticamente o método de entrada ativo (Toque, Teclado/Mouse, Gamepad)
- **Adaptação de UI Dinâmica** - Controles virtuais aparecem/ocultam automaticamente com base no dispositivo detectado
- **Modos de Visibilidade** - `ALWAYS`, `TOUCHSCREEN_ONLY` para comportamento flexível de UI
- **Mapeamento de Ações** - Integração direta com o sistema InputMap da Godot
- **Sistema de Entrada Polimórfico** - A classe base `InputEvent` agora usa métodos virtuais para correspondência de ações e comparação de eventos, permitindo que eventos virtuais customizados se integrem perfeitamente ao core da engine como cidadãos de primeira classe.
- **Integração com o Editor** - Suporte total no inspetor com configuração visual
- **Otimizado para Performance** - Overhead mínimo com manipulação eficiente de eventos

- [x] **Save Server** - Orquestrador de Persistência e Serialização

     Um sistema de persistência robusto e de alta performance integrado ao core da engine, permitindo salvar e carregar estados complexos de forma assíncrona e segura.

     **Recursos Principais:**

- **Protocolo Declarativo (@persistent)** - Automação total via anotações no GDScript e flags no C++. Suporta tipagem forte e organização hierárquica por `@persistent_group`.
- **Threaded Architecture** - Operações de I/O, compressão ZSTD e criptografia AES-256 rodam em threads dedicadas, garantindo autosaves fluidos sem travamentos.
- **Arquitetura Híbrida (Manifesto/Satélites)** - Suporte a persistência parcial através de tags, permitindo salvar subconjuntos de dados (ex: inventário, mundo) em arquivos satélites modulares.
- **Dynamic Spawning & Cleanup** - Recriação automática de nós instanciados em tempo de execução e remoção de órfãos (inimigos mortos) para garantir paridade total entre a cena e o snapshot.
- **Sistema Incremental (Amend Save)** - Inspirado no Git, rastreia objetos modificados e aplica "patches" cirúrgicos no snapshot, reduzindo a escrita em disco em até 95%.
- **Global ID Registry (persistence_id)** - Abstração de hierarquia que desvincula os dados de NodePaths, impedindo que saves quebrem se nós forem renomeados ou movidos.
- **Evolução de Dados (Migrações)** - Sistema de versionamento que permite registrar transformações de dados (`register_migration`) para manter compatibilidade com saves antigos.
- **Segurança e Integridade** - Sistema de backups rotativos, validação por checksum SHA-256 e mecanismo de *Flush* no encerramento para evitar corrupção de dados.

### Em Desenvolvimento

- [ ] **Ability System** - Framework de Gameplay Orientado a Dados

    Uma implementação nativa de alto desempenho projetada para escalar de mecânicas simples a sistemas de combate RPG complexos.

    **Sistemas Centrais:**
  - `AbilitySystem` - Singleton global que gerencia o registry de Gameplay Tags e as configurações do projeto.
  - `AbilitySystemComponent` - O processador central anexado a entidades (Player, Inimigos). Orquestra habilidades, efeitos, atributos e cues.
  - `AbilitySystemAbilityContainer` - Recurso de arquétipo que define a configuração base de uma entidade (Habilidades, Atributos Iniciais, Efeitos Inatos, Cues).
  - `AbilitySystemAttributeSet` - Container que gerencia coleções de AbilitySystemAttribute Resources com validação de limites e valores runtime.
  - `AbilitySystemAttribute` - Define estatísticas vitais (HP, Mana, Stamina, Level) com suporte a `BaseValue` e `CurrentValue` (com buffs/debuffs).
  - `AbilitySystemAbility` - Lógica modular de habilidades (Pular, Atirar, Magia) com **Costs & Cooldowns** e ciclo de vida `can_activate → activate → end`.
  - `AbilitySystemEffect` - Regras de alteração de atributos (Dano, Cura, Buffs/Debuffs) com políticas de duração (Instantâneo, Duração, Infinito).
  - `AbilitySystemTask` - Ações assíncronas para habilidades complexas (ex: aguardar input, mover para posição, spawnar projéteis).
  - `AbilitySystemTag` - Hierarchical named tag (e.g., `state.buff.speed`) using optimized `StringName` storage with O(1) performance via `HashSet<StringName>`.
  - `AbilitySystemCue` - Sistema de ativação e sincronização de eventos (animações, sons) disparado por efeitos ou habilidades. [CueParticles em planejamento]

    **Sistemas de Runtime (Specs):**

    Os *Specs* são instâncias de runtime que envolvem um Resource de definição com o contexto de execução necessário. Permitem que habilidades, efeitos e cues acessem dados dinâmicos (quem atacou, qual a magnitude calculada, duração restante) sem poluir o Resource compartilhado.
  - `AbilitySystemAbilitySpec` - Instância de runtime de uma `AbilitySystemAbility` concedida ao ASC. Armazena nível e estado ativo.
  - `AbilitySystemEffectSpec` - Instância de runtime de um `AbilitySystemEffect` aplicado. Armazena duração restante, magnitudes dinâmicas e referência ao ASC fonte/alvo.
  - `AbilitySystemCueSpec` - Contexto de execução de uma `AbilitySystemCue`. Carrega ASC fonte/alvo, o `AbilitySystemEffectSpec` que disparou a cue e a magnitude calculada.

    **Sistemas Auxiliares:**
  - `AbilitySystemMagnitudeCalculation` - Classe base para cálculos customizados de magnitude (MMC). Permite escalar dano por atributos (Força, Inteligência) ou curvas.
  - `AbilitySystemTagContainer` - Container otimizado de tags para consultas e modificações em runtime (add, remove, has, get_all).
  - `AbilitySystemTargetData` - Pipeline de validação e filtragem de alvos. Suporta Line Trace, Sphere Overlap, Box e shapes customizados.

    **Recursos Principais:**
- **Performance Bruta:** Cálculos de modificadores e processamento de tags em C++.
- **Data-Driven:** Habilidades e efeitos são `Resource`, permitindo que designers criem mecânicas sem tocar em código.
- **Desacoplamento:** A lógica de "como um dano é calculado" é separada de "quem causa o dano".
- **Integração Nativa:** Cache automático de nós irmãos (AnimationPlayer, AudioStreamPlayer, etc.) para performance otimizada.
- **Sinais Reativos:** `attribute_changed`, `tag_changed` para UI e feedback em tempo real.
- **Gameplay Tasks:** Sistema assíncrono para operações de longa duração dentro de habilidades.
- **Magnitude Calculation (MMC):** Cálculos complexos de magnitude para efeitos com curvas de dano e escala por atributos.
- **Ferramentas do Editor:** Inspector customizado para seletores especializados de Tags e Atributos.
- **Sincronização e Persistência:** Integrado nativamente com o `SaveServer` para persistência automática.

### Em Planejamento

- [ ] **Behavior Tree** - IA Modular e Reativa

    Uma implementação robusta de IA baseada em nós, focada em criar comportamentos complexos através de lógica visual simples e reutilizável.

    **Arquitetura:**
  - `BTAgent`: O orquestrador anexado ao NPC, gerenciando a memória (`Blackboard`) e o tick da árvore.
  - **Composites** - Nós de fluxo (`Sequence`, `Selector`) que definem a tomada de decisão.
  - **Decorators** - Lógica condicional e modificadores (`Cooldown`, `Loop`, `Blackboard Check`) que envolvem outros nós.
  - **Leafs** - As unidades de trabalho reais (`MoveTo`, `Wait`, `ActivateAbility`).

    **Integração com Editor (EditorPlugin):**
    Behavior Trees utilizam um **Editor Visual Dedicado** que opera em modo exclusivo.
  - **Visual Board** - Canvas infinito para organizar hierarquias de IA complexas com clareza visual, suportando drag-and-drop de nós.
  - **Live Debugging** - Rastreamento visual do fluxo de execução em tempo real (os nós acendem enquanto ativos).
  - **Sub-Trees** - Capacidade de criar nós que encapsulam árvores inteiras, permitindo reutilização massiva (ex: uma árvore "Combat" usada dentro de várias IAs diferentes).

    **Diferenciais:**
  - **Nativo & GAS** - Projetado para disparar `GameplayAbilities` diretamente via nós de ação.
  - **Performance** - Travessia de árvore em C++ puro, eliminando gargalos de script em IAs densas.

    **Casos de Uso:**
- **NPCs de RPG** - Comportamentos complexos de combate, diálogo e exploração.
- **Inimigos de FPS** - Táticas de squad, cobertura e patrulhamento.
- **IA de Estratégia** - Tomada de decisão em nível macro e microgerenciamento.

- [ ] **Inventory System** - Gestão de Items e Transações
  - **InventoryServer** - Singleton autoritativo que valida todas as movimentações de items, prevenindo estados inconsistentes ou trapaças.
  - **Equipment Bridge (GAS)** - Ao equipar um item, o sistema injeta/remove automaticamente *Gameplay Abilities* e *Effects* no componente do portador, sem scripts manuais.
  - **Smart UI Nodes** - Conjunto de nós `Control` (`InventoryGrid`, `EquipmentSlot`) que gerenciam drag-and-drop e sincronização automática com o servidor.
  - **Loot Tables** - Geração procedural integrada ao sistema de sorte e tags do GAS.

    **Arquitetura:**
- **Server-Side Authority** - Todas as transações validadas no servidor.
- **Client Prediction** - Interface responsiva com validação posterior.
- **Data-Driven** - Items definidos como Resources com propriedades e comportamentos.

    **Recursos Técnicos:**
- **Stack Management** - Gerenciamento automático de stacks e quantidades.
- **Slot Validation** - Validação de regras de equipamento (classe, nível, etc.).
- **Transaction Rollback** - Reversão automática de transações inválidas.
- **Network Sync** - Sincronização eficiente de mudanças de inventário.

    **Casos de Uso:**
- **RPGs** - Inventários complexos com equipamento e consumíveis.
- **Survival** - Crafting e gestão de recursos limitados.
- **MMOs** - Economia virtual e prevenção de trapaças.

- [ ] **Camera System (vCam)** - Arbitragem Cinematográfica
  - **Virtual Camera Resources** - Perfis de câmera definidos como Resources, contendo FOV, Follow Target, LookAt e restrições.
  - **Priority Arbiter** - O `vCamServer` avalia em tempo real qual vCam possui a maior prioridade para assumir o controle da viewport principal com blends configuráveis.
  - **Procedural Shake** - Sistema de trauma baseado em ruído de Perlin, onde os perfis de tremor são Resources editáveis e acumulativos.
  - **Integração com Viewport** - Utiliza **Gizmos 3D Customizados** para desenhar e ajustar visualmente zonas de transição, caminhos de tracking e volumes de influência diretamente na cena.

    **Arquitetura:**
- **vCamServer** - Singleton central que gerencia todas as câmeras virtuais do sistema Zyris.
- **Priority System** - Sistema de prioridades dinâmicas com blends suaves.
- **Resource-Based** - Configurações salvas como Resources para reutilização.
- **Non-Intrusive** - Interage com o sistema de câmera existente da Godot sem modificá-lo.

    **Recursos Técnicos:**
- **Dynamic Blending** - Transições suaves entre câmeras com curvas customizáveis.
- **Dead Zones** - Áreas mortas para evitar movimentação excessiva.
- **Aim Tracking** - Seguimento automático de alvos com previsão.
- **Cinematic Sequences** - Suporte a sequências cinematográficas pré-definidas.

    **Casos de Uso:**
- **Jogos de Ação** - Câmeras dinâmicas que respondem ao gameplay.
- **Cinematics** - Sequências de câmera complexas e dramáticas.
- **RPGs** - Câmeras over-the-shoulder com comportamento contextual.

- [ ] **Multiplayer Update** - Replicação de Próxima Geração e Predição

     Uma atualização sistêmica que injeta lógica de predição e replicação performática em todos os módulos centrais da engine.

     **Recursos Principais:**

- **Replicação por Camadas (Network LOD)** - Gerenciamento inteligente de banda baseado em prioridade e proximidade.
- **Predição e Reconciliação** - Framework nativo para mitigar latência com suporte a rollback autoritativo.
- **IA Autoritativa (Behavior Tree)** - Processamento da árvore de comportamento no servidor com sincronização de estados.
- **Multiplayer GAS** - Integração do Ability System com rede autoritativa e predição de atributos.
- **Debugger de Rede** - Monitoramento visual de largura de banda e latência por objeto diretamente no Editor.

     **API e Configuração:**
- **Configurações de Rede** - Server Tick Rate, Client Prediction, Relevance Distance, Bandwidth Limit via Project Settings.
- **Prioridades de Replicação** - CRITICAL, HIGH, LOW, NONE para controle granular de banda.
- **Predição de Movimento** - Input buffers, state buffers e reconciliação suave.
- **Validação Autoritativa** - Verificação de ações no servidor com broadcast de resultados.

     **Casos de Uso:**
- **FPS Multiplayer** - Movement prediction, hit registration, weapon sync.
- **RPG Multiplayer** - Ability system, inventory sync, world state persistente.
- **Jogos de Estratégia** - AI authority, unit commands, resource management autoritativo.

     **Performance e Otimização:**
- **Latency Compensation** - Compensação automática de latência.
- **Packet Loss Recovery** - Recuperação de pacotes perdidos.
- **Bandwidth Optimization** - Uso eficiente de banda com delta compression e bit-packing nativo.
- **Debug Tools** - Network Visualizer, Latency Graph, State Inspector.

  - [ ] **Dialogue System** - Motor Narrativo Interativo

    Um sistema de diálogo abrangente inspirado no Dialogic 2, projetado para criar conversas complexas com ramificações, visual novels e experiências de narrativa interativa.

    **Sistemas Centrais:**
  - `DialogueServer` - Singleton global gerenciando estado de diálogo, histórico e conversas ativas
  - `DialogueComponent` - Nó de cena anexado a personagens/NPCs para interações de diálogo
  - `DialogueResource` - Árvore de diálogo completa com caminhos ramificados, condições e definições de personagens
  - `DialogueLine` - Entradas de diálogo individuais com falante, texto, condições e ações
  - `DialogueChoice` - Opções selecionáveis pelo jogador com requisitos e consequências
  - `DialogueCondition` - Sistema de lógica condicional usando tags, atributos e estado do jogo
  - `DialogueAction` - Ações acionáveis (dar itens, iniciar quests, mudar estados)
  - `DialogueVariable` - Sistema de variáveis globais e locais para conteúdo de diálogo dinâmico

    **Recursos Avançados:**
  - **Editor Visual** - Editor de árvore de diálogo baseado em nós com preview em tempo real
  - **Sistema de Personagens** - Retratos de personagens, expressões e integração de voz
  - **Suporte à Localização** - Diálogo multi-idioma com resolução automática de texto
  - **Integração de Save** - Integração nativa com SaveServer para estado persistente de diálogo
  - **Sistema de Sinais** - Eventos de diálogo reativos para atualizações de UI e lógica do jogo
  - **Efeitos de Digitação** - Aparência de texto configurável com efeitos sonoros
  - **Integração com Timeline** - Suporte para cutscenes e animações durante diálogos

    **Casos de Uso:**
- **RPGs** - Conversas complexas de NPCs com histórias ramificadas
- **Visual Novels** - Experiências narrativas completas com desenvolvimento de personagens
- **Jogos de Aventura** - Quebra-cabeças narrativos e storytelling interativo
- **Jogos Educacionais** - Sistemas de tutorial e experiências de aprendizagem interativa

- [ ] **Quest System** - Gestão de Missões e Objetivos

    Um sistema robusto de gerenciamento de quests construído em torno de QuestTags, fornecendo acompanhamento abrangente de missões, gestão de objetivos e distribuição de recompensas.

    **Sistemas Centrais:**
  - `QuestServer` - Singleton autoritativo gerenciando todas as quests ativas e progresso
  - `QuestComponent` - Nó de cena para fornecedores de quests e entidades relacionadas a quests
  - `QuestResource` - Definição completa de quest com objetivos, requisitos e recompensas
  - `QuestTag` - Sistema de tags hierárquico para categorização e filtragem de quests
  - `QuestObjective` - Tarefas individuais com critérios de conclusão e acompanhamento de progresso
  - `QuestReward` - Recompensas configuráveis (itens, experiência, habilidades, moeda)
  - `QuestPrerequisite` - Sistema de requisitos usando tags, nível, itens ou quests completadas
  - `QuestTracker` - Componente de UI para exibir quests ativas e objetivos

    **Recursos Avançados:**
  - **Geração Dinâmica de Quests** - Criação procedural de quests usando templates e variáveis
  - **Cadeias de Quests** - Quests multi-partes com caminhos de progressão ramificados
  - **Quests com Limite de Tempo** - Quests com prazos e objetivos cronometrados
  - **Quests Compartilhadas** - Sincronização multiplayer de quests e compartilhamento de progresso
  - **Diário de Quests** - Histórico completo de quests com acompanhamento detalhado de progresso
  - **Sistema de Notificações** - Atualizações de quests, conclusões e alertas de novas quests
  - **Ponte de Integração** - Conectividade nativa com Ability System, Inventory e Dialogue

    **Recursos Técnicos:**
- **Sistema QuestTag** - Tags hierárquicas (ex: `quest.main.story.chapter1`) para organização
- **Persistência de Progresso** - Integração automática de save/load com SaveServer
- **Sistema de Condições** - Verificação complexa de pré-requisitos usando estado do jogo e tags
- **Atualizações Orientadas a Eventos** - Progresso de quests reativo baseado em eventos do jogo
- **Otimizado para Performance** - Verificação eficiente de quests e acompanhamento de progresso

    **Casos de Uso:**
- **MMOs** - Sistemas de quests em larga escala com milhares de quests simultâneas
- **RPGs** - Cadeias de quests narrativas com desenvolvimento de personagens
- **Jogos de Ação** - Sistemas de conquistas e acompanhamento de desafios
- **Jogos de Estratégia** - Progressão de campanha e gerenciamento de missões

  - [ ] **World Management System** - Orquestração de Mundo e Estados

    Um sistema abrangente de orquestração de mundo projetado para lidar com mundos de jogo em grande escala através de gerenciamento de estados determinísticos e zonas de streaming inteligentes.

    **Sistemas Centrais:**
  - `WorldManager` - Singleton global gerenciando o ciclo de vida da aplicação através de estados determinísticos (`BOOT`, `MENU`, `LOADING`, `GAMEPLAY`, `TRANSITION`)
  - `WorldRoot` - Arquitetura Super Node permitindo múltiplos mundos ou "universos" isolados coexistirem, facilitando transições de mapa sem interrupção da lógica core
  - `StreamingZone` - Sistema de volumes espaciais gerenciando o carregamento assíncrono de chunks do mundo baseado na posição e prioridade do jogador
  - `SceneStreaming` - Streaming inteligente de cenas com carregamento/descarregamento em background para transições seamless do mundo
  - `WorldInstance` - Sistema de instanciação dinâmica para dungeons, casas e áreas separadas do mundo
  - `LevelOfDetail` - Gerenciamento automático de LOD para performance otimizada em escala
  - `WorldPortal` - Sistema de portais otimizado para transições seamless entre áreas do mundo

    **Recursos Avançados:**
  - **Máquina de Estado Determinística** - GSM garante ciclo de vida previsível da aplicação e transições de estado
  - **Suporte Multi-Universo** - WorldRoot permite instâncias de mundo paralelas com lógica isolada
  - **Background Streaming** - Carregamento/descarregamento assíncrono de seções do mundo sem quedas de FPS
  - **Gerenciamento de Memória** - Pooling inteligente e coleta de lixo para dados do mundo
  - **Otimização de Rede** - Streaming de mundo do lado do servidor com predição de cliente para multiplayer
  - **Integração de Persistência** - Integração nativa com SaveServer para persistência de estado do mundo
  - **Ferramentas de Editor** - Ferramentas visuais de edição de mundo com preview e debugging de streaming
  - **Profile de Performance** - Ferramentas integradas de profiling para otimização de performance de streaming

    **Recursos Técnicos:**
- **Spatial Hash Grid** - Consultas espaciais eficientes para objetos e entidades do mundo
- **Chunk-Based Streaming** - Mundo dividido em chunks gerenciáveis com carregamento inteligente
- **Distance-Based Culling** - Culling automático de objetos distantes para performance
- **Hot-Swapping** - Atualizações do mundo em runtime sem interromper o gameplay
- **Otimização de Footprint de Memória** - Uso mínimo de memória através de cache inteligente
- **Persistência de Estado** - Estado do mundo automaticamente preservado durante transições

    **Casos de Uso:**
- **RPGs de Mundo Aberto** - Mundos massivos e seamless com dungeons e cidades
- **MMOs** - Mundos persistentes grandes com milhares de jogadores
- **Jogos de Ação** - Níveis grandes com streaming para reduzir tempos de loading

- [ ] **Cloud Providers** - Serviços de Nuvem e Abstração de Plataforma

     O módulo unificado para integração com serviços externos (Steam, EOS, Mobile), consolidando o sistema de salvamento remoto, contas e sessões de descoberta.

     **Recursos Principais:**

- **Abstração de Provedores (CloudProvider)** - Interface polimórfica que unifica Steam, Epic Online Services e PlayFab sob uma única API.
- **Sincronização de Save (Cloud Save)** - Upload/Download assíncrono de snapshots totalmente integrado ao `SaveServer`.
- **Lobbies e Discovery Nativo** - Gerenciamento de salas e matchmaking integrado diretamente às APIs dos provedores.
- **Single Sign-On (SSO)** - Autenticação transparente e gerenciamento de perfis persistentes.
- **Gestão de SDKs (EditorSDKManager)** - Interface no Editor para configuração e validação de SDKs de terceiros (Steamworks, GDK, etc).

     **Arquitetura de Provedores:**
- **SteamProvider** - Integração completa com Steam Cloud, Auth e Networking.
- **EOSProvider** - Suporte a Epic Online Services (Save, Friends, Sessions).
- **CustomHTTPProvider** - Backend genérico via REST para infraestrutura própria.
- **GooglePlayProvider** - Integração com Google Play Games Services.

     **Recursos Técnicos:**
- **Resolução de Conflitos** - Baseado em timestamps e checksums SHA-256.
- **Sincronização Incremental** - Apenas mudanças são transferidas.
- **Relay Networking** - Transporte de pacotes sem necessidade de Port Forwarding.
- **Lobby Management** - Criação, descoberta e metadados customizáveis.

     **Casos de Uso:**
- **Jogos Multiplayer** - Matchmaking, cross-platform, persistência na nuvem.
- **Jogos Single-Player com Cloud** - Progresso universal, backup automático, conquistas.
- **Aplicações Corporativas** - Backend custom, autenticação enterprise, analytics.

- [ ] **Melhorias no Audio System**

    O Zyris expande e reformula seu sistema de áudio nativo, atuando diretamente sobre os componentes existentes (`AudioServer`, `AudioStream`, `AudioStreamPlayer`, `AudioBus`, `AudioEffect`), sem introduzir servidores paralelos ou nomenclaturas externas.

    **Escopo das Melhorias:**
  - **Reformulação dos AudioStreamPlayers e Listeners**
    - Players passam a suportar lógica sonora de alto nível.
    - Integração direta com eventos, estados e parâmetros de gameplay.
    - Comportamento mais expressivo sem abandonar o modelo da Godot.

  - **Expansão dos Audio Resources existentes**
    - `AudioStream` com metadados e parâmetros dinâmicos.
    - Suporte a variações, camadas e resolução contextual.

  - **Introdução de novos Audio Resources**
    - Novos tipos seguem rigorosamente a herança padrão da Godot e nomenclatura existente.
    - Integração direta com o AudioServer.

  - **Mixagem Avançada e DSP**
    - DSP por stream, bus ou global (Equalização, filtros, compressão e efeitos temporais).
    - Controle dinâmico via código ou eventos.

  - **Eventos Sonoros**
    - Sons deixam de ser disparados apenas por arquivos; execução baseada em intenção e contexto.
    - Resolução dinâmica do áudio em runtime.

  - **Integração com Gameplay**
    - Integração direta com GAS, Behavior Trees, LSS e Save Server.
    - Estados do jogo influenciam mixagem, execução e transições sonoras.

    Essas melhorias elevam o áudio do Zyris a um nível moderno, reativo e escalável, mantendo compatibilidade conceitual com a engine base.

- [ ] **AOT Export System** - Arquitetura baseada em SDK

    O AOT Export System é um dos pilares centrais da visão do Zyris, projetado para entregar execução nativa de alto desempenho sem alterar o fluxo de desenvolvimento da Godot.

    O AOT não substitui a VM durante o desenvolvimento. Ele atua exclusivamente no momento da exportação, removendo a VM apenas do jogo final buildado e gerando binários nativos direcionados ao hardware de destino.

    **Benefícios Chave:**
  - **Performance Nativa** - Ao remover a Virtual Machine (VM) do executável final, o jogo roda diretamente no hardware, eliminando o overhead de interpretação de scripts.
  - **Segurança (Anti-Hack)** - A compilação para código de máquina nativo torna a engenharia reversa exponencialmente mais difícil se comparada à decompilação de scripts ou bytecode, protegendo a propriedade intelectual.
  - **Export-Only** - O fluxo de desenvolvimento permanece ágil com Hot-Reloading, enquanto o AOT atua apenas na pipeline de build final.

    Para garantir sustentabilidade, escalabilidade e facilidade de contribuição, o AOT é concebido como um SDK externo, integrado ao pipeline de exportação da engine.

    **Diretrizes do Modelo:**
  - Execução apenas na exportação (não participa do editor ou debug)
  - Editor permanece leve e desacoplado do custo do compilador
  - Contribuição facilitada via repositório dedicado
  - Arquitetura alinhada ao modelo de SDKs profissionais (Android SDK, NDK)

    O SDK utiliza Python como camada de orquestração, responsável por coordenar pipelines complexos de compilação, transformação de IR e integração com toolchains nativas.

## Licença

A Zyris Engine é licenciada sob a **Licença MIT**, mantendo compatibilidade com o licenciamento da Godot Engine.

Copyright (c) 2025-present Zyris contributors
Copyright (c) 2014-present Godot Engine contributors
Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur

Veja LICENSE.txt e COPYRIGHT.txt para detalhes completos.

## Agradecimentos

O Zyris é construído sobre o trabalho incrível da comunidade da **Godot Engine**. Somos profundamente gratos a:

- **Juan Linietsky** e **Ariel Manzur** - Fundadores da Godot Engine
- Todos os **contribuidores da Godot Engine** - Por criarem uma base fantástica
- A **comunidade open-source** - Por tornar projetos como este possíveis

Para a lista completa de contribuidores da Godot, veja [AUTHORS.md](AUTHORS.md) e [DONORS.md](DONORS.md).

## Links

- **Godot Engine Official**: <https://godotengine.org>
- **GitHub da Godot**: <https://github.com/godotengine/godot>
- **Repositório do Zyris**: <https://github.com/Kaffyn/ZyrisEngine>
- **Rastreador de Issues**: <https://github.com/Kaffyn/ZyrisEngine/issues>

---

**Zyris Engine** - Expandindo os limites do desenvolvimento de jogos de código aberto
