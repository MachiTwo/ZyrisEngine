🚀 *Ability System — Lógica Orientada a Dados*

O *Ability System* é o framework oficial da Zyris Engine para criação de combate, habilidades e atributos.

Projetado para escalar desde mecânicas simples (pulo, dash, tiro) até sistemas completos de RPG com buffs, debuffs, cooldowns e cálculos avançados de dano — tudo com alta performance, arquitetura modular e totalmente *orientado a dados*.

---

🧠 *Estrutura Principal*

*AbilitySystem (Singleton)*

Singleton global responsável por:

- Registro e organização das `AbilitySystemTag`
- Configurações globais do sistema
- Tipagem e validação das Tags disponíveis
- *Sinal Global:* `tags_changed` (Notifica o editor para atualizar seletores)

Garante consistência e validação em todo o projeto.

---

*AbilitySystemComponent (ASC)*

Componente anexado a *Personagens* (`CharacterBody2D` / `CharacterBody3D`). É o processador central da entidade.

- Concede habilidades
- Aplica efeitos
- Processa atributos
- Dispara Cues (Automático)
- Gerencia Specs em runtime
- *API de Registro:* `set_animation_player(node)`, `set_audio_player(node)`, `register_node(name, node)`

*API Principal:*

- `give_ability(ability, level)` — Registra habilidade
- `try_activate_ability_by_tag(tag)` — Tenta ativar
- `play_montage(name, target)` — Feedback (Auto-fallback para slot padrão)
- `play_sound(audiostream, target)` — Feedback (Auto-fallback para slot padrão)
- `execute_cue(tag, data)` — Dispara Cue manual
- `add_tag(tag)` / `has_tag(tag)` — Gerencia tags

---

🧠 *Tags:*
As Tags são os **identificadores** e **estados** (buffs/debuffs) do personagem.

- *Otimizadas:* Usam `StringName` (ID numérico interno) para busca ultrarrápida.
- *Hierárquicas:* Suportam níveis (ex: `state.buff.speed`).
- *Strict Selection:* O sistema agora detecta automaticamente propriedades `_tag` e `_tags` no Inspetor e oferece um seletor baseado no registro global.

*AbilitySystemAbilityContainer*

Define o *contrato de permissão* (blueprint) de uma entidade. Funciona como o catálogo completo de tudo o que aquele arquétipo (ex: uma Raça ou Classe) *pode* ter.

- *Não* concede acesso imediato — apenas define o que é permitido desbloquear.
- *Propósito:* Permite que o sistema lide com níveis e classes separadamente em paralelo, consultando o Container para saber se uma habilidade é válida para aquele personagem.
- *Armazena:*
  - `abilities`: Catálogo de habilidades disponíveis para o arquétipo.
  - `effects`: Catálogo de efeitos, passivas e *instâncias de impacto* permitidas (ex: veneno, escudos, buffs).
  - `attributes`: Definições de atributos compatíveis.
  - `tags/cues`: Identidade e recursos visuais autorizados.

*"O Container é o teto de possibilidades; o progresso do jogo (level/classe) é quem decide o que o personagem realmente possui em um dado momento."*

---

*AbilitySystemAttribute*

Define estatísticas individuais com schema, limites e validação:

- `HP`, `Mana`, `Stamina`, `Level`...
- Cada atributo possui:
  - *BaseValue* — Valor base
  - *CurrentValue* — Valor modificado por Effects ativos

*Propriedades:*

- `name` — Identificador único
- `base_value` — Valor inicial
- `min_value` / `max_value` — Limites para validação automática

*Métodos:* `is_valid_value(v)`, `clamp_value(v)`

---

*AbilitySystemAttributeSet*

O estado *atual e concreto* do personagem. Enquanto o Container é a "lei", o AttributeSet é a "execução":

- Define os *valores base iniciais* (HP, Mana) conforme a classe/level.
- Armazena as *habilidades efetivamente desbloqueadas* e prontas para uso.
- Gerencia o estado de runtime dos atributos e suas validações (`min`/`max`).

*API:*

- `add_attribute_definition(attr)` — Registra definição
- `get_attribute_definition(name)` — Obtém Resource de definição
- `set_attribute_base_value(name, value)` — Altera com validação automática
- `get_attribute_value(name)` — Lê o valor atual

*Se o `AbilityContainer` diz "o que uma raça pode aprender", o `AttributeSet` diz "o que este personagem já aprendeu e quanto de HP ele tem agora".*

---

*AbilitySystemAbility (Resource)*

Define a lógica modular de uma habilidade:

- *Custo* — Effect aplicado como custo
- *Cooldown* — Tempo de recarga
- *Ciclo de vida:* `can_activate` → `activate` → `end`

*Propriedades de Tags:*

- `ability_tag` — Tag única identificadora
- `activation_required_tags` — Tags necessárias para ativar
- `activation_blocked_tags` — Tags que bloqueiam ativação
- `activation_owned_tags` — Tags concedidas durante ativação
- `cues` — Lista de Resources de Cue (Execução Automática)

*Métodos Virtuais:* `_can_activate_ability()`, `_activate_ability()`, `_on_end_ability()`

Pode usar *Tasks* para comportamentos assíncronos:

- Esperar input
- Spawnar projétil
- Aguardar animação
- Movimentação especial

---

*AbilitySystemEffect (Resource)*

Define as regras e o "pacote" de alteração de uma entidade:

- *Lógica de Origem:* Registrado no `Container` de quem *causa* o efeito. O Sniper tem o "efeito tiro" em seu catálogo; o inimigo apenas recebe o impacto do cálculo via código.
- *Variedade:*
  - *Dano Único* (Sniper) vs *Dano Contínuo/DOT* (Veneno, Área de Fogo).
  - *Buffs/Shields* (Aumento de Força, Escudos de Proteção).
- *Políticas de duração:*
  - `INSTANT` — Aplicado e descartado (ex: Tiro de Sniper).
  - `DURATION` — Ativo por tempo definido (ex: Veneno, Shield).
  - `INFINITE` — Ativo até remoção explícita (ex: Encantamento, Aura).

*Modificadores:* `ADD`, `MULTIPLY`, `DIVIDE`, `OVERRIDE`
*Tags:* `granted_tags`, `removed_tags`, `blocked_tags`, `cue_tags`
*Magnitude Custom:* via `AbilitySystemMagnitudeCalculation`

---

*AbilitySystemTag*

Sistema de validação baseado em Tags hierárquicas (`StringName` otimizado).

Um personagem não precisa saber *qual* ataque recebeu. Ele apenas verifica:

*Tenho a tag `state.debuff.burning`? Então recebo dano por segundo.*

Isso mantém o sistema *desacoplado e escalável*.

---

*AbilitySystemCue (Resource)*

Sistema de ativação e sincronização de eventos disparado por efeitos ou habilidades:

- *CueAnimation* — Executa animação no nó alvo
- *CueAudio* — Executa som no nó alvo

*API Pública vs Interna:*

- O programador usa a API de alto nível (`activate_ability`).
- O sistema usa a API de feedback (`play_montage`/`play_sound`) internamente através dos Cues.
- Scripts podem usar `play_montage` e `play_sound` diretamente para feedbacks rápidos sem precisar de um Resource de Cue.

---

⚙️ *Runtime (Specs)*

Os *Specs* são instâncias em tempo real dos Resources. Permitem que o Resource original permaneça *imutável*.

*AbilitySpec*
→ Estado ativo, nível, referência ao Resource

*EffectSpec*
→ ASC fonte/alvo, magnitude calculada, duração restante

*CueSpec*
→ Cue resource, context de execução, magnitude

---

🛠 *Ferramentas do Editor*

*AbilitySystemTagsEditor*
Localizado em **Project Settings**, é onde você registra as tags oficiais do seu jogo.

- Evita erros de digitação (Magic Strings).
- Atualização instantânea em todos os seletores dos Resources.
- Salvo diretamente no `project.godot`.

*AbilitySystemInspectorPlugin*
Automatiza a interface de design:

- Detecta sufixos `_tag` e `_tags` para injetar o seletor.
- Sinaliza tags órfãs (não registradas) com ícones de erro.

---

🛠 *Sistemas Auxiliares*

*AbilitySystemMagnitudeCalculation (MMC)*

Permite cálculos avançados de magnitude:

- Escalar dano por Força
- Usar Inteligência como multiplicador
- Aplicar curvas de progressão
- Fórmulas customizadas via `_calculate_magnitude(spec)`

---

*AbilitySystemTargetData*

Container para dados de targeting:

- `hit_position` (Vector3)
- `target_node` (Object)
- Comunicação de hits e seleções

---

*Signals Reativos*

*ASC:*

- `ability_activated(spec)` — Habilidade iniciada
- `ability_ended(spec, was_cancelled)` — Habilidade finalizada
- `ability_failed(name, reason)` — Falha na ativação
- `effect_applied(spec)` — Efeito aplicado
- `effect_removed(spec)` — Efeito removido
- `tag_event_received(tag, data)` — Evento genérico
- `cooldown_started(tag, duration)` — Início de cooldown
- `cooldown_ended(tag)` — Fim de cooldown
- `attribute_changed(name, old, new)` — Para UI e feedback
- `tag_changed(name, present)` — Para mudanças de estado visual

*AttributeSet:*

- `attribute_pre_change(name, old, new)` — Antes da mudança
- `attribute_changed(name, old, new)` — Após a mudança

Permitem atualizar UI e efeitos em tempo real.

---

*Persistência*

Integração opcional com `SaveServer`:

- Salvar atributos
- Salvar efeitos ativos
- Persistir estados de habilidades

---

🧩 *Organização Conceitual*

Permissão → Inicialização → Execução
Container → AttributeSet → ASC
(o que pode)   (com o quê)   (processa)

*Camadas:*

- *Permissão* → `AbilityContainer` — Regra de acesso (catálogo)
- *Inicialização* → `AttributeSet` — Estado inicial + desbloqueios
- *Execução* → `ASC` — Processamento em runtime

Arquitetura limpa, modular e escalável.
