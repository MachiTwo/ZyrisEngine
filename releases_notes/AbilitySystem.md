# Ability System

## 1. Contexto e Filosofia

O **Ability System** na Zyris Engine é um framework nativo de alta performance projetado para lidar com lógica de combate, habilidades e atributos de forma modular e baseada em dados.

Diferente de implementações em script puro, o Ability System do Zyris foca em:

- **Performance Bruta:** Cálculos de modificadores e processamento de tags em C++.
- **Data-Driven:** Habilidades e efeitos são `Resource`, permitindo que designers criem mecânicas sem tocar em código.
- **Desacoplamento:** A lógica de "como um dano é calculado" é separada de "quem causa o dano".

## 2. Arquitetura Core (C++)

O sistema está organizado em subpastas em `modules/ability_system/` para melhor manutenibilidade:

- `core/`: Singleton central, Tags e Specs.
- `resources/`: Definições de Atributos, Habilidades e Efeitos.
- `scene/`: Componentes de Cena.

### 2.0 AbilitySystem (Singleton)

O ponto central de verdade do sistema. Gerenciado como um Singleton da Engine [Object].

- **Global Tag Registry:** Armazena todas as tags registradas no projeto.
- **Project Settings:** Integrado às configurações do projeto para persistência de definições de tags.

### 2.1 AbilitySystemComponent (ASC)

O "cérebro" do Ator. Deve ser anexado a um `CharacterBody2D/3D` [Node].

- **API de Registro (Setup Manual):**
  - `set_animation_player(Node)`: Define o slot padrão para animações (aceita `AnimationPlayer` ou `AnimatedSprite`).
  - `set_audio_player(Node)`: Define le slot padrão para áudio (aceita `AudioStreamPlayer`, `2D` ou `3D`).
  - `register_node(StringName name, Node)`: Registra um nó nomeado para Cues específicos.
- **API de Feedback (Execução):**
  - `play_montage(StringName anim, Node target)`: Toca animação. Se `target` nulo, usa o slot padrão.
  - `play_sound(AudioStream sound, Node target)`: Toca áudio. Se `target` nulo, usa o slot padrão.
- **API Principal (Execução):**
  - `give_ability(AbilitySystemAbility ability)`: Registra uma nova habilidade.
  - `try_activate_ability(AbilitySystemAbility ability)`: Tenta executar a habilidade passada.
  - `apply_effect_spec_to_self(AbilitySystemEffectSpec spec)`: Aplica modificadores.
- **Signals Reativos:**
  - `attribute_changed(name, old, new)`: Para UI e Feedback.
  - `tag_changed(name, present)`: Para mudanças de estado visual.

> [!NOTE]
> Os métodos `play_montage` e `play_sound` são públicos e podem ser usados via script, mas o fluxo recomendado é o **Data-Driven**, onde as Habilidades e Efeitos disparam esses métodos internamente através de Cues.

### 2.2 AbilitySystemAbilityContainer (Resource)

Define o **contrato de permissão** (arquétipo) de uma entidade, catalogando tudo o que ela é capaz de possuir ou realizar.

- **Propósito:** Atua como um "blueprint" ou "template" fixo para raças, classes ou tipos de inimigos. Ele define o escopo de possibilidades, permitindo que o jogo gerencie o desbloqueio progressivo (via nível ou classe) de forma segura.
- **Conteúdo (Catálogo):**
  - `abilities`: Lista de habilidades que a entidade **pode** aprender ou equipar (catálogo de acesso).
  - `effects`: Catálogo de efeitos, passivas e **instâncias de ataque** permitidas (ex: veneno, escudos, buffs), que o arquétipo está autorizado a carregar ou instanciar.
  - `attributes`: Atributos permitidos e suas definições para este arquétipo.
  - `tags`: Tags de identidade e permissão inerentes ao arquétipo.
  - `cues`: Mapeamento de feedbacks audiovisuais disponíveis para o arquétipo.

### 2.3 AbilitySystemAttribute

Define um atributo individual com schema, limites e validação.

- **Propósito:** Resource de definição para estatísticas (HP, Mana, Stamina).
- **Propriedades:**
  - `name`: Identificador único do atributo.
  - `base_value`: Valor base inicial.
  - `min_value`/`max_value`: Limites para validação automática.
- **Validação:** Métodos `is_valid_value()` e `clamp_value()` para garantir consistência.
- **Uso:** Configurável no Editor, reutilizável entre múltiplos personagens.

### 2.4 AbilitySystemAttributeSet

Container que gerencia coleções de AbilitySystemAttribute Resources.

- **Modularidade:** Suporta múltiplos sets por AbilitySystemComponent.
- **Validação Obrigatória:** Exige que atributos sejam definidos como Resources antes do uso.
- **API Resource-based:**
  - `add_attribute_definition(AbilitySystemAttribute)`: Registra definição.
  - `get_attribute_definition(name)`: Obtém Resource de definição.
  - `set_attribute_base_value(name, value)`: Manipula com validação automática.
- **Runtime Performance:** Mantém HashMap interno para valores (base/current) com validação via Resource.
- **Sinais:** `attribute_changed`, `attribute_pre_change` para reatividade.

### 2.5 AbilitySystemAbility (Resource)

Define a lógica de "o que acontece" e principalmente "o que esta habilidade é".

- **Identidade vs Comportamento:** Uma habilidade no Ability System se define primeiramente pelo seu `Ability Tag` único, que atua como o seu "Documento de Identidade". O restante dos campos de Tags (os Arrays de Ativação) não ditam *quem* a Habilidade é, mas sim o seu *comportamento* mediante o personagem e o cenário.
- **Hierarquia do Polimorfismo:** Por causa do formato das Tags (e.g. `ability.spell.fire.fireball`), a Engine lida facilmente com grupos ou classificadores sem usar um Array de Tags na Identidade. Se o player tentar ativar `ability.spell` no ASC, todas as sub-skills de magia respondem. Se quiser especificidade, chame de forma integral: `ability.spell.fire`.

**Propriedades de Controle de Estado e Condições (As "Activation Tags"):**

- **`ability_tag` (O Registro Único):** Tag principal e singular formadora de identidade e filtragem (`StringName`). Exemplo: `ability.movement.dodge`
- **`activation_owned_tags` (O Buff Temporário):** Array de Tags. Quando a habilidade iniciar, ela carimba todo este grupo no Character e os remove assim que terminar o ciclo de vida. Dita o que seu Personagem "se torna" (ex: durante a Esquiva, ganha *state.immune.damage* e *state.animation.rolling*).
- **`activation_required_tags` (Os Requisitos):** Array de Tags. A habilidade recusa ativar se tentar ser invocada por um Player que não detenha todas estas tags listadas (Pode exigir *state.combat.advantage* ou algo nativo como *state.in_air*).
- **`activation_blocked_tags` (O CC/Block):** Array de Tags. A habilidade recusa ativar se o usuário tentar usá-la enquanto estiver sofrendo de qualquer uma destas Tags (As famosas janelas de Stun, Silence e Root - *state.debuff.stun*).

**Outras Estruturas Core:**

- **Fluxo de Vida:** `can_activate` -> `activate_ability` -> `end_ability`.
- **Encapsulamento Audiovisual:** Contém uma lista de `cues` executados automaticamente.
- **Restritores Físicos e Lógicos (Resources Conexos):**
  - `cost_effect`: Efeito aplicado como custo na barra de mana/stamina no momento de tentar a Ativação.
  - `cooldown_effect`: Efeito aplicado no usuário ao tentar engatilhar o gatilho pós-uso.
- **Métodos Virtuais:** `_can_activate_ability()`, `_activate_ability()`, `_on_end_ability()`.

### 2.6 AbilitySystemEffect (Resource)

Define as regras e o "pacote" de alteração de um personagem:

- **Lógica de Origem:** Registrado no `AbilitySystemAbilityContainer` de quem **causa** o efeito. O Sniper tem o "efeito tiro" em seu catálogo; o inimigo apenas recebe o impacto do cálculo via código.
- **Variedade:**
  - **Dano Único** (Sniper) vs **Dano Contínuo/DOT** (Veneno, Área de Fogo).
  - **Buffs de Status** (Força, Escudos de Proteção).
- **Políticas de duração:**
  - `INSTANT` — Ex: Um tiro de sniper (dano único registrado na arma/personagem).
  - `DURATION` — Ex: Uma flechada envenenada ou um escudo protetor temporário.
  - `INFINITE` — Ex: Um encantamento de força ou aura permanente.
- **Modificadores:** Operações de `ADD`, `MULTIPLY`, `DIVIDE` e `OVERRIDE`.
- **Costs:** Custos de ativação (Mana, Stamina, etc.).
- **Cooldowns:** Tempo de recarga entre usos.
- **Tags:** `granted_tags`, `removed_tags`, `blocked_tags`.
- **Cues:** `cue_tags` para disparar feedback visual/sonoro.
- **Magnitude Custom:** `AbilitySystemMagnitudeCalculation` para fórmulas complexas.

### 2.7 AbilitySystemCue (Resource)

Sistema de ativação e sincronização de eventos (animações, sons) disparado por efeitos ou habilidades.

- **Função:** Sistema para ativação e sincronização de eventos de gameplay.
- **Dependência:** Requer classes especializadas (CueAnimation, CueAudio, CueParticles [em planejamento]) para funcionalidade real.
- **Trigger:** Executado quando efeitos são aplicados/removidos ou habilidades ativadas.
- **Tipos:** `ON_EXECUTE` (instantâneo), `ON_ACTIVE` (enquanto ativo), `ON_REMOVE` (ao remover).
- **Arquitetura:** Fornece estrutura base mas não implementa funcionalidade específica.
- **Métodos Virtuais:** `_on_execute()`, `_on_active()`, `_on_remove()` para customização via script.
- **Uso:** Herança para criar cues especializados ou customização via GDScript/C#.

#### 2.7.1 AbilitySystemCueAnimation (Resource)

Especializado em executar animações.

- **Propriedade:** `animation_name` (StringName) - Nome da animação.
- **Propriedade:** `node_name` (StringName) - Nome do nó alvo registrado no ASC (opcional).
- **Execução:** O sistema chama internamente `asc->play_montage(animation_name, target)`. Se `node_name` estiver vazio, usa o `AnimationPlayer` padrão do ASC.

#### 2.7.2 AbilitySystemCueAudio (Resource)

Especializado em reproduzir áudios.

- **Propriedade:** `audio_stream` (Ref<AudioStream>) - Recurso de áudio.
- **Propriedade:** `node_name` (StringName) - Nome do nó alvo registrado no ASC (opcional).
- **Execução:** O sistema chama internamente `asc->play_sound(audio_stream, target)`. Se `node_name` estiver vazio, usa o `AudioStreamPlayer` padrão do ASC.

**Nota de Fluxo:** Play Montage e Play Sound são a "ponta final" da execução de feedback do sistema. Embora públicos no ASC, seu uso via Cues (Data-Driven) é preferível para manter a lógica desacoplada do código de animação/som.

### 2.8 AbilitySystemTag

Identificador único para estados do jogo [RefCounted].

- **Hierárquico:** Ex: `state.debuff.poison`, `ability.fireball`.
- **Otimizado:** `StringName` puro com armazenamento direto em `HashSet` para máxima performance.
- **Global:** Gerenciado centralmente pelo singleton `AbilitySystem`.
- **Strict-Mode:** No editor, seletores garantem que apenas tags registradas no `AbilitySystemTagsEditor` sejam utilizadas.

### 2.9 AbilitySystemTagContainer

Container otimizado de tags para consultas e modificações em runtime [RefCounted].

- **Gerenciamento:** `add_tag()`, `remove_tag()` com reference counting.
- **Consultas:** `has_tag()`, `has_any_tags()`, `has_all_tags()`.
- **Matching:** Suporte a matching hierárquico (`exact` parameter).
- **Performance:** Estrutura otimizada para alta frequência de operações.

### 2.10 AbilitySystemTargetData

Container para dados de targeting em multiplayer [RefCounted].

- **Dados:** `hit_position` (Vector3), `target_node` (Object).
- **Uso:** Comunicação client-server de hits e seleções.

### 2.11 AbilitySystemTask

Base para tarefas assíncronas em habilidades [RefCounted].

- **Tipos:** `TASK_WAIT_DELAY`, `TASK_PLAY_MONTAGE`, `TASK_WAIT_EVENT`.
- **Métodos Estáticos:** `wait_delay()`, `play_montage()`.
- **Sinais:** `completed` para coordenação assíncrona.
- **Extensível:** Herança para tarefas customizadas.

### 2.12 AbilitySystemMagnitudeCalculation

Lógica customizada para cálculo de magnitudes (MMC) [RefCounted].

- **Método Virtual:** `_calculate_magnitude(spec)`.
- **Utilitários:** `get_source_attribute_value()`, `get_target_attribute_value()`.
- **Uso:** Escalar dano por atributos, curvas de progressão, modificadores ambientais.

## 3. Sistema de Runtime (Specs)

### 3.1 AbilitySystemAbilitySpec

Instância de runtime de uma `AbilitySystemAbility` concedida ao ASC.

- **Dados:** Nível, estado ativo, referência ao Resource.
- **Métodos:** `get_ability()`, `get_is_active()`, `get_level()`.
- **Ciclo:** Gerenciado pelo `AbilitySystemComponent`.

### 3.2 AbilitySystemEffectSpec

Instância de runtime de um `AbilitySystemEffect` aplicado.

- **Dados:** Duração restante, magnitudes dinâmicas, nível.
- **Contexto:** Source/target ASCs, cálculos de magnitude.
- **Métodos:** `get_duration_remaining()`, `set_magnitude()`, `calculate_modifier_magnitude()`.

### 3.3 AbilitySystemCueSpec

Contexto de execução de uma `AbilitySystemCue`.

- **Dados:** Cue resource, effect spec trigger, magnitude calculada.
- **Contexto:** Source ASC, target ASC, dados extras.
- **Uso:** Passado para callbacks da Cue com contexto completo.

## 4. Ability System Tags

As **Tags do Ability System** são `StringName` hierárquicos otimizados.

- **Exemplo:** `state.buff.speed`, `state.debuff.stun`.
- **Registro:** Gerenciados pelo singleton `AbilitySystem`.
- **Matching:** Suporte a matching hierárquico via `AbilitySystem.tag_matches()`.
- **Performance:** Otimizados para consultas de alta frequência sem overhead de classes.
- **Workflow:** Registre as tags no Project Settings e use os seletores automáticos nos Resources.

## 5. Ferramentas do Editor

### 5.1 AbilitySystemEditorPlugin

Integração nativa com o **Inspector** da Godot para automatização de UI:

- **Seletores Unificados:** Qualquer propriedade terminada em `_tag` (StringName) ou `_tags` (Array de StringName) recebe automaticamente um seletor dropdown.
- **Sincronização em Real-time:** O seletor se conecta ao sinal `tags_changed` do `AbilitySystem`, garantindo que novos registros no Tag Editor apareçam instantaneamente no Inspetor sem reiniciar a engine.
- **Validação Visual:** Tags selecionadas que não existem mais no registro global são marcadas com um ícone de erro e aviso "(Inexistente)".

### 5.2 AbilitySystemTagsEditor (Tag Registry)

Localizado em **Project Settings > Ability System Tags**, é a central de verdade do sistema:

- **Gestão Centralizada:** Adicione ou remova tags globais que serão usadas em todo o projeto.
- **Persistência Segura:** As tags são salvas nas configurações do projeto (`project.godot`), garantindo que a equipe de design tenha o mesmo catálogo validado.
- **Prevenção de Erros:** Evita o uso de "Magic Strings" espalhadas pelo projeto, forçando um workflow baseado em registro prévio.

### 5.3 AbilitySystemTagsSelector (Integração de Propriedades Array)

Componente de UI customizado puro C++ (herdeiro de `EditorProperty`) que intercede no inspetor da Engine substituindo o editor padrão de Arrays para propriedades terminadas em `_tags`.

- **Seleção Multi-Tags Dinâmica:** Oferece um botão que abre um dialog flutuante. O layout conta com uma `Tree` listando todas as tags cadastradas e checkboxes nativos, suportando múltiplas inserções de uma só vez.
- **Filtro em Tempo Real:** Possui campo de pesquisa dinâmico acoplado no topo da lista que mascara e filtra as tags (ex: ao digitar "poison", todas as sub-tags associadas surgem para marcação rápida).
- **Sumário Visual Constante:** Mantém sua interface limpa quando colapsada, exibindo diretamente a estatística das marcações (ex: "4 Tags Selected" ou "(Empty Tags)").

## 6. Sinais e Eventos

### 6.1 AbilitySystemComponent Signals

- **`ability_activated(spec)`**: Habilidade iniciada.
- **`ability_ended(spec, was_cancelled)`**: Habilidade finalizada.
- **`ability_failed(name, reason)`**: Falha na ativação.
- **`attribute_changed(name, old, new)`**: Mudança de atributo.
- **`tag_changed(name, present)`**: Adição/remoção de tag.
- **`effect_applied(spec)`**: Efeito aplicado.
- **`effect_removed(spec)`**: Efeito removido.
- **`tag_event_received(tag, data)`**: Evento genérico disparado por Cues ou manualmente.
- **`cooldown_started(tag, duration)`**: Início de cooldown.
- **`cooldown_ended(tag)`**: Fim de cooldown.

### 6.2 AbilitySystemAttributeSet Signals

- **`attribute_changed(attribute_name, old_value, new_value)`**: Disparado em qualquer mudança.
- **`attribute_pre_change(attribute_name, old_value, new_value)`**: Antes da mudança (pode ser cancelado).
- **Uso:** Atualizar UI (health bars), triggers de lógica (morte quando HP = 0).

## 7. Interface de Script (GDScript)

### Exemplo: Pipeline de Combate com Atributos Validados

```gdscript
func setup_character():
    # Cache manual via @onready
    asc.set_animation_player($AnimationPlayer)
    asc.set_audio_player($AudioStreamPlayer)

    # Registrar nós específicos para Cues
    asc.register_node(&"weapon_vfx", $Weapon/Particles)

    # Criar atributo com limites
    var health_attr = AbilitySystemAttribute.new()
    ...
```

### Exemplo: Habilidades Assíncronas (Montagens e Eventos)

```gdscript
func _activate_ability():
    # Cache interno do AbilitySystemComponent lida com a busca de nós irmãos (AnimationPlayer)
    asc.play_montage("attack_swing")

    # O uso de await permite lógica linear AAA (ex: aguardar animação ou sinal)
    await asc.tag_event_received # Exemplo genérico, o sistema de Tasks é o foco aqui

    _apply_area_damage()
    end_ability()
```

### Exemplo: UI Reativa com Atributos Type-Safe

```gdscript
func _ready():
    # UI passiva conectada aos sinais de alta performance do C++
    asc.attribute_changed.connect(func(attr, old, new):
        if attr == &"Health":
            update_health_bar(new)
        elif attr == &"Mana":
            update_mana_bar(new)
    )

    # Acesso seguro com validação
    var health_def = asc.get_attribute_set().get_attribute_definition("Health")
    if health_def:
        print("Health limits: ", health_def.min_value, " - ", health_def.max_value)
```

### Exemplo: Cues Especializados para Feedback Audiovisual

```gdscript
func setup_combat_cues():
    # Cue de animação para ataque
    var attack_anim = AbilitySystemCueAnimation.new()
    attack_anim.cue_tag = &"combat.attack"
    attack_anim.animation_name = "sword_slash"
    attack_anim.event_type = AbilitySystemCue.ON_EXECUTE
    asc.register_cue_resource(attack_anim)

    # Cue de áudio para impacto
    var impact_sound = AbilitySystemCueAudio.new()
    impact_sound.cue_tag = &"combat.impact"
    impact_sound.audio_stream = load("res://sounds/sword_hit.wav")
    impact_sound.event_type = AbilitySystemCue.ON_EXECUTE
    asc.register_cue_resource(impact_sound)


func activate_fireball():
    # Dispara habilidade que automaticamente executa os cues
    asc.try_activate_ability_by_tag(&"spell.fireball")
    # Resultado: Toca animação + som + partículas automaticamente
```

## 8. Sincronização e Persistência

Integrado nativamente com o `SaveServer` para persistência automática de atributos e estados de habilidades.

---

*Este documento define a especificação técnica oficial do módulo Ability System na Zyris Engine.*
