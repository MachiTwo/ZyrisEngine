# Edit the following variables to change version info

major = 0
minor = 1
patch = 0
status = "dev"
doc_branch = "latest"


def get_godot_cpp_ref():
    import os

    deps_env_path = os.path.join(os.path.dirname(__file__), "deps.env")
    if not os.path.exists(deps_env_path):
        return "unknown"
    with open(deps_env_path, "r") as f:
        for line in f:
            if line.startswith("GODOT_CPP_REF="):
                return line.strip().split("=", 1)[1]
    return "unknown"


# *** Code that generates version header


def _git_hash(short: bool = False):
    import subprocess

    ret = "unknown"
    try:
        if short:
            cmd = ["git", "rev-parse", "--short", "HEAD"]
        else:
            cmd = ["git", "rev-parse", "HEAD"]
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, _ = proc.communicate()
        if proc.returncode == 0:
            ret = out.strip().decode("utf-8")
    except Exception:
        pass
    return ret


def _get_version_info():
    return {
        "major": major,
        "minor": minor,
        "patch": patch,
        "status": status,
        "doc_branch": doc_branch,
        "git_short_hash": _git_hash(short=True),
        "git_hash": _git_hash(short=False),
    }


def generate_version_header():
    import os

    version_info = _get_version_info()

    # Ensure core directory exists
    core_dir = os.path.join(os.path.dirname(__file__), "core")
    if not os.path.exists(core_dir):
        os.makedirs(core_dir)

    header_path = os.path.join(core_dir, "ability_system_version.gen.h")

    with open(header_path, "w") as f:
        f.write(
            """/* THIS FILE IS GENERATED DO NOT EDIT */
#ifndef ABILITY_SYSTEM_VERSION_GEN_H
#define ABILITY_SYSTEM_VERSION_GEN_H

#define ABILITY_SYSTEM_VERSION_MAJOR {major}
#define ABILITY_SYSTEM_VERSION_MINOR {minor}
#define ABILITY_SYSTEM_VERSION_PATCH {patch}
#define ABILITY_SYSTEM_VERSION_STATUS "{status}"

#define ABILITY_SYSTEM_VERSION_HASH "{git_hash}"
#define ABILITY_SYSTEM_VERSION_SHORT_HASH "{git_short_hash}"

#define ABILITY_SYSTEM_VERSION_DOC_BRANCH "{doc_branch}"
#define ABILITY_SYSTEM_VERSION_DOC_URL "https://github.com/MachiTwo/AbilitySystemPlugin"

#endif // ABILITY_SYSTEM_VERSION_GEN_H
""".format(**version_info)
        )
    print(f"Generated {header_path}")


if __name__ == "__main__":
    generate_version_header()
