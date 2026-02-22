"""Functions used to generate source files during build time"""

import os

import methods


def parse_template(inherits, source, delimiter):
    script_template = {
        "inherits": inherits,
        "name": "",
        "description": "",
        "version": "",
        "script": "",
        "space-indent": "4",
    }
    meta_prefix = delimiter + " meta-"
    meta = ["name", "description", "version", "space-indent"]

    with open(source, "r", encoding="utf-8") as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith(meta_prefix):
                line = line[len(meta_prefix) :]
                for m in meta:
                    if line.startswith(m):
                        strip_length = len(m) + 1
                        script_template[m] = line[strip_length:].strip()
            else:
                script_template["script"] += line
        if script_template["space-indent"] != "":
            indent = " " * int(script_template["space-indent"])
            script_template["script"] = script_template["script"].replace(indent, "_TS_")
        if script_template["name"] == "":
            script_template["name"] = os.path.splitext(os.path.basename(source))[0].replace("_", " ").title()

        # Escape string for C++ string literal
        script_template["script"] = (
            script_template["script"]
            .replace("\\", "\\\\")
            .replace('"', '\\"')
            .replace("\n", "\\n")
            .replace("\t", "_TS_")
        )

        return (
            f'{{ "{script_template["inherits"]}", '
            + f'"{script_template["name"]}", '
            + f'"{script_template["description"]}", '
            + f'"{script_template["script"]}" }},'
        )


def make_templates(target, source, env):
    parsed_templates = []

    for filepath in source:
        filepath = str(filepath)
        node_name = os.path.basename(os.path.dirname(filepath))

        # Determine delimiter based on file extension
        delimiter = "#"
        ext = os.path.splitext(filepath)[1]
        if ext == ".cs":
            delimiter = "//"

        parsed_templates.append(parse_template(node_name, filepath, delimiter))

    parsed_template_string = "\n\t".join(parsed_templates)

    with methods.generated_wrapper(str(target[0])) as file:
        file.write('#include "core/object/object.h"\n')
        file.write('#include "core/object/script_language.h"\n\n')
        file.write("struct AbilitySystemScriptTemplate {\n")
        file.write("\tconst char *inherit;\n")
        file.write("\tconst char *name;\n")
        file.write("\tconst char *description;\n")
        file.write("\tconst char *content;\n")
        file.write("};\n\n")
        file.write(f"inline constexpr int ABILITY_SYSTEM_TEMPLATES_ARRAY_SIZE = {len(parsed_templates)};\n")
        file.write(
            "static const struct AbilitySystemScriptTemplate ABILITY_SYSTEM_TEMPLATES[ABILITY_SYSTEM_TEMPLATES_ARRAY_SIZE] = {\n"
        )
        file.write(f"\t{parsed_template_string}\n")
        file.write("};\n")
