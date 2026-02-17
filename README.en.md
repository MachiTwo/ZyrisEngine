# Zyris Engine

[![Zyris Version](https://img.shields.io/badge/Zyris-1.3--AbilitySystem-blue.svg)](https://github.com/godotengine/godot)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE.txt)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](README.en.md) | [**Português**](README.md)

Zyris is an open-source engine oriented towards systems and production architecture.

Designed to offer structural control, predictability, and scalability,
Zyris positions itself in the same ecosystem as engines like Godot, Unreal Engine, and Unity.

---

## Technological Origin

Zyris is a fork of the Godot Engine.

This means that:

- Projects made in Godot can be opened in Zyris
- Your knowledge in GDScript remains valid
- The workflow remains familiar

Zyris maintains compatibility with the Godot base,
but has its own governance of commits and roadmap.

Upstream updates are not applied automatically.
Every change is analyzed, filtered, and integrated in a controlled manner.

---

## Philosophy

Zyris is an **intentionally opinionated** engine. We believe that absolute freedom without direction leads to chaotic code, structural rework, and inconsistent systems over time.

Therefore, Zyris establishes **clear conventions**, recommended architectures, and integrated systems as standard. These conventions do not remove freedom—they reduce the need to reinvent fundamental solutions.

You can still develop however you prefer, but by default, Zyris offers:

- **Predictable structures**
- **Integrated persistent systems**
- **Deterministic state management**
- **Consistent architectural base** for AI and gameplay

Our goal is not to limit creativity, but to **eliminate unnecessary complexity**. Zyris evolves as a production-oriented engine, focused on systemic coherence, project longevity, and technical maturity.

### Evolution Principles

Every system introduced in Zyris must integrate natively into the core, adhere to deterministic principles, and prioritize production stability.

The engine evolves in a **curated** manner. External updates are not adopted automatically; every change is evaluated through the lens of structural coherence and technical maturity. Our focus is to build a solid foundation for scalable projects prepared for longevity and continuous evolution.

All credit for the base engine goes to the [official Godot Engine repository](https://github.com/godotengine/godot) and its incredible community of contributors.

## Official Communication

Zyris development and official announcements are communicated through:

🎥 [Machi](https://www.youtube.com/@machiatodev)
🎥 [Alen](https://www.youtube.com/@yatsuragames)

These channels are responsible for:

- Engine updates
- Technical explanations
- System demonstrations
- Official tutorials

## Roadmap

Zyris is implementing a comprehensive set of systems. Below is our development roadmap:

### Implemented

- [x] **Virtual Input Devices** - Cross-platform Input Abstraction Layer

    A complete virtual input system integrated directly into the engine core, providing fluid touch controls for mobile and hybrid devices.

    **Interface Nodes:**

- `VirtualButton` - Configurable touch button with visual feedback and action mapping
- `VirtualJoystick` - Analog control with customizable deadzones and sensitivity
- `VirtualDPad` - Directional pad with support for 4 and 8 directions
- `VirtualTouchPad` - Multi-touch gesture area for camera and viewport control

    **Key Features:**

- **Input Device Tracking** - `LastInputType` API automatically detects and tracks the active input method (Touch, Keyboard/Mouse, Gamepad)
- **Dynamic UI Adaptation** - Virtual controls automatically appear/hide based on the detected device
- **Visibility Modes** - `ALWAYS`, `TOUCHSCREEN_ONLY` for flexible UI behavior
- **Action Mapping** - Direct integration with Godot's InputMap system
- **Polymorphic Input System** - The `InputEvent` base class now uses virtual methods for action matching and event comparison, allowing custom virtual events to integrate seamlessly into the engine core as first-class citizens.
- **Editor Integration** - Full inspector support with visual configuration
- **Performance Optimized** - Minimal overhead with efficient event handling

- [x] **Save Server** - Persistence and Serialization Orchestrator

     A robust, high-performance persistence system integrated into the engine core, allowing for secure and asynchronous saving and loading of complex states.

     **Key Features:**

- **Declarative Protocol (@persistent)** - Full automation via GDScript annotations and C++ flags. Supports strong typing and hierarchical organization via `@persistent_group`.
- **Threaded Architecture** - I/O operations, ZSTD compression, and AES-256 encryption run on dedicated threads, ensuring fluid autosaves without stuttering.
- **Hybrid Architecture (Manifest/Satellites)** - Supports partial persistence through tags, allowing sub-sets of data (e.g., inventory, world) to be saved in modular satellite files.
- **Dynamic Spawning & Cleanup** - Automatic reconstruction of nodes instantiated at runtime and removal of orphans (dead enemies) to ensure total parity between the scene and the snapshot.
- **Incremental System (Amend Save)** - Inspired by Git, it tracks modified objects and applies surgical "patches" to the snapshot, reducing disk writes by up to 95%.
- **Global ID Registry (persistence_id)** - Hierarchy abstraction that detaches data from NodePaths, preventing save breakage if nodes are renamed or moved.
- **Data Evolution (Migrations)** - Versioning system that allows registering data transformations (`register_migration`) to maintain compatibility with legacy saves.
- **Safety and Integrity** - Rotative backup system, SHA-256 checksum validation, and *Flush* mechanism on shutdown to prevent data corruption.

### In Development

- [ ] **Ability System** - Data-Oriented Gameplay Framework

    A high-performance native implementation designed to scale from simple mechanics to complex RPG combat systems.

    **Core Systems:**
  - `AbilitySystem` - Global singleton managing the Gameplay Tag registry and project settings.
  - `AbilitySystemComponent` - The central processor attached to entities (Player, Enemies). Orchestrates abilities, effects, attributes, and cues.
  - `AbilitySystemAbilityContainer` - Archetype resource defining the base configuration of an entity (Abilities, Initial Attributes, Innate Effects, Cues).
  - `AbilitySystemAttributeSet` - Container that manages collections of AbilitySystemAttribute Resources with limit validation and runtime values.
  - `AbilitySystemAttribute` - Defines a single attribute with `BaseValue` and `CurrentValue` (with buffs/debuffs).
  - `AbilitySystemAbility` - Modular ability logic (Jump, Shoot, Magic) with **Costs & Cooldowns** and lifecycle `can_activate → activate → end`.
  - `AbilitySystemEffect` - Attribute alteration rules (Damage, Healing, Buffs/Debuffs) with duration policies (Instant, Duration, Infinite).
  - `AbilitySystemTask` - Asynchronous actions for complex abilities (e.g., waiting for input, moving to position, spawning projectiles).
  - `AbilitySystemTag` - Hierarchical named tag (e.g., `state.buff.speed`) for ability filtering and validation.
  - `AbilitySystemCue` - Visual/audio feedback (VFX, SFX, screen shake) triggered by effects or abilities.

    **Runtime Systems (Specs):**

    *Specs* are runtime instances that wrap a definition Resource with the execution context needed. They allow abilities, effects, and cues to access dynamic data (who attacked, what magnitude was calculated, remaining duration) without polluting the shared Resource.
  - `AbilitySystemAbilitySpec` - Runtime instance of a granted `AbilitySystemAbility`. Stores level and active state.
  - `AbilitySystemEffectSpec` - Runtime instance of an applied `AbilitySystemEffect`. Stores remaining duration, dynamic magnitudes, and references to source/target ASC.
  - `AbilitySystemCueSpec` - Execution context for an `AbilitySystemCue`. Carries source/target ASC, the triggering `AbilitySystemEffectSpec`, and calculated magnitude.

    **Auxiliary Systems:**
  - `AbilitySystemMagnitudeCalculation` - Base class for custom magnitude calculations (MMC). Enables scaling damage by attributes (Strength, Intelligence) or curves.
  - `AbilitySystemTagContainer` - Optimized tag container for runtime queries and modifications (add, remove, has, get_all).
  - `AbilitySystemTargetData` - Target validation and filtering pipeline. Supports Line Trace, Sphere Overlap, Box and custom shapes.

### In Planning

- [ ] **Behavior Tree** - Modular and Reactive AI

    A robust node-based AI implementation, focused on creating complex behaviors through simple and reusable visual logic.

    **Architecture:**
  - `BTAgent`: The orchestrator attached to the NPC, managing memory (`Blackboard`) and tree ticking.
  - **Composites** - Flow nodes (`Sequence`, `Selector`) that define decision making.
  - **Decorators** - Conditional logic and modifiers (`Cooldown`, `Loop`, `Blackboard Check`) that wrap other nodes.
  - **Leafs** - The actual work units (`MoveTo`, `Wait`, `ActivateAbility`).

    **Editor Integration (EditorPlugin):**
    Behavior Trees use a **Dedicated Visual Editor** that operates in exclusive mode.
  - **Visual Board** - Infinite canvas to organize complex AI hierarchies with visual clarity, supporting drag-and-drop of nodes.
  - **Live Debugging** - Visual tracking of execution flow in real-time (nodes light up while active).
  - **Sub-Trees** - Capability to create nodes that encapsulate entire trees, allowing massive reuse (e.g., a "Combat" tree used inside various different AIs).

    **Differentials:**
  - **Native & GAS** - Designed to trigger `GameplayAbilities` directly via action nodes.
  - **Performance** - Tree traversal in pure C++, eliminating script bottlenecks in dense AIs.

    **Use Cases:**
- **RPG NPCs** - Complex combat, dialogue, and exploration behaviors.
- **FPS Enemies** - Squad tactics, cover, and patrolling.
- **Strategy AI** - Macro and micro-level decision making.

- [ ] **Inventory System** - Item and Transaction Management
  - **InventoryServer** - Authoritative singleton that validates all item movements, preventing inconsistent states or cheating.
  - **Equipment Bridge (GAS)** - When equipping an item, the system automatically injects/removes *Gameplay Abilities* and *Effects* on the bearer's component, without manual scripts.
  - **Smart UI Nodes** - Set of `Control` nodes (`InventoryGrid`, `EquipmentSlot`) that manage drag-and-drop and automatic synchronization with the server.
  - **Loot Tables** - Procedural generation integrated with the GAS luck and tag system.

    **Architecture:**
- **Server-Side Authority** - All transactions validated on the server.
- **Client Prediction** - Responsive interface with subsequent validation.
- **Data-Driven** - Items defined as Resources with properties and behaviors.

    **Technical Features:**
- **Stack Management** - Automatic stack and quantity management.
- **Slot Validation** - Equipment rule validation (class, level, etc.).
- **Transaction Rollback** - Automatic reversal of invalid transactions.
- **Network Sync** - Efficient synchronization of inventory changes.

    **Use Cases:**
- **RPGs** - Complex inventories with equipment and consumables.
- **Survival** - Crafting and limited resource management.
- **MMOs** - Virtual economy and cheating prevention.

- [ ] **Camera System (vCam)** - Cinematographic Arbitration
  - **Virtual Camera Resources** - Camera profiles defined as Resources, containing FOV, Follow Target, LookAt, and constraints.
  - **Priority Arbiter** - The `vCamServer` evaluates in real-time which vCam has the highest priority to take control of the main viewport with configurable blends.
  - **Procedural Shake** - Trauma system based on Perlin noise, where shake profiles are editable and cumulative Resources.
  - **Viewport Integration** - Uses **Custom 3D Gizmos** to draw and visually adjust transition zones, tracking paths, and influence volumes directly in the scene.

    **Architecture:**
- **vCamServer** - Central singleton that manages all virtual cameras in the Zyris system.
- **Priority System** - Dynamic priority system with smooth blends.
- **Resource-Based** - Configurations saved as Resources for reusability.
- **Non-Intrusive** - Interacts with Godot's existing camera system without modifying it.

    **Technical Features:**
- **Dynamic Blending** - Smooth transitions between cameras with customizable curves.
- **Dead Zones** - Dead zones to prevent excessive movement.
- **Aim Tracking** - Automatic target following with prediction.
- **Cinematic Sequences** - Support for predefined cinematic sequences.

    **Use Cases:**
- **Action Games** - Dynamic cameras that respond to gameplay.
- **Cinematics** - Complex and dramatic camera sequences.
- **RPGs** - Over-the-shoulder cameras with contextual behavior.

- [ ] **Multiplayer Update** - Next-Gen Replication and Prediction

     A systemic update that injects performance-driven prediction and replication logic into all core engine modules.

     **Key Features:**

- **Layered Replication (Network LOD)** - Intelligent bandwidth management based on priority and proximity.
- **Prediction and Reconciliation** - Native framework to mitigate latency with authoritative rollback support.
- **Authoritative AI (Behavior Tree)** - Behavior tree processing on the server with state synchronization.
- **Multiplayer GAS** - Ability System integration with authoritative networking and attribute prediction.
- **Network Debugger** - Visual monitoring of bandwidth and latency per object directly in the Editor.

     **API and Configuration:**
- **Network Settings** - Server Tick Rate, Client Prediction, Relevance Distance, Bandwidth Limit via Project Settings.
- **Replication Priorities** - CRITICAL, HIGH, LOW, NONE for granular bandwidth control.
- **Movement Prediction** - Input buffers, state buffers, and smooth reconciliation.
- **Authoritative Validation** - Server-side action verification with result broadcasting.

     **Use Cases:**
- **FPS Multiplayer** - Movement prediction, hit registration, weapon sync.
- **RPG Multiplayer** - Ability system, inventory sync, persistent world state.
- **Strategy Games** - AI authority, unit commands, authoritative resource management.

     **Performance and Optimization:**
- **Latency Compensation** - Automatic latency compensation.
- **Packet Loss Recovery** - Recovery from lost packets.
- **Bandwidth Optimization** - Efficient bandwidth usage with delta compression and native bit-packing.
- **Debug Tools** - Network Visualizer, Latency Graph, State Inspector.

- [ ] **Cloud Providers** - Cloud Services and Platform Abstraction

     A unified module for integration with external services (Steam, EOS, Mobile), consolidating remote saving, accounts, and discovery sessions.

     **Key Features:**

- **Provider Abstraction (CloudProvider)** - Polymorphic interface that unifies Steam, Epic Online Services, and PlayFab under a single API.
- **Save Synchronization (Cloud Save)** - Asynchronous upload/download of snapshots fully integrated into the `SaveServer`.
- **Native Lobbies and Discovery** - Room management and matchmaking integrated directly with active provider APIs.
- **Single Sign-On (SSO)** - Transparent authentication and persistent profile management.
- **SDK Management (EditorSDKManager)** - Dedicated Editor interface for configuration and validation of third-party SDKs (Steamworks, GDK, etc).

     **Provider Architecture:**
- **SteamProvider** - Complete integration with Steam Cloud, Auth, and Networking.
- **EOSProvider** - Support for Epic Online Services (Save, Friends, Sessions).
- **CustomHTTPProvider** - Generic backend via REST for proprietary infrastructure.
- **GooglePlayProvider** - Integration with Google Play Games Services.

     **Technical Features:**
- **Conflict Resolution** - Based on timestamps and SHA-256 checksums.
- **Incremental Synchronization** - Only changes are transferred.
- **Relay Networking** - Packet transport without Port Forwarding.
- **Lobby Management** - Creation, discovery, and customizable metadata.

     **Use Cases:**
- **Multiplayer Games** - Matchmaking, cross-platform, cloud persistence.
- **Single-Player Games with Cloud** - Universal progress, automatic backup, achievements.
- **Enterprise Applications** - Custom backend, enterprise authentication, analytics.

- [ ] **Audio System Improvements**

    Zyris expands and overhauls its native audio system, acting directly on existing components (`AudioServer`, `AudioStream`, `AudioStreamPlayer`, `AudioBus`, `AudioEffect`), without introducing parallel servers or external naming conventions.

    **Scope of Improvements:**
  - **Overhaul of AudioStreamPlayers and Listeners**
    - Players now support high-level sound logic.
    - Direct integration with events, states, and gameplay parameters.
    - More expressive behavior without abandoning Godot's model.

  - **Expansion of existing Audio Resources**
    - `AudioStream` with metadata and dynamic parameters.
    - Support for variations, layers, and contextual resolution.

  - **Introduction of new Audio Resources**
    - New types strictly follow Godot's standard inheritance and existing naming.
    - Direct integration with the AudioServer.

  - **Advanced Mixing and DSP**
    - DSP per stream, bus, or global (Equalization, filters, compression, and temporal effects).
    - Dynamic control via code or events.

  - **Sound Events**
    - Sounds are no longer triggered just by files; execution based on intent and context.
    - Dynamic audio resolution at runtime.

  - **Gameplay Integration**
    - Direct integration with GAS, Behavior Trees, LSS, and Save Server.
    - Game states influence mixing, execution, and sound transitions.

    These improvements elevate Zyris audio to a modern, reactive, and scalable level, maintaining conceptual compatibility with the base engine.

- [ ] **AOT Export System** - SDK-based Architecture

    The AOT Export System is one of the central pillars of the Zyris vision, designed to deliver high-performance native execution without altering Godot's development flow.

    AOT does not replace the VM during development. It acts exclusively at export time, removing the VM only from the final built game and generating native binaries targeted at the destination hardware.

    **Key Benefits:**
  - **Native Performance** - By removing the Virtual Machine (VM) from the final executable, the game runs directly on hardware, eliminating script interpretation overhead.
  - **Security (Anti-Hack)** - Compilation to native machine code makes reverse engineering exponentially harder compared to script or bytecode decompilation, protecting intellectual property.
  - **Export-Only** - The development flow remains agile with Hot-Reloading, while AOT acts only in the final build pipeline.

    To ensure sustainability, scalability, and ease of contribution, AOT is conceived as an external SDK, integrated into the engine's export pipeline.

    **Model Guidelines:**
  - Execution only on export (does not participate in editor or debug)
  - Editor remains lightweight and decoupled from compiler cost
  - Contributions facilitated via dedicated repository
  - Architecture aligned with professional SDK models (Android SDK, NDK)

    The SDK uses Python as an orchestration layer, responsible for coordinating complex compilation pipelines, IR transformation, and integration with native toolchains.

## License

Zyris Engine is licensed under the **MIT License**, maintaining compatibility with Godot Engine licensing.

Copyright (c) 2025-present Zyris contributors
Copyright (c) 2014-present Godot Engine contributors
Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur

See LICENSE.txt and COPYRIGHT.txt for full details.

## Acknowledgments

Zyris is built upon the incredible work of the **Godot Engine** community. We are deeply grateful to:

- **Juan Linietsky** and **Ariel Manzur** - Founders of Godot Engine
- All **Godot Engine contributors** - For creating a fantastic foundation
- The **open-source community** - For making projects like this possible

For the full list of Godot contributors, see [AUTHORS.md](AUTHORS.md) and [DONORS.md](DONORS.md).

## Links

- **Godot Engine Official**: <https://godotengine.org>
- **Godot GitHub**: <https://github.com/godotengine/godot>
- **Zyris Repository**: <https://github.com/Kaffyn/ZyrisEngine>
- **Issue Tracker**: <https://github.com/Kaffyn/ZyrisEngine/issues>

---

**Zyris Engine** - Expanding the boundaries of open-source game development
