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

- **Integração Nativa (Sibling Cache):**
  - Detecta automaticamente e cacheia: `AnimationPlayer`, `AnimatedSprite`, `AudioStreamPlayer`, `RayCast`, `GPUParticles`, `Area`, `Marker`.
- **API Principal:**
  - `give_ability(AbilitySystemAbility ability)`: Registra uma nova habilidade.
  - `try_activate_ability_by_tag(StringName tag)`: Tenta disparar uma habilidade via tag.
  - `make_outgoing_spec(AbilitySystemEffect effect)`: Cria uma especificação para aplicação dinâmica.
  - `apply_gameplay_effect_spec_to_self(AbilitySystemEffectSpec spec)`: Aplica modificadores.
  - `play_montage(StringName anim)`: Toca animação no nó irmão cacheado.
- **Signals Reativos:**
  - `attribute_changed(name, old, new)`: Para UI e Feedback.
  - `tag_changed(name, present)`: Para mudanças de estado visual.

### 2.2 AbilitySystemAbilityContainer (Resource)

Define o arquétipo de um personagem, agrupando habilidades e atributos iniciais.

- **Propósito:** Configuração base de classes (Guerreiro, Mago) ou inimigos.
- **Conteúdo:**
  - `granted_abilities`: Lista de habilidades concedidas.
  - `innate_effects`: Efeitos passivos aplicados ao iniciar.
  - `initial_attributes`: Valores base para atributos (Força, Vigor).
  - `innate_tags`: Tags concedidas permanentemente.
  - `cues`: Mapeamento de cues visuais/sonoros.

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

Define a lógica de "o que acontece".

- **Fluxo de Vida:** `can_activate` -> `activate_ability` -> `end_ability`.
- **Tags:** Gerencia tags de habilidade, cancelamento e bloqueio.
- **Propriedades:**
  - `ability_tag`: Tag única identificadora.
  - `activation_required_tags`: Tags necessárias para ativação.
  - `activation_blocked_tags`: Tags que bloqueiam ativação.
  - `activation_owned_tags`: Tags concedidas durante ativação.
  - `cost_effect`: Efeito aplicado como custo.
  - `cooldown_effect`: Efeito aplicado como cooldown.
- **Métodos Virtuais:** `_can_activate_ability()`, `_activate_ability()`, `_on_end_ability()`.

### 2.6 AbilitySystemEffect (Resource)

Regras para alterar atributos ou tags.

- **Duração:** Instantâneo, Duração ou Infinito.
- **Modificadores:** Operações de `ADD`, `MULTIPLY`, `DIVIDE` e `OVERRIDE`.
- **Costs:** Custos de ativação (Mana, Stamina, etc.).
- **Cooldowns:** Tempo de recarga entre usos.
- **Tags:** `granted_tags`, `removed_tags`, `blocked_tags`.
- **Cues:** `cue_tags` para disparar feedback visual/sonoro.
- **Magnitude Custom:** `AbilitySystemMagnitudeCalculation` para fórmulas complexas.

### 2.7 AbilitySystemCue (Resource)

Sistema de feedback visual e sonoro para eventos de gameplay.

- **Trigger:** Executado quando efeitos são aplicados/removidos ou habilidades ativadas.
- **Tipos:** `ON_EXECUTE` (instantâneo), `ON_ACTIVE` (enquanto ativo), `ON_REMOVE` (ao remover).
- **Uso:** Spawn de VFX, SFX, screen shake, números flutuantes.
- **Métodos Virtuais:** `_on_execute()`, `_on_active()`, `_on_remove()`.

### 2.8 AbilitySystemTag

Identificador único para estados de gameplay [RefCounted].

- **Hierárquico:** Ex: `state.debuff.poison`, `ability.fireball`.
- **Otimizado:** `StringName` para performance.
- **Global:** Registrado no singleton `AbilitySystem`.

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

## 4. Gameplay Tags

As **GameplayTags** são `StringName` hierárquicos otimizados.

- **Exemplo:** `state.buff.speed`, `state.debuff.stun`.
- **Registro:** Gerenciados pelo singleton `AbilitySystem`.
- **Matching:** Suporte a matching hierárquico via `AbilitySystem.tag_matches()`.
- **Performance:** Otimizados para consultas de alta frequência.

## 5. Ferramentas do Editor

### 5.1 AbilitySystemEditorPlugin

Integração nativa com o **Inspector** da Godot para facilitar o desenvolvimento:

- **Inspector Customizado:** Seletores especializados de Tags e Atributos.
- **Project Settings:** As Gameplay Tags globais são gerenciadas e registradas via singleton central [AbilitySystem].
- **Validação Visual:** Feedback imediato de configurações inválidas.

## 6. Sinais e Eventos

### 6.1 AbilitySystemComponent Signals

- **`ability_activated(spec)`**: Habilidade iniciada.
- **`ability_ended(spec, was_cancelled)`**: Habilidade finalizada.
- **`ability_failed(name, reason)`**: Falha na ativação.
- **`attribute_changed(name, old, new)`**: Mudança de atributo.
- **`tag_changed(name, present)`**: Adição/remoção de tag.
- **`effect_applied(spec)`**: Efeito aplicado.
- **`effect_removed(spec)`**: Efeito removido.
- **`gameplay_event_received(tag, data)`**: Evento genérico.
- **`cooldown_started(tag, duration)`**: Início de cooldown.
- **`cooldown_ended(tag)`**: Fim de cooldown.

### 6.2 AbilitySystemAttributeSet Signals

- **`attribute_changed(attribute_name, old_value, new_value)`**: Disparado em qualquer mudança.
- **`attribute_pre_change(attribute_name, old_value, new_value)`**: Antes da mudança (pode ser cancelado).
- **Uso:** Atualizar UI (health bars), triggers de gameplay (morte quando HP = 0).

## 7. Interface de Script (GDScript)

### Exemplo: Pipeline de Combate com Atributos Validados

```gdscript
func setup_character():
    # Criar atributo com limites
    var health_attr = AbilitySystemAttribute.new()
    health_attr.name = "Health"
    health_attr.min_value = 0.0
    health_attr.max_value = 1000.0

    # Adicionar ao AttributeSet
    var attr_set = AbilitySystemAttributeSet.new()
    attr_set.add_attribute_definition(health_attr)

    # Valores são automaticamente validados
    attr_set.set_attribute_base_value("Health", 150.0)  # OK
    # attr_set.set_attribute_base_value("Health", 1500.0)  # Error!

func apply_damage(target: AbilitySystemComponent, amount: float):
    # Efeitos baseados em dados (.tres) para facilidade de design
    var spec = asc.make_outgoing_spec(load("res://effects/damage_fire.tres"))

    # Magnitudes dinâmicas (MMC) com validação automática
    spec.set_magnitude("base_damage", amount * asc.get_attribute_base_value("Health"))

    asc.apply_gameplay_effect_spec_to_target(spec, target)
```

### Exemplo: Habilidades Assíncronas (Montagens e Eventos)

```gdscript
func _activate_ability():
    # Cache interno do AbilitySystemComponent lida com a busca de nós irmãos (AnimationPlayer)
    asc.play_montage("attack_swing")

    # O uso de await com 'wait_for_gameplay_event' permite lógica linear AAA
    var impact = await asc.wait_for_gameplay_event("event.combat.impact").completed

    _apply_area_damage(impact.position)
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

## 8. Sincronização e Persistência

Integrado nativamente com o `SaveServer` para persistência automática de atributos e estados de habilidades.

## 9. Integração Multiplayer

O Ability System foi projetado com suporte nativo para multiplayer autoritativo:

- **Validação Servidor:** Todas as ativações são validadas no servidor.
- **Predição Client:** Predição local de habilidades para responsividade.
- **Rollback:** Reversão de estados se rejeitado pelo servidor.
- **Replicação:** Sincronização eficiente de estados entre clientes.

---

_Este documento define a especificação técnica oficial do módulo GAS na Zyris Engine._
