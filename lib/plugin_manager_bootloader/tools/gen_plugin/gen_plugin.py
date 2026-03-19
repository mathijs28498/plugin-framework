import argparse
from pathlib import Path


def snake_to_pascal_case(snake_str: str):
    return "".join(x.capitalize() for x in snake_str.lower().split("_"))


def parse_arguments() -> tuple[Path, Path, str, str]:
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--interface-include-directory",
        required=True,
        type=Path,
        help="Interface directory",
    )

    parser.add_argument(
        "--plugin-base-directory",
        required=True,
        type=Path,
        help="Plugin source directory to place the plugin",
    )

    parser.add_argument(
        "--interface-name",
        required=True,
        help="Interface name (e.g. renderer)",
    )
    parser.add_argument(
        "--plugin-name", required=True, help="Plugin name (e.g. vulkan)"
    )

    args = parser.parse_args()

    return (
        args.interface_include_directory.resolve(),
        args.plugin_base_directory.resolve(),
        args.interface_name,
        args.plugin_name,
    )


def create_and_write_to_file(
    destination_path: Path,
    content: str,
    skip_unchanged_file: bool,
):
    if skip_unchanged_file and destination_path.exists():
        with open(destination_path, "r", encoding="utf-8") as f:
            if f.read() == content:
                print(
                    f"-- Skip writing as new content is the same as old: {destination_path}"
                )
                return
    print(f"-- Creating and writing to: {destination_path}")
    destination_path.parent.mkdir(exist_ok=True, parents=True)

    with open(destination_path, "w", encoding="utf-8") as f:
        f.write(content)


def configure_file(
    source_path: Path,
    destination_path: Path,
    replacements: dict[str, str],
    skip_unchanged_file: bool,
):
    print(f"-- Reading: {source_path}")
    with open(source_path, "r", encoding="utf-8") as f:
        generated_file = f.read()

    for placeholder, actual_value in replacements.items():
        generated_file = generated_file.replace(f"@{placeholder}@", actual_value)

    create_and_write_to_file(
        destination_path,
        generated_file,
        skip_unchanged_file,
    )


def main():
    (
        interface_include_directory_path,
        plugin_base_directory_path,
        interface_name,
        plugin_name,
    ) = parse_arguments()

    target_name = f"{interface_name}_{plugin_name}"
    plugin_source_directory_path = plugin_base_directory_path / target_name
    if plugin_source_directory_path.exists():
        raise FileExistsError(
            f"Plugin directory already exists: {str(plugin_source_directory_path)}"
        )

    interface_name_pascal = snake_to_pascal_case(interface_name)
    interface_header_file_name = f"{interface_name}_interface.h"
    plugin_register_header_file_name = f"{target_name}_register.h"
    plugin_register_source_file_name = f"{target_name}_register.c"
    plugin_header_file_name = f"{target_name}.h"
    plugin_source_file_name = f"{target_name}.c"
    source_file_names = [
        plugin_register_source_file_name,
        plugin_source_file_name,
    ]

    replacements = {
        "INTERFACE_NAME": interface_name,
        "PLUGIN_NAME": plugin_name,
        "INTERFACE_TYPE_NAME": f"{interface_name_pascal}Interface",
        "INTERFACE_CONTEXT_TYPE_NAME": f"{interface_name_pascal}InterfaceContext",
        "PLUGIN_INTERFACE_HEADER_FILE_NAME": interface_header_file_name,
        "PLUGIN_REGISTER_HEADER_FILE_NAME": plugin_register_header_file_name,
        "PLUGIN_HEADER_FILE_NAME": plugin_header_file_name,
        "TARGET_NAME": target_name,
        "SOURCE_FILE_NAMES": "\n    ".join(source_file_names),
    }

    template_directory = Path(__file__).parent / "templates"

    plugin_source_and_destination_file_names = [
        ("plugin_register.h.in", plugin_register_header_file_name),
        ("plugin_register.c.in", plugin_register_source_file_name),
        ("plugin.h.in", plugin_header_file_name),
        ("plugin.c.in", plugin_source_file_name),
        ("CMakeLists.txt.in", "CMakeLists.txt"),
        ("manifest.toml.in", "manifest.toml"),
    ]

    for source, destination in plugin_source_and_destination_file_names:
        configure_file(
            template_directory / source,
            plugin_source_directory_path / destination,
            replacements,
            False,
        )

    interface_destination_path = (
        interface_include_directory_path / interface_header_file_name
    )
    if interface_destination_path.exists():
        print(
            f"-- Skipping creation of interface header as interface header already exists: {interface_destination_path.as_posix()}"
        )
        return

    configure_file(
        template_directory / "interface.h.in",
        interface_destination_path,
        replacements,
        False,
    )


if __name__ == "__main__":
    main()
