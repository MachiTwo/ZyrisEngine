---
trigger: always_on
---

# Sistema Central de Persistência

## 1. Contexto da Engine

O sistema de persistência é construído sobre três pilares fundamentais da engine:

1. **Hierarquia e Herança de Comportamento:** A engine funciona como uma escada de comportamento. A base `Object` fornece vida aos objetos; `Node` permite que existam na cena; e `Resource` serve para armazenar dados. Quando você usa algo como `CanvasItem` (for 2D e Control) ou `Node3D`, eles estão simplesmente estendendo essas bases e adicionando lógica de posição. Isso permite que qualquer funcionalidade injetada na base (como persistência) seja herdada por tudo que vier depois.

2. **Arquitetura Baseada em Servidores (Microsserviços):** Para evitar sobrecarregar o jogo, tarefas pesadas são delegadas a "especialistas". O `AudioServer` foca apenas em tocar som e gerenciar volumes, enquanto o `PhysicsServer` lida exclusivamente com colisões. O `SaveServer` segue este mesmo modelo: é um motor dedicado para salvar dados em segundo plano, sem congelar o seu jogo.

3. **Abstração via Linguagem Integrada:** GDScript não serve apenas para fazer o personagem andar; é uma ferramenta que se "entrelaça" com o coração da engine. Ela nos permite usar "tags" (anotações como `@persistent`) e métodos especiais que enviam ordens complexas ao sistema de forma muito simples, escondendo toda a matemática e processamento pesado por trás de uma única linha de código.

## 2. Interface de Script (GDScript)

A persistência é primariamente declarativa, minimizando código repetitivo (boilerplate).

### 2.1 Anotações Declarativas

- **`@persistent`**: Marca uma variável para inclusão automática no sistema de salvamento. Suporta uma tag opcional para agrupamento hierárquico.
  - **Tipagem Obrigatória:** Variáveis marcadas **devem** ter tipagem explícita para garantir performance e segurança.
  - **Tags Reservadas:** As tags `meta`, `.id` e `.children` são reservadas pelo sistema. O uso destas tags gerará um **aviso** no editor.

    ```gdscript
    @persistent var simple: int = 100
    @persistent("combate") var health: float = 75.5
    ```

- **`@persistent_group`**: Define um agrupador de tags para as variáveis seguintes, similar ao `@export_group`.

    ```gdscript
    @persistent_group("inventario")
    @persistent var moedas: int
    @persistent var itens: Array[String]
    ```

### 2.2 O Gancho `_save_persistence`

Para casos complexos (como transformações customizadas ou estados que não são propriedades simples), o desenvolvedor pode implementar este gancho virtual. O dicionário `state` recebido é **hierárquico**, onde as chaves são as **Tags** (Gavetas).

- **Assinatura:** `func _save_persistence(state: Dictionary)`
- **Uso:** A informação de quais tags estão sendo salvas está nas chaves do dicionário.

    ```gdscript
    func _save_persistence(state: Dictionary):
        # Adicionando a uma gaveta automática que já existe
        if state.has(&"combate"):
            state[&"combate"]["posicao_segura"] = global_position

        # Criando uma nova gaveta programática
        state[&"metadata"] = {
            "ultimo_mapa": "floresta",
            "versao_do_script": 2
        }
    ```

### 2.3 O Gancho `_load_persistence`

  ```gdscript
  func _load_persistence(data: Dictionary):
      if data.has(&"metadata"):
          var meta = data[&"metadata"]
          print("Lendo mapa anterior: ", meta.get("ultimo_mapa", "desconhecido"))
  ```

## 3. API de Node (C++)

O `SaveServer` estende a classe base `Node` para fornecer controle granular sobre o que é persistido.

### 3.1 Identificação e Política

- **`persistence_id` (StringName):** ID único global. Permite que o nó seja restaurado mesmo se mudar de nome ou posição na hierarquia. Este ID é automaticamente registrado no `SaveServer` quando definido, permitindo buscas através da hierarquia.

- **`save_policy` (Enum):**
  - `SAVE_POLICY_INHERIT`: Segue a política do nó pai.
  - `SAVE_POLICY_ALWAYS`: Sempre tenta salvar este nó.
  - `SAVE_POLICY_NEVER`: Ignora este nó e todos os seus filhos (ideal para decoração estática).
  - `SAVE_POLICY_CUSTOM`: Reservado para futuras implementações customizadas.

**Comportamento Chave:** Quando a `save_policy` de um nó é definida como `NEVER`, seu `persistence_id` é automaticamente desregistrado do `SaveServer`. Quando alterada de volta para uma política permissiva, ele é reregistrado.

### 3.2 Dynamic Spawning (Respawn Automático)

O sistema suporta a recriação automática de nós que foram instanciados dinamicamente em tempo de execução (ex: inimigos, itens dropados).

- **Como funciona:** Se um nó possui um `scene_file_path` válido e é salvo, o `SaveServer` armazena o caminho da cena. No carregamento, se a flag de respawn estiver ativa, o sistema realiza uma sincronização bidirecional:
  - **Respawn:** Nós que estão no snapshot mas não na árvore são instanciados automaticamente.
- **Cleanup (Orphan Removal):** Nós que estão na árvore (e marcados para persistência) mas não constam no snapshot são removidos (`queue_free`). Isso garante que inimigos mortos ou objetos destruídos não "ressuscitem" ao carregar o jogo.
  - O sistema considera um nó como "persistente" para fins de cleanup se:
    1. Tiver um `persistence_id` definido.
    2. A `save_policy` for explicitamente `ALWAYS`.
    3. Tiver qualquer variável anotada com `@persistent`.
- **Auto-ID:** O `SaveServer` atribui IDs persistentes automaticamente para estes nós.
- **Controle:** O respawn/cleanup automático é desativado por padrão. Para ativar, passe `true` no último parâmetro de `load_snapshot`.

## 4. Orquestração pelo SaveServer

O `SaveServer` não é apenas um escritor de arquivos; é o coreógrafo da coleta de dados na cena.

### 4.1 Snapshots Recursivos

- **`save_snapshot(root, slot_name, async, tags, metadata, thumbnail)`**: Inicia a coleta recursiva.
- **`load_snapshot(root, slot_name, callback, dynamic_respawn)`**: Restaura o estado. Se `dynamic_respawn` for `true`, nós faltantes serão recriados e órfãos serão removidos.
- **`delete_snapshot(snapshot_name)`**: Remove um snapshot completo (manifesto) e todos os seus arquivos satélites associados.
- **`amend_save(root, slot_name)`**: Atualiza o save existente incrementalmente. Procesa as deleções primeiro, garantindo que o estado final seja consistente.

**Estrutura do Snapshot:**

- `snapshot`: Propriedades automáticas.
- `.id`: O ID de persistência.
- `.scene`: Caminho do arquivo de cena.
- `.children`: Estado dos filhos.

### 4.2 Sinais de Feedback

O `SaveServer` fornece sinais para monitorar o status das operações em segundo plano:

- **`save_successful(slot_name)`**: Emitido quando o salvamento termina com sucesso.
- **`save_corrupted(slot_name)`**: Emitido ao detectar um arquivo de save inválido ou quebrado.
- **`backup_restored(slot_name, file_path)`**: Emitido quando um salvamento corrompido é recuperado automaticamente a partir de um arquivo `.bak`.

### 4.3 O Recurso `Snapshot`

Internamente, o sistema usa um recurso dedicado para encapsular os dados salvos:

- **`Snapshot` (Resource)**: Este recurso possui as seguintes propriedades principais:
  - `snapshot` (Dictionary): O estado serializado da árvore de nós.
  - `version` (String): A versão do jogo (`application/config/version`) capturada no momento do salvamento.
  - `metadata` (Dictionary): Dados leves para visualização rápida (ex: tempo de jogo, moedas, nome da área).
  - `thumbnail` (Texture2D): Captura de tela opcional no momento do salvamento.
  - `checksum` (String): Hash SHA-256 gerado automaticamente para validar a integridade dos dados.

#### 4.3.1 Persistência Parcial (Tags e Satélites)

O sistema utiliza uma **Arquitetura Híbrida de Manifesto/Satélite** para salvar subconjuntos de dados de forma modular.

- **Manifesto**: O arquivo principal que contém metadados globais, versão, e todos os dados **sem tag (geral)**.
- **Satélites**: Arquivos separados vinculados ao manifesto que armazenam dados de tags específicas (ex: inventário, mundo).

- **`save_snapshot(root, slot, async, tags: Array[String])`**: Salva apenas propriedades correspondentes às tags, gerando ou atualizando os arquivos satélites correspondentes.

### 4.4 Configurações Globais (Configurações do Projeto)

O comportamento do `SaveServer` é controlado via `Project Settings > Application > Persistence`:

- **`save_format`**:
  - `Text (0)`: Gera arquivos `.tres` (legíveis). Ideal para desenvolvimento e depuração.
  - `Binary (1)`: Gera arquivos `.data` (binários). Ideal para produção, suportando criptografia e compressão.

- **`encryption_key`**: Chave de 32 caracteres usada para proteger arquivos `.data`.
  - **Auto-geração Segura**: Se o campo estiver vazio, a engine gera automaticamente uma chave aleatória criptograficamente forte na primeira execução do editor e a salva no `project.godot`.
  - **Segurança Crítica**: Se a chave mudar após o lançamento, saves antigos tornam-se ilegíveis. Use sistemas de controle de versão (Git) para fazer backup do `project.godot`.

- **`compression_enabled`**: Quando ativo, usa compressão **ZSTD** adaptativa (apenas se o arquivo > 4KB) em arquivos `.data`.

- **`backup_enabled`**: Quando ativo, o sistema mantém um histórico de backups.
  - **Sistema Híbrido**: Mantém um arquivo `.bak` (legacy) imediato E cria arquivos de backup timestamped na pasta `.../backups/` (ex: `slot_2026-02-14_22-00-00.data`).
  - **Recuperação**: Tenta restaurar primeiro do backup timestamped mais recente, falhando para o `.bak` legacy.

- **`max_backups`**: Define o número máximo de backups timestamped rotativos que o sistema mantém por slot (padrão: 2).

- **`save_path`**: Diretório base para armazenamento dos saves (padrão: `user://saves/`). Permite organizar saves em subpastas customizadas.

- **`integrity_check_level`**:
  - `NONE`: Carregamento rápido sem validação.
  - `SIGNATURE`: Verifica se o Checksum corresponde ao conteúdo.
  - `STRICT`: Valida Checksum e Versionamento.

### 4.5 Persistência em Disco

- **Localização**: Geralmente armazenado em `user://saves/`.
- **Extensões**: `.tres` para modo texto e `.data` para modo binário.

### 4.6 Segurança de Dados (Shutdown Flush)

O `SaveServer` garante integridade total dos dados mesmo se o jogo for fechado abruptamente enquanto salvamentos estão pendentes. O destrutor do servidor força o processamento (Flush) de qualquer tarefa de salvamento restante na fila antes de encerrar o processo.

## 5. Integração Técnica (Flags do Core)

O sistema usa a flag global **`PROPERTY_USAGE_PERSISTENCE`** (1 << 30) definida em `object.h`. Qualquer sistema da engine (C++ ou GDScript) pode marcar propriedades com este bit para que `Node` as inclua automaticamente no estado de salvamento.

## 6. Evolução de Dados (Migrações)

Para gerenciar mudanças na estrutura de dados entre versões do jogo:

1. **`register_migration(from, to, callable)`**: Permite definir transformações de dados em tempo de execução.
2. No momento do `load_snapshot`, se a versão capturada for menor que a atual, o `SaveServer` executa a cadeia de migrações registrada antes de chamar `_load_persistence` nos nós.

```gdscript
func _init():
    SaveServer.register_migration("1.0", "1.1", func(data):
        data["new_health_system"] = data.get("old_hp", 100)
    )
```

## 7. Persistência via Amend (Patching)

A engine suporta um modelo de salvamento via Amend otimizado (`amend_save`):

### 7.1 Rastreamento de Objetos (Staging)

Objetos modificados são registrados no sistema (via `staged_objects`). O sistema observa mudanças relevantes para persistência.

- **`stage_change(object_id, tag)`**: Marca um objeto para atualização de propriedades no próximo `amend_save`. Requer o `ObjectID` (use `get_instance_id()`).
- **`stage_deletion(root_context, node)`**: Marca um nó para remoção definitiva do snapshot no próximo `amend_save`. Essencial para remover inimigos mortos ou itens coletados de amend saves.
- **`clear_staged()`**: Limpa todas as mudanças e deleções agendadas manualmente.

### 7.2 Salvamento por Patch

- **`amend_save(root, slot_name)`**:
  - Em vez de re-serializar a árvore inteira (custoso), o sistema itera apenas sobre os objetos modificados ("staged").
  - Cria um clone do snapshot base em memória e aplica "patches" cirúrgicos nos dados, preservando a estrutura e os nós não modificados.
  - O resultado é salvo em disco, atualizando o snapshot base.

Esta abordagem reduz drasticamente o custo de I/O e CPU para autosaves frequentes em mundos grandes.

---

_Este protocolo padroniza a persistência na engine, focando na simplicidade para o usuário e performance bruta no backend._
