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
    [Export] private AnimatedSprite2D _sprite;

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
    [Export] private float _baseSpeed = 300.0f;
    [Export] private float _jumpVelocity = -400.0f;

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

    #endregion

    #region Initialization

    public override void _Ready()
    {
        SetupAbilitySystem();
        SetupInitialAttributes();
        ConnectSignals();
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

    #endregion

    #region Gameplay Loop

    public override void _PhysicsProcess(double delta)
    {
        if (!_canMove) return;

        HandleGravity((float)delta);
        HandleMovement((float)delta);
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

    private void HandleMovement(float delta)
    {
        Vector2 direction = Input.GetVector("ui_left", "ui_right", "ui_up", "ui_down");

        if (direction != Vector2.Zero)
        {
            Velocity = new Vector2(direction.X * _movementSpeed, Velocity.Y);
            PlayMovementAnimation(direction);
        }
        else
        {
            Velocity = new Vector2(Mathf.MoveToward(Velocity.X, 0, _movementSpeed), Velocity.Y);
            PlayIdleAnimation();
        }
    }

    private void HandleJump()
    {
        if (Input.IsActionJustPressed("ui_accept") && IsOnFloor() && _canJump)
        {
            Velocity = new Vector2(Velocity.X, _jumpPower);

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

    private void PlayMovementAnimation(Vector2 direction)
    {
        if (_animationPlayer == null) return;

        if (direction.X > 0)
        {
            _sprite.FlipH = false;
            _animationPlayer.Play("run");
        }
        else if (direction.X < 0)
        {
            _sprite.FlipH = true;
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
}
