import argparse
from pathlib import Path
import re


def check_cmake_subdirectory_already_added(
    path_file: Path, subdirectory_name: str
) -> bool:
    escaped_subdirectory_name = re.escape(subdirectory_name)
    re_pattern = re.compile(
        rf'^\s*add_subdirectory\s*\(\s*(["\']?){escaped_subdirectory_name}\1\s*(?:[^\)]*)\)', 
        re.IGNORECASE
    )

    with open(path_file, "r") as f:
        for line in f:
            if re_pattern.search(line):
                return True

    return False


def snake_to_camel_case(snake_str: str):
    return "".join(x.capitalize() for x in snake_str.lower().split("_"))


def create_file(path_file: Path, content: str):
    print(f"Creating file: {path_file}")
    with open(path_file, "w") as f:
        f.write(content)


def append_file(path_file: Path, content: str):
    print(f"Appending to file: {path_file}")
    with open(path_file, "a") as f:
        f.write(content)


def create_dir(path_dir: Path):
    print(f"Creating dir: {path_dir}")
    path_dir.mkdir()


def create_interface(interface_name: str, path_interface_dir: Path):
    path_interface_header = path_interface_dir / f"{interface_name}_interface.h"
    if path_interface_header.exists():
        print(
            f"{interface_name}_interface already has a header, skipping header creation: {str(path_interface_header)}"
        )
        return

    create_file(
        path_interface_header,
        f"""#pragma once

#pragma pack(push, 8)

struct {snake_to_camel_case(interface_name)}InterfaceContext;

typedef struct {snake_to_camel_case(interface_name)}Interface
{{
    struct {snake_to_camel_case(interface_name)}InterfaceContext *context;
}} {snake_to_camel_case(interface_name)}Interface;

#pragma pack(pop)
""",
    )


def write_plugin_register_header(interface_name: str, path_file: Path):
    create_file(
        path_file,
        f"""#pragma once

#pragma pack(push, 8)

struct LoggerInterface;

typedef struct {snake_to_camel_case(interface_name)}InterfaceContext
{{
    struct LoggerInterface *logger;
}} {snake_to_camel_case(interface_name)}InterfaceContext;

#pragma pack(pop)""",
    )


def write_plugin_register_source(interface_name: str, plugin_name: str, path_file: Path):
    create_file(
        path_file,
        f"""#include "{plugin_name}_register.h"

#include <stdint.h>

#include <logger_interface.h>
#include <plugin_sdk.h>
#include <{interface_name}_interface.h>

#include "{plugin_name}.h"

#define REGISTER_DEPENDENCIES(X) \\
    X(LoggerInterface, logger, logger)

PLUGIN_REGISTER_DEPENDENCIES({snake_to_camel_case(interface_name)}InterfaceContext, REGISTER_DEPENDENCIES);

{snake_to_camel_case(interface_name)}Interface *get_interface(void)
{{
    static {snake_to_camel_case(interface_name)}InterfaceContext context = {{0}};

    static {snake_to_camel_case(interface_name)}Interface iface = {{
        .context = &context,
    }};

    return &iface;
}}

PLUGIN_REGISTER_INTERFACE(get_interface, {snake_to_camel_case(interface_name)}Interface);""",
    )


def write_plugin_header(plugin_name: str, path_file: Path):
    create_file(
        path_file,
        f"""#pragma once
""",
    )


def write_plugin_source(plugin_name: str, path_file: Path):
    create_file(
        path_file,
        f"""#include "{plugin_name}.h"

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER({plugin_name}, LOG_LEVEL_DEBUG);

#include "{plugin_name}_register.h" """,
    )


def write_plugin_cmakelists(interface_name: str, plugin_name: str, path_file: Path):
    create_file(
        path_file,
        f"""add_library({plugin_name} SHARED)

target_sources({plugin_name}
    PRIVATE
        {plugin_name}.c
        {plugin_name}_register.c

    PUBLIC
        FILE_SET HEADERS
)

target_link_libraries({plugin_name}
    PRIVATE
        plugin_manager_include
)

add_compile_definitions(PLUGIN_BUILD_SHARED)
add_compile_definitions(PLUGIN_INTERFACE_NAME={interface_name})""",
    )


def create_plugin(interface_name: str, plugin_name: str, path_plugins_dir: Path):
    path_plugin_dir = path_plugins_dir / f"{plugin_name}"

    create_dir(path_plugin_dir)

    path_plugins_cmakelists = path_plugins_dir / f"CMakeLists.txt"

    append_file(path_plugins_cmakelists, f"\nadd_subdirectory({plugin_name})")

    write_plugin_register_header(
        interface_name, path_plugin_dir / f"{plugin_name}_register.h"
    )
    write_plugin_register_source(
        interface_name, plugin_name, path_plugin_dir / f"{plugin_name}_register.c"
    )
    write_plugin_header(plugin_name, path_plugin_dir / f"{plugin_name}.h")
    write_plugin_source(plugin_name, path_plugin_dir / f"{plugin_name}.c")
    write_plugin_cmakelists(interface_name, plugin_name, path_plugin_dir / f"CMakeLists.txt")


def parse_arguments() -> tuple[str, str]:
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--interface_name",
        required=True,
        help="Interface name (e.g. renderer_interface)",
    )
    parser.add_argument(
        "--plugin_name", required=True, help="Plugin name (e.g. renderer_vulkan)"
    )

    args = parser.parse_args()

    return args.interface_name, args.plugin_name


def main():
    interface_name, plugin_name = parse_arguments()

    # Assuming this file is placed at root/tools
    path_root_dir = Path(__file__).parent.resolve().parent

    path_interface_dir = path_root_dir / "plugin_manager_include"
    if not path_interface_dir.exists():
        raise FileNotFoundError(
            f"Interface directory not found at: {str(path_interface_dir)}"
        )

    path_plugins_dir = path_root_dir / "plugins"
    if not path_plugins_dir.exists():
        raise FileNotFoundError(
            f"Plugins directory not found at: {str(path_plugins_dir)}"
        )

    path_plugins_cmakelists = path_plugins_dir / f"CMakeLists.txt"
    if not path_plugins_cmakelists.exists():
        raise FileNotFoundError(
            f"Plugins CMakeLists.txt not found at: {str(path_plugins_cmakelists)}"
        )

    if check_cmake_subdirectory_already_added(path_plugins_cmakelists, plugin_name):
        raise ValueError(
            f"Plugin '{plugin_name}' is already listed in: {str(path_plugins_cmakelists)}"
        )

    path_plugin_dir = path_plugins_dir / f"{plugin_name}"
    if path_plugin_dir.exists():
        raise FileExistsError(
            f"Plugin directory already exists: {str(path_plugin_dir)}"
        )

    create_interface(interface_name, path_interface_dir)
    create_plugin(interface_name, plugin_name, path_plugins_dir)


if __name__ == "__main__":
    main()
