# Save Server - Sistema de Persistência de Dados

## Visão Geral

Sistema centralizado de persistência de dados para jogos, oferecendo salvamento automático, recuperação de corrupção e operações assíncronas de alta performance.

## Recursos Principais

### 1. Persistência Declarativa

Anotações simples para automação completa do sistema de salvamento:

```gdscript
@persistent var player_health: float = 100.0
@persistent var inventory: Array[Item] = []
@persistent("combat") var experience: int = 0
```

**Recursos:**

- **@persistent**: Marca variáveis para salvamento automático
- **@persistent_group**: Agrupa variáveis por categoria
- **Tipagem obrigatória**: Garante performance e segurança
- **Tags hierárquicas**: Organização lógica dos dados

### 2. API de Alto Nível

Controle granular sobre o processo de persistência:

```gdscript
# Salvamento completo
SaveServer.save_snapshot(get_tree().current_scene, "slot_1", true)

# Carregamento com respawn automático
SaveServer.load_snapshot(get_tree().current_scene, "slot_1", _on_load_complete, true)

# Salvamento incremental (amend)
SaveServer.amend_save(get_tree().current_scene, "slot_1")
```

### 3. Dynamic Spawning & Cleanup

Recriação automática de objetos dinâmicos e limpeza de órfãos:

- **Respawn automático**: Inimigos e itens instanciados em runtime são recriados
- **Cleanup inteligente**: Objetos destruídos não reaparecem no carregamento
- **ID persistente**: `persistence_id` permite rastreamento entre sessões

### 4. Arquitetura Híbrida Manifesto/Satélite

Sistema modular para salvar subconjuntos de dados:

- **Manifesto**: Dados globais e metadados
- **Satélites**: Arquivos separados para tags específicas (inventário, mundo, etc.)
- **Persistência parcial**: Salve apenas o que mudou

### 5. Segurança e Integridade

Proteção completa contra corrupção de dados:

- **Checksum SHA-256**: Validação automática de integridade
- **Sistema de backups**: Recuperação automática de saves corrompidos
- **Flush seguro**: Garante escrita completa mesmo em crashes
- **Criptografia AES-256**: Proteção de dados em modo binário

### 6. Performance Otimizada

Operações assíncronas e algoritmos eficientes:

- **Threaded I/O**: Salvamento em segundo plano sem bloquear o jogo
- **Compressão ZSTD**: Redução automática do tamanho de arquivos
- **Amend saves**: Atualizações incrementais (até 95% mais rápidas)
- **Staging system**: Rastreamento eficiente de mudanças

## Configuração

### Formatos de Saída

- **Texto (.tres)**: Legível, ideal para desenvolvimento
- **Binário (.data)**: Compacto, com criptografia e compressão

### Configurações Globais

Acessível via `Project Settings > Application > Persistence`:

- **Chave de criptografia**: Auto-gerada ou customizada
- **Compression**: Ativável para arquivos > 4KB
- **Backups**: Sistema rotativo com histórico timestamped
- **Validação**: Níveis de verificação de integridade

## Migração de Dados

Sistema robusto para evolução de estrutura de dados:

```gdscript
func _init():
    SaveServer.register_migration("1.0", "1.1", func(data):
        data["new_health_system"] = data.get("old_hp", 100)
    )
```

## Sinais de Feedback

Monitoramento completo das operações:

- `save_successful(slot_name)`: Salvamento concluído
- `save_corrupted(slot_name)`: Detectada corrupção
- `backup_restored(slot_name, file_path)`: Backup restaurado

## Casos de Uso

### RPGs com Inventário Complexo

- Persistência separada para inventário e progresso
- Salvamento rápido durante gameplay
- Recuperação automática de corrupção

### Jogos de Mundo Aberto

- Dynamic spawning de NPCs e itens
- Cleanup de objetos destruídos
- Amend saves para autosaves frequentes

### Jogos Multiplayer

- Validação de integridade de dados
- Sincronização de estado entre sessões
- Backup automático de progresso

---

_Este sistema oferece persistência enterprise-ready para jogos de qualquer escala._
