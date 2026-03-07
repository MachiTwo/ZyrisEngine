def can_build(env, platform):
    return True


def configure(env):
    env.Append(CPPDEFINES=["ABILITY_SYSTEM_MODULE"])


def get_doc_classes():
    return [
        "AbilitySystem",
        "AbilitySystemAbility",
        "AbilitySystemAbilityContainer",
        "AbilitySystemAbilitySpec",
        "AbilitySystemAttribute",
        "AbilitySystemAttributeSet",
        "AbilitySystemComponent",
        "AbilitySystemCue",
        "AbilitySystemCueAnimation",
        "AbilitySystemCueAudio",
        "AbilitySystemCueSpec",
        "AbilitySystemEditorPlugin",
        "AbilitySystemEditorPropertyName",
        "AbilitySystemEditorPropertySelector",
        "AbilitySystemEditorPropertyTagSelector",
        "AbilitySystemEffect",
        "AbilitySystemEffectSpec",
        "AbilitySystemInspectorPlugin",
        "AbilitySystemTagSpec",
        "AbilitySystemTagsPanel",
        "AbilitySystemTask",
    ]


def get_doc_path():
    return "doc_classes"
