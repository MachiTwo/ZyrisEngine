def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_path():
    return "doc_classes"


def get_doc_classes():
    return [
        "AbilitySystemComponent",
        "AbilitySystemAttributeSet",
        "AbilitySystemAttribute",
        "AbilitySystemAbilityContainer",
        "AbilitySystemAbility",
        "AbilitySystemAbilitySpec",
        "AbilitySystemEffect",
        "AbilitySystemEffectSpec",
        "AbilitySystemTagSpec",
        "AbilitySystemCue",
        "AbilitySystemCueAnimation",
        "AbilitySystemCueAudio",
        "AbilitySystemCueSpec",
        "AbilitySystemMagnitudeCalculation",
        "AbilitySystemTargetData",
        "AbilitySystemTask",
        "AbilitySystem",
        "AbilitySystemEditorPlugin",
        "AbilitySystemTagsEditor",
        "AbilitySystemTagsSelector",
    ]


def get_icons_path():
    return "icons"
