# meta-description: Advanced character controller with full Ability System and SaveServer integration

extends _BASE_

# ============================================================================
# CORE COMPONENTS
# ============================================================================

@onready var ability_system: AbilitySystemComponent = $AbilitySystemComponent
@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var sprite_3d: Sprite3D = $Sprite3D
@onready var camera_3d: Camera3D = $Camera3D

# ============================================================================
# ATTRIBUTES & COMBAT
# ============================================================================

@export var character_name: String = "Player"
@export_group("Attributes")
@export var level: int = 1
@export var experience: int = 0
@export_group("Combat")
@export var current_health: float = 100.0
@export var max_health: float = 100.0
@export_group("Movement")
@export var base_speed: float = 5.0
@export var jump_velocity: float = 4.5
@export var mouse_sensitivity: float = 0.002

# ============================================================================
# ABILITY SYSTEM CONFIGURATION
# ============================================================================

@export var ability_container: AbilitySystemAbilityContainer
@export var starting_abilities: Array[AbilitySystemAbility] = []
@export var jump_ability: AbilitySystemAbility

# Movement attributes managed by Ability System
var movement_speed: float
var jump_power: float
var can_move: bool = true
var can_jump: bool = true

# Camera control
var camera_rotation: Vector2

# ============================================================================
# INITIALIZATION
# ============================================================================

func _ready() -> void:
	_setup_ability_system()
	_setup_initial_attributes()
	_connect_signals()
	_setup_camera()

func _setup_ability_system() -> void:
	if not ability_system:
		push_error("AbilitySystemComponent required!")
		return

	# Setup explicit slots for animations and audio (Required as sibling cache was removed)
	ability_system.set_animation_player(animation_player)
	# ability_system.set_audio_player($AudioStreamPlayer)

	# Apply ability container if provided
	if ability_container:
		ability_system.apply_ability_container(ability_container)

	# Grant starting abilities
	for ability in starting_abilities:
		ability_system.give_ability(ability)

func _setup_initial_attributes() -> void:
	if not ability_system:
		return

	# Initialize base attributes
	ability_system.set_attribute_base_value(&"movement.speed", base_speed)
	ability_system.set_attribute_base_value(&"movement.jump_power", jump_velocity)
	ability_system.set_attribute_base_value(&"combat.health", current_health)
	ability_system.set_attribute_base_value(&"combat.max_health", max_health)

	# Cache current values
	movement_speed = ability_system.get_attribute_value(&"movement.speed")
	jump_power = ability_system.get_attribute_value(&"movement.jump_power")

func _connect_signals() -> void:
	if ability_system:
		ability_system.attribute_changed.connect(_on_attribute_changed)
		ability_system.tag_changed.connect(_on_tag_changed)

func _setup_camera() -> void:
	if camera_3d:
		# Lock mouse for first-person camera control
		Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

		# Initialize camera rotation
		camera_rotation = Vector2.ZERO

# ============================================================================
# INPUT HANDLING
# ============================================================================

func _unhandled_input(event: InputEvent) -> void:
	if not can_move:
		return

	if event is InputEventMouseMotion:
		_handle_camera_rotation(event)

	if event.is_action_pressed("ui_cancel"):
		_toggle_mouse_mode()

func _handle_camera_rotation(event: InputEventMouseMotion) -> void:
	if camera_3d:
		camera_rotation.y -= event.relative.x * mouse_sensitivity
		camera_rotation.x -= event.relative.y * mouse_sensitivity
		camera_rotation.x = clamp(camera_rotation.x, deg_to_rad(-80), deg_to_rad(80))

		camera_3d.rotation_degrees.x = rad_to_deg(camera_rotation.x)
		rotation_degrees.y = rad_to_deg(camera_rotation.y)

func _toggle_mouse_mode() -> void:
	if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
		Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
	else:
		Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

# ============================================================================
# GAMEPLAY LOOP
# ============================================================================

func _physics_process(delta: float) -> void:
	if not can_move:
		return

	_handle_gravity(delta)
	_handle_movement_3d(delta)
	_handle_jump()
	_apply_movement()

func _handle_gravity(delta: float) -> void:
	if not is_on_floor():
		velocity += get_gravity() * delta

func _handle_movement_3d(delta: float) -> void:
	var input_dir := Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	var direction := (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()

	if direction:
		velocity.x = direction.x * movement_speed
		velocity.z = direction.z * movement_speed
		_play_movement_animation(direction)
	else:
		velocity.x = move_toward(velocity.x, 0, movement_speed)
		velocity.z = move_toward(velocity.z, 0, movement_speed)
		_play_idle_animation()

func _handle_jump() -> void:
	if Input.is_action_just_pressed("ui_accept") and is_on_floor() and can_jump:
		velocity.y = jump_power

		# Try to activate jump ability
		if ability_system and jump_ability:
			ability_system.try_activate_ability(jump_ability)

func _apply_movement() -> void:
	move_and_slide()

# ============================================================================
# ANIMATION SYSTEM
# ============================================================================

func _play_movement_animation(direction: Vector3) -> void:
	if not animation_player:
		return

	# Determine animation based on movement direction
	var forward_dot = direction.dot(-global_transform.basis.z)
	var right_dot = direction.dot(global_transform.basis.x)

	if absf(forward_dot) > 0.5:
		if forward_dot > 0:
			animation_player.play("run_forward")
		else:
			animation_player.play("run_backward")
	elif absf(right_dot) > 0.5:
		if right_dot > 0:
			animation_player.play("run_right")
		else:
			animation_player.play("run_left")
	else:
		animation_player.play("run")

func _play_idle_animation() -> void:
	if animation_player and animation_player.has_animation("idle"):
		animation_player.play("idle")

# ============================================================================
# ABILITY SYSTEM EVENTS
# ============================================================================

func _on_attribute_changed(attribute_name: StringName, old_value: float, new_value: float) -> void:
	match attribute_name:
		&"movement.speed":
			movement_speed = new_value
		&"movement.jump_power":
			jump_power = new_value
		&"combat.health":
			current_health = new_value
			_handle_health_change(new_value)
		&"combat.max_health":
			max_health = new_value

func _on_tag_changed(tag_name: StringName, is_present: bool) -> void:
	match tag_name:
		&"state.stun":
			can_move = not is_present
		&"state.cant_jump":
			can_jump = not is_present
		&"state.dead":
			if is_present:
				_handle_death()

func _handle_health_change(health: float) -> void:
	# Update UI, play hurt animations, etc.
	if health <= 0 and not ability_system.has_tag(&"state.dead"):
		ability_system.add_tag(&"state.dead")

func _handle_death() -> void:
	can_move = false
	set_physics_process(false)

	# Release mouse on death
	Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)

	if animation_player and animation_player.has_animation("death"):
		animation_player.play("death")

# ============================================================================
# UTILITY METHODS
# ============================================================================

func _get_active_ability_names() -> Array[String]:
	var names: Array[String] = []
	if ability_system:
		# This would need to be implemented in the AbilitySystemComponent
		pass
	return names

func take_damage(amount: float, damage_type: StringName = &"physical") -> void:
	if not ability_system:
		current_health = maxf(0, current_health - amount)
		return

	# Create damage effect spec
	var damage_effect = AbilitySystemEffect.new()
	damage_effect.add_modifier(&"combat.health", AbilitySystemEffect.ADD, -amount)

	var spec = ability_system.make_outgoing_spec(damage_effect)
	ability_system.apply_effect_spec_to_self(spec)

func heal(amount: float) -> void:
	if not ability_system:
		current_health = minf(max_health, current_health + amount)
		return

	# Create healing effect spec
	var heal_effect = AbilitySystemEffect.new()
	# Using ADD with positive amount to increase health
	heal_effect.add_modifier(&"combat.health", AbilitySystemEffect.ADD, amount)

	var spec = ability_system.make_outgoing_spec(heal_effect)
	ability_system.apply_effect_spec_to_self(spec)

# ============================================================================
# 3D SPECIFIC METHODS
# ============================================================================

func get_look_direction() -> Vector3:
	return -global_transform.basis.z

func get_right_direction() -> Vector3:
	return global_transform.basis.x

func is_moving() -> bool:
	var horizontal_velocity := Vector2(velocity.x, velocity.z)
	return horizontal_velocity.length() > 0.1
