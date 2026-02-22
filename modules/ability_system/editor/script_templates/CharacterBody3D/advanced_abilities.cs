// meta-description: Advanced character controller with full Ability System and SaveServer integration
using _BINDINGS_NAMESPACE_;
using System;
using System.Collections.Generic;

public partial class _CLASS_ : _BASE_
{
    #region Core Components

    [ExportGroup("Core Components")]
    [Export] private AbilitySystemComponent _abilitySystem;
    [Export] private AnimationPlayer _animationPlayer;
    [Export] private Sprite3D _sprite3D;
    [Export] private Camera3D _camera3D;

    #endregion

    #region Attributes & Combat

    [ExportGroup("Attributes")]
    [Export] private string _characterName = "Player";
    [Export] private int _level = 1;
    [Export] private int _experience = 0;
    [ExportGroup("Combat")]
    [Export] private float _currentHealth = 100.0f;
    [Export] private float _maxHealth = 100.0f;
    [ExportGroup("Movement")]
    [Export] private float _baseSpeed = 5.0f;
    [Export] private float _jumpVelocity = 4.5f;
    [Export] private float _mouseSensitivity = 0.002f;

    #endregion

    #region Ability System Configuration

    [ExportGroup("Ability System")]
    [Export] private AbilitySystemAbilityContainer _abilityContainer;
    [Export] private AbilitySystemAbility[] _startingAbilities = Array.Empty<AbilitySystemAbility>();
    [Export] private AbilitySystemAbility _jumpAbility;

    #endregion

    #region Runtime State

    // Movement attributes managed by Ability System
    private float _movementSpeed;
    private float _jumpPower;
    private bool _canMove = true;
    private bool _canJump = true;

    // Camera control
    private Vector2 _cameraRotation;

    #endregion

    #region Initialization

    public override void _Ready()
    {
        SetupAbilitySystem();
        SetupInitialAttributes();
        ConnectSignals();
        SetupCamera();
    }

    private void SetupAbilitySystem()
    {
        if (_abilitySystem == null)
        {
            GD.PushError("AbilitySystemComponent required!");
            return;
        }

        // Setup explicit slots for animations and audio (Required as sibling cache was removed)
        _abilitySystem.SetAnimationPlayer(_animationPlayer);
        // _abilitySystem.SetAudioPlayer(GetNode<AudioStreamPlayer>("AudioStreamPlayer"));

        // Apply ability container if provided
        if (_abilityContainer != null)
        {
            _abilitySystem.ApplyAbilityContainer(_abilityContainer);
        }

        // Grant starting abilities
        foreach (var ability in _startingAbilities)
        {
            _abilitySystem.GiveAbility(ability);
        }
    }

    private void SetupInitialAttributes()
    {
        if (_abilitySystem == null) return;

        // Initialize base attributes
        _abilitySystem.SetAttributeBaseValue("movement.speed", _baseSpeed);
        _abilitySystem.SetAttributeBaseValue("movement.jump_power", _jumpVelocity);
        _abilitySystem.SetAttributeBaseValue("combat.health", _currentHealth);
        _abilitySystem.SetAttributeBaseValue("combat.max_health", _maxHealth);

        // Cache current values
        _movementSpeed = _abilitySystem.GetAttributeValue("movement.speed");
        _jumpPower = _abilitySystem.GetAttributeValue("movement.jump_power");
    }

    private void ConnectSignals()
    {
        if (_abilitySystem != null)
        {
            _abilitySystem.AttributeChanged += OnAttributeChanged;
            _abilitySystem.TagChanged += OnTagChanged;
        }
    }

    private void SetupCamera()
    {
        if (_camera3D != null)
        {
            // Lock mouse for first-person camera control
            Input.MouseMode = Input.MouseModeEnum.Captured;

            // Initialize camera rotation
            _cameraRotation = Vector2.Zero;
        }
    }

    #endregion

    #region Input Handling

    public override void _UnhandledInput(InputEvent @event)
    {
        if (!_canMove) return;

        if (@event is InputEventMouseMotion mouseMotion)
        {
            HandleCameraRotation(mouseMotion);
        }

        if (@event.IsActionPressed("ui_cancel"))
        {
            ToggleMouseMode();
        }
    }

    private void HandleCameraRotation(InputEventMouseMotion mouseMotion)
    {
        if (_camera3D != null)
        {
            _cameraRotation.Y -= mouseMotion.Relative.X * _mouseSensitivity;
            _cameraRotation.X -= mouseMotion.Relative.Y * _mouseSensitivity;
            _cameraRotation.X = Mathf.Clamp(_cameraRotation.X, Mathf.DegToRad(-80), Mathf.DegToRad(80));

            _camera3D.RotationDegrees.X = Mathf.RadToDeg(_cameraRotation.X);
            RotationDegrees.Y = Mathf.RadToDeg(_cameraRotation.Y);
        }
    }

    private void ToggleMouseMode()
    {
        if (Input.MouseMode == Input.MouseModeEnum.Captured)
        {
            Input.MouseMode = Input.MouseModeEnum.Visible;
        }
        else
        {
            Input.MouseMode = Input.MouseModeEnum.Captured;
        }
    }

    #endregion

    #region Gameplay Loop

    public override void _PhysicsProcess(double delta)
    {
        if (!_canMove) return;

        HandleGravity((float)delta);
        HandleMovement3D((float)delta);
        HandleJump();
        ApplyMovement();
    }

    private void HandleGravity(float delta)
    {
        if (!IsOnFloor())
        {
            Velocity += GetGravity() * delta;
        }
    }

    private void HandleMovement3D(float delta)
    {
        Vector2 inputDir = Input.GetVector("ui_left", "ui_right", "ui_up", "ui_down");
        Vector3 direction = (Transform.Basis * new Vector3(inputDir.X, 0, inputDir.Y)).Normalized();

        if (direction != Vector3.Zero)
        {
            Velocity = new Vector3(direction.X * _movementSpeed, Velocity.Y, direction.Z * _movementSpeed);
            PlayMovementAnimation(direction);
        }
        else
        {
            Velocity = new Vector3(Mathf.MoveToward(Velocity.X, 0, _movementSpeed),
                                  Velocity.Y,
                                  Mathf.MoveToward(Velocity.Z, 0, _movementSpeed));
            PlayIdleAnimation();
        }
    }

    private void HandleJump()
    {
        if (Input.IsActionJustPressed("ui_accept") && IsOnFloor() && _canJump)
        {
            Velocity = new Vector3(Velocity.X, _jumpPower, Velocity.Z);

            // Try to activate jump ability
            if (_abilitySystem != null && _jumpAbility != null)
            {
                _abilitySystem.TryActivateAbility(_jumpAbility);
            }
        }
    }

    private void ApplyMovement()
    {
        MoveAndSlide();
    }

    #endregion

    #region Animation System

    private void PlayMovementAnimation(Vector3 direction)
    {
        if (_animationPlayer == null) return;

        // Determine animation based on movement direction
        float forwardDot = direction.Dot(-GlobalTransform.Basis.Z);
        float rightDot = direction.Dot(GlobalTransform.Basis.X);

        if (Mathf.Abs(forwardDot) > 0.5f)
        {
            if (forwardDot > 0)
            {
                _animationPlayer.Play("run_forward");
            }
            else
            {
                _animationPlayer.Play("run_backward");
            }
        }
        else if (Mathf.Abs(rightDot) > 0.5f)
        {
            if (rightDot > 0)
            {
                _animationPlayer.Play("run_right");
            }
            else
            {
                _animationPlayer.Play("run_left");
            }
        }
        else
        {
            _animationPlayer.Play("run");
        }
    }

    private void PlayIdleAnimation()
    {
        if (_animationPlayer != null && _animationPlayer.HasAnimation("idle"))
        {
            _animationPlayer.Play("idle");
        }
    }

    #endregion

    #region Ability System Events

    private void OnAttributeChanged(StringName attributeName, float oldValue, float newValue)
    {
        switch (attributeName)
        {
            case "movement.speed":
                _movementSpeed = newValue;
                break;
            case "movement.jump_power":
                _jumpPower = newValue;
                break;
            case "combat.health":
                _currentHealth = newValue;
                HandleHealthChange(newValue);
                break;
            case "combat.max_health":
                _maxHealth = newValue;
                break;
        }
    }

    private void OnTagChanged(StringName tagName, bool isPresent)
    {
        switch (tagName)
        {
            case "state.stun":
                _canMove = !isPresent;
                break;
            case "state.cant_jump":
                _canJump = !isPresent;
                break;
            case "state.dead":
                if (isPresent)
                {
                    HandleDeath();
                }
                break;
        }
    }

    private void HandleHealthChange(float health)
    {
        // Update UI, play hurt animations, etc.
        if (health <= 0 && !_abilitySystem.HasTag("state.dead"))
        {
            _abilitySystem.AddTag("state.dead");
        }
    }

    private void HandleDeath()
    {
        _canMove = false;
        SetPhysicsProcess(false);

        // Release mouse on death
        Input.MouseMode = Input.MouseModeEnum.Visible;

        if (_animationPlayer != null && _animationPlayer.HasAnimation("death"))
        {
            _animationPlayer.Play("death");
        }
    }

    #endregion

    #region Utility Methods

    private Godot.Collections.Array GetActiveAbilityNames()
    {
        var names = new Godot.Collections.Array();
        if (_abilitySystem != null)
        {
            // This would need to be implemented in the AbilitySystemComponent
        }
        return names;
    }

    public void TakeDamage(float amount, StringName damageType = "physical")
    {
        if (_abilitySystem == null)
        {
            _currentHealth = Mathf.Max(0, _currentHealth - amount);
            return;
        }

        // Create damage effect spec
        var damageEffect = new AbilitySystemEffect();
        damageEffect.AddModifier("combat.health", AbilitySystemEffect.ModifierOp.Add, -amount);

        var spec = _abilitySystem.MakeOutgoingSpec(damageEffect);
        _abilitySystem.ApplyEffectSpecToSelf(spec);
    }

    public void Heal(float amount)
    {
        if (_abilitySystem == null)
        {
            _currentHealth = Mathf.Min(_maxHealth, _currentHealth + amount);
            return;
        }

        // Create healing effect spec
        var healEffect = new AbilitySystemEffect();
        healEffect.AddModifier("combat.health", AbilitySystemEffect.ModifierOp.Add, amount);

        var spec = _abilitySystem.MakeOutgoingSpec(healEffect);
        _abilitySystem.ApplyEffectSpecToSelf(spec);
    }

    #endregion

    #region 3D Specific Methods

    public Vector3 GetLookDirection()
    {
        return -GlobalTransform.Basis.Z;
    }

    public Vector3 GetRightDirection()
    {
        return GlobalTransform.Basis.X;
    }

    public bool IsMoving()
    {
        Vector2 horizontalVelocity = new Vector2(Velocity.X, Velocity.Z);
        return horizontalVelocity.Length() > 0.1f;
    }

    #endregion
}
