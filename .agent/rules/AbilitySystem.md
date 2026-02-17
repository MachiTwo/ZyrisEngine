---
trigger: always_on
---

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

O ponto central de verdade do sistema. Gerenciado como um Singleton da Engine.

- **Global Tag Registry:** Armazena todas as tags registradas no projeto.
- **Project Settings:** Integrado às configurações do projeto para persistência de definições de tags.

### 2.1 AbilitySystemComponent (ASC)

O "cérebro" do Ator. Deve ser anexado a um `CharacterBody2D/3D`.

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

### 2.1.1 AbilitySystemAbilityContainer (Resource)

Define o arquétipo de um personagem, agrupando habilidades e atributos iniciais.

- **Propósito:** Configuração base de classes (Guerreiro, Mago) ou inimigos.
- **Conteúdo:**
  - `granted_abilities`: Lista de habilidades concedidas.
  - `innate_effects`: Efeitos passivos aplicados ao iniciar.
  - `initial_attributes`: Valores base para atributos (Força, Vigor).
  - `cues`: Mapeamento de cues visuais/sonoros.

### 2.2 AbilitySystemAttribute

Define um atributo individual com schema, limites e validação.

- **Propósito:** Resource de definição para estatísticas (HP, Mana, Stamina).
- **Propriedades:**
  - `name`: Identificador único do atributo.
  - `base_value`: Valor base inicial.
  - `min_value`/`max_value`: Limites para validação automática.
- **Validação:** Métodos `is_valid_value()` e `clamp_value()` para garantir consistência.
- **Uso:** Configurável no Editor, reutilizável entre múltiplos personagens.

### 2.3 AbilitySystemAttributeSet

Container que gerencia coleções de AbilitySystemAttribute Resources.

- **Modularidade:** Suporta múltiplos sets por AbilitySystemComponent.
- **Validação Obrigatória:** Exige que atributos sejam definidos como Resources antes do uso.
- **API Resource-based:**
  - `add_attribute_definition(AbilitySystemAttribute)`: Registra definição.
  - `get_attribute_definition(name)`: Obtém Resource de definição.
  - `set_attribute_base_value(name, value)`: Manipula com validação automática.
- **Runtime Performance:** Mantém HashMap interno para valores (base/current) com validação via Resource.

### 2.4 AbilitySystemAbility (Resource)

Define a lógica de "o que acontece".

- **Fluxo de Vida:** `can_activate` -> `activate_ability` -> `end_ability`.
- **Tags:** Gerencia tags de habilidade, cancelamento e bloqueio.

### 2.5 AbilitySystemEffect (Resource)

Regras para alterar atributos ou tags.

- **Duração:** Instantâneo, Duração ou Infinito.
- **Modificadores:** Operações de `ADD`, `MULTIPLY`, `DIVIDE` e `OVERRIDE`.
- **Costs:** Custos de ativação (Mana, Stamina, etc.).
- **Cooldowns:** Tempo de recarga entre usos.

### 2.6 AbilitySystemCue (Resource)

Sistema de feedback visual e sonoro para eventos de gameplay.

- **Trigger:** Executado quando efeitos são aplicados/removidos ou habilidades ativadas.
- **Tipos:** `OnActive` (enquanto ativo), `OnExecute` (instantâneo), `OnRemove` (ao remover).
- **Uso:** Spawn de VFX, SFX, screen shake, números flutuantes.

### 2.7 Attribute Events

Signals emitidos quando atributos mudam.

- **`attribute_changed(attribute_name, old_value, new_value)`**: Disparado em qualquer mudança.
- **`attribute_pre_change(attribute_name, old_value, new_value)`**: Antes da mudança (pode ser cancelado).
- **Uso:** Atualizar UI (health bars), triggers de gameplay (morte quando HP = 0).

### 2.8 Gameplay Tasks

Sistema assíncrono para operações de longa duração dentro de habilidades.

- **Tasks:** `WaitDelay`, `WaitInputPress`, `MoveToLocation`, `SpawnProjectile`.
- **Async:** Permite escrever lógica de habilidade linear usando `await`.

### 2.9 Target Data

Pipeline de validação e filtragem de alvos.

- **Fluxo:** O Ator seleciona alvos através de logic handles.
- **Support:** Line Trace, Sphere Overlap, Box, Custom Shapes.

### 2.10 Magnitude Calculation (MMC)

Cálculos complexos de magnitude para efeitos.

- **Custom logic:** Permite definir curvas de dano, escala por atributos (Força, Inteligência) e modificadores de ambiente.
- **GEE:** Gameplay Effect Execution para lógica arbitrária ao aplicar efeitos.

## 3. Gameplay Tags

As **GameplayTags** são `StringName` hierárquicos otimizados.

- **Exemplo:** `state.buff.speed`, `state.debuff.stun`.

## 4. Interface de Script (GDScript)

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

## 5. Ferramentas do Editor

O módulo foca na integração nativa com o **Inspector** da Godot para facilitar o desenvolvimento:

- **AbilitySystemEditorPlugin:** Customiza o Inspector lateral para fornecer seletores especializados de Tags e Atributos.
- **Project Settings:** As Gameplay Tags globais são gerenciadas e registradas via singleton central [AbilitySystem].

## 6. Sincronização e Persistência

Integrado nativamente com o `SaveServer` para persistência automática de atributos e estados de habilidades.

---

_Este documento define a especificação técnica oficial do módulo GAS na Zyris Engine._
