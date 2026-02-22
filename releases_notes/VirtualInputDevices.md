# Virtual Input Devices - Sistema de Controle Touch

## Visão Geral

Sistema completo de entrada virtual para dispositivos móveis e híbridos, fornecendo controles de toque nativos que se integram perfeitamente ao sistema de input existente.

## Recursos Principais

### 1. Controles Virtuais Nativos ou Emulados

Nós de UI otimizados para controle por toque:

- **VirtualButton**: Botão configurável com suporte a texturas, ícones e mapeamento de index
- **VirtualJoystick**: Controle analógico com modos Fixo e Dinâmico
- **VirtualJoystickDynamic**: Extensão que permite o spawn do joystick na posição do toque
- **VirtualDPad**: Direcional com suporte a 4 e 8 direções
- **VirtualTouchPad**: Área de gestos com rastro visual (Trace/Circle) e suporte a eixos virtuais

### 2. Detecção Automática de Dispositivo

Sistema inteligente que adapta a UI dinamicamente:

- **LastInputType API**: Detecta automaticamente o método de entrada ativo
- **Modos de Visibilidade**: `ALWAYS`, `TOUCHSCREEN_ONLY`
- **Transição Suave**: Controles aparecem/ocultam automaticamente baseado no dispositivo

### 3. Integração com InputMap

Mapeamento direto com as ações do jogo:

```gdscript
# Configuração no InputMap
action "ui_left" mapeado para:
- VirtualButton (DPAD_LEFT)
- VirtualJoystick (eixo X negativo)
- Teclado (seta esquerda)
```

### 4. Performance Otimizada

Arquitetura desenvolvida para alta performance:

- **Arquitetura Híbrida**: Escolha entre Modo Nativo (eventos reais) ou Emulado (ações simples)
- **Eventos Atômicos**: Despacho direto para o processador de input da engine
- **Polimorfismo**: Funciona com `Input.is_action_pressed()` nativamente
- **Cache Local**: Estado otimizado para consultas de alta frequência e baixa latência

## Configuração

### Modos de Visibilidade

- **ALWAYS**: Controles sempre visíveis
- **TOUCHSCREEN_ONLY**: Ocultam-se automaticamente se o último input for Teclado/Mouse ou Gamepad

### Modos de Entrada (Input Mode)

- **NATIVE**: Emite eventos `InputEventVirtual*` tratados como dispositivos reais
- **EMULATED**: Emula ações diretamente do `InputMap` (estilo TouchScreenButton)

### Configurações do Joystick

- **Deadzone**: Área central insensível (configurável em %)
- **Clamp Zone**: Limite máximo de movimento (configurável em %)
- **Modos**: FIXED (fixo) ou DYNAMIC (toque em qualquer lugar)

## API de Uso

### Configuração Básica

```gdscript
# Criar botão virtual
var jump_button = VirtualButton.new()
jump_button.text = "Jump"
jump_button.action = "jump"
jump_button.modulate = Color.WHITE

# Criar joystick
var move_joystick = VirtualJoystick.new()
move_joystick.deadzone_size = 0.2
move_joystick.clamp_zone_size = 0.8
```

### Detecção de Dispositivo

```gdscript
func _ready():
    # Verificar tipo de entrada atual
    match Input.get_last_input_type():
        Input.LAST_INPUT_TOUCH:
            print("Dispositivo touch detectado")
        Input.LAST_INPUT_KEYBOARD_MOUSE:
            print("Teclado/mouse detectado")
        Input.LAST_INPUT_JOYPAD:
            print("Gamepad detectado")
```

## Casos de Uso

### Jogos Móveis

- Controles touch-first com fallback para gamepads
- UI adaptativa que se ajusta ao dispositivo

### Jogos Híbridos (PC/Mobile)

- Mesmos controles funcionam em todas as plataformas
- Detecção automática do método de entrada
- Transição suave entre diferentes esquemas de controle

### Aplicações Multiplataforma

- Código único funciona em desktop, mobile e web
- Sistema de input unificado e consistente

## Integração com Editor

### Configuração Visual

- **Inspector**: Propriedades customizadas para todos os nós virtuais
- **Input Map Editor**: Suporte direto para eventos virtuais
- **Preview**: Visualização em tempo real dos controles

### Ferramentas de Debug

- **Visual Feedback**: Indicadores visuais de estado dos controles
- **Event Monitor**: Visualização de eventos de entrada em tempo real
- **Performance Metrics**: Análise de performance dos controles virtuais

---

_Sistema completo de entrada touch-ready para jogos modernos e multiplataforma._
