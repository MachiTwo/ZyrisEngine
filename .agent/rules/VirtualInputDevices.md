---
trigger: always_on
---

# Implementação de Dispositivos de Entrada Virtual

Este documento descreve a implementação do sistema de **Virtual Input Devices** (Dispositivos de Entrada Virtual), que compõe a **v1** da Zyris Engine. Este sistema fornece uma camada de abstração polimórfica de alto desempenho para entradas baseadas em toque, integrando-as como cidadãs de primeira classe ao lado das entradas de teclado, mouse e gamepad.

## 1. Arquitetura Core

O sistema é construído sobre dois novos tipos de `InputEvent` e um gerenciamento de estado de `Input` especializado.

### Novos Tipos de Eventos de Entrada (`core/input/input_event.h`)

- **`InputEventVirtualButton`**: Representa um botão virtual baseado em estado (pressionado/solto). Carrega um `button_index` (0-15 por padrão).
- **`InputEventVirtualMotion`**: Representa a deflexão de um eixo virtual. Carrega um índice de `axis` e um `axis_value` (ponto flutuante, geralmente de -1.0 a 1.0).

Estes eventos herdam de `InputEvent` e implementam métodos polimórficos como `action_match` e `is_match`, permitindo que funcionem perfeitamente com o `InputMap` da engine.

### Extensões do Singleton Input (`core/input/input.h`)

O singleton `Input` foi estendido para rastrear o estado dos "Dispositivos Virtuais":

- **`VirtualDeviceState`**: Uma struct contendo um conjunto de botões virtuais pressionados e um mapa de valores de eixos virtuais.
- **`virtual_device_states`**: Um mapa que permite a coexistência de múltiplos dispositivos virtuais (embora normalmente apenas o dispositivo 0 seja usado).
- **`is_virtual_button_pressed()`** e **`get_virtual_axis_value()`**: Novos métodos de API para consultar o estado virtual diretamente.

O método `_parse_input_event_impl` em `input.cpp` agora atualiza esses estados sempre que um evento virtual é analisado. Ele também dispara `_set_last_input_type(LAST_INPUT_TOUCH)` para suportar a adaptação automática da UI.

## 2. Integração com a Engine

### Integração com Input Map (`core/input/input_map.cpp/h`)

A classe `InputMap` agora define índices padrão para botões e eixos virtuais para garantir consistência em toda a engine:

- **`VirtualButton`**: `DPAD_UP` (12), `DPAD_DOWN` (13), `DPAD_LEFT` (14), `DPAD_RIGHT` (15).
- **`VirtualAxis`**: `LEFT_X` (0), `LEFT_Y` (1), `RIGHT_X` (2), `RIGHT_Y` (3).

As ações padrão da engine (`ui_left`, `ui_right`, `ui_up`, `ui_down`) foram atualizadas em `load_default()` para incluir esses mapeamentos virtuais. Por exemplo, `ui_left` agora é disparado por:

- `InputEventVirtualButton` com índice 14.
- `InputEventVirtualMotion` com eixo 0 e valor -1.0.

Isso garante que a navegação na UI funcione nativamente com controles virtuais.

### Serialização (`core/input/input_event_codec.cpp`)

O `InputEventCodec` foi atualizado para suportar os novos tipos de eventos, garantindo que possam ser salvos nas configurações do projeto, usados em RPCs ou gravados em streams de entrada.

### Suporte ao Editor (`editor/settings/`)

- **`EventListenerLineEdit`**: Atualizado para capturar eventos virtuais durante o modo "listen" no editor de Input Map.
- **`InputEventConfigurationDialog`**: Atualizado para permitir a configuração manual e adição de eventos de Botão Virtual e Movimento Virtual a ações.

## 3. Sistema de Nós de GUI (`scene/gui/`)

O sistema fornece uma hierarquia de nós para facilitar a implementação em jogos.

### `VirtualDevice` (Classe Base)

- Gerencia o rastreio de toque (touch tracking) e a lógica de visibilidade.
- **Modos de Visibilidade**: `ALWAYS` (Sempre) ou `TOUCHSCREEN_ONLY` (Apenas telas sensíveis ao toque - oculta automaticamente se a última entrada foi teclado/mouse/gamepad).
- Faz o proxy de eventos de toque/mouse para uma interface limpa de `_on_touch_down`, `_on_touch_up` e `_on_drag` para as subclasses.

### Nós Especializados

- **`VirtualButton`**: Um botão que mapeia para um `button_index`. Ele imita o `BaseButton`, mas é otimizado para despacho de eventos de toque e virtuais. Suporta texturas, texto e ícones.
- **`VirtualJoystick`**: Fornece uma interface de joystick padrão.
  - **Modos**: `FIXED` (fixo) ou `DYNAMIC` (re-centraliza onde o usuário toca pela primeira vez).
  - **Zonas**: `deadzone_size` (zona morta) e `clamp_zone_size` (zona de limite) configuráveis (porcentagem do raio).
- **`VirtualDPad`**: Um direcional de 4/8 direções que mapeia para 4 índices de botões virtuais distintos.
- **`VirtualTouchPad`**: Uma área invisível ou decorativa para rastreio de gestos (ex: controle de câmera). Gera `InputEventVirtualMotion` baseado no movimento relativo de arraste.
- **`VirtualJoystickDynamic`**: Uma extensão do `VirtualJoystick` especificamente adaptada para comportamento de joystick "toque em qualquer lugar".

## 4. Detecção Automática de Entrada

O sistema introduz o rastreamento de **`LastInputType`** em `Input`:

- `LAST_INPUT_KEYBOARD_MOUSE` (Teclado/Mouse)
- `LAST_INPUT_JOYPAD` (Gamepad)
- `LAST_INPUT_TOUCH` (Toque)

Sempre que um evento de hardware ou virtual é recebido, a engine atualiza este estado e o fornece via `Input::get_singleton()->get_last_input_type()`. Os nós `VirtualDevice` escutam essa mudança para alternar sua visibilidade se configurados como `TOUCHSCREEN_ONLY`.

## 5. Performance e Boas Práticas

- **Eventos Atômicos**: Nós virtuais despacham eventos diretamente para `Input::parse_input_event()`, beneficiando-se da acumulação e limpeza de eventos interna da engine.
- **Polimorfismo**: Ao estender `InputEvent`, os dispositivos virtuais não requerem "hacks" especiais na lógica do jogo; use `Input.is_action_pressed()` como de costume.
- **Localidade de Cache**: O rastreamento de estado em `Input` usa containers otimizados (`RBSet`, `HashMap`) para garantir overhead mínimo durante a consulta de entrada de alta frequência.

## 6. Problemas Conhecidos e Limitações

- **Flickering em Híbridos (Touch + Mouse)**:
  - **Sintoma:** Em dispositivos com suporte simultâneo a mouse e toque (ex: laptops 2-em-1 ou tablets com mouse), desabilitar a opção `emulate_touch_from_mouse` pode causar "flickering" (intermitência) na visibilidade dos controles virtuais.
  - **Causa:** O sistema operacional pode enviar eventos de mouse "fantasmas" (sintéticos) imediatamente após um toque real. O sistema interpreta isso como uma mudança rápida entre `LAST_INPUT_TOUCH` e `LAST_INPUT_KEYBOARD_MOUSE`, fazendo com que os controles (configurados como `TOUCHSCREEN_ONLY`) apareçam e desapareçam rapidamente.
  - **Status:** Investigação pendente para uma solução robusta de "debounce" (filtragem temporal) que rejeite eventos de mouse fantasmas sem bloquear interações legítimas ou causar instabilidade no ciclo de vida inicial da engine.
