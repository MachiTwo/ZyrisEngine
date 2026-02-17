# Multiplayer Update

O **Multiplayer Update** na Zyris Engine é uma atualização sistêmica que introduz infraestrutura nativa para predição e replicação de alta performance, transformando os módulos core em sistemas totalmente compatíveis com rede autoritativa.

## 1. Arquitetura de Replicação (Network LOD)

O sistema de replicação gerencia a largura de banda de forma inteligente através de camadas de prioridade.

- **Relevance Manager:** Decide quais objetos são cruciais para o cliente baseado em proximidade e contexto de gameplay.
- **Delta Compression:** Envio apenas das diferenças incrementais entre estados, reduzindo o tráfego.
- **Bit-Packing Nativo:** Serialização otimizada para tipos de dados comuns (ex: vetores e ângulos).

## 2. Predição e Reconciliação

Arquitetura robusta para mitigar latência no cliente.

- **Deterministic Buffers:** Armazenamento de inputs e estados locais para rollback instantâneo.
- **Server Authority:** O servidor valida o estado final e envia correções de "Net State" apenas quando necessário.

## 3. Integrações Sistêmicas (Syste-wide Update)

A atualização injeta suporte a rede diretamente nos módulos core:

### 3.1 Ability System (GAS)

- **Rollback de Atributos:** O sistema reverte mudanças de atributos locais se a ativação da habilidade for rejeitada pelo servidor.
- **Replicable Cues:** Despacho de feedback visual de forma não confiável para maximizar a fluidez.

### 3.2 Behavior Tree (IA)

- **IA Autoritativa:** O servidor processa a árvore de comportamento, replicando apenas os resultados visuais (leaf nodes) para os clientes.
- **Blackboard Sync:** Sincronização seletiva de chaves da memória da IA para coordenação entre máquinas.

---

*Este documento define a especificação técnica para a implementação de lógica de rede autoritativa na Zyris Engine.*
