import argparse
from pathlib import Path


def check_cmake_subdirectory_already_added(
    path_file: Path, subdirectory_name: str
) -> bool:
    # TODO: Add more robust check with different permutations of the subdirectory_line
    subdirectory_line = f"add_subdirectory({subdirectory_name})"
    with open(path_file, "r") as f:
        for line in f:
            if line.strip() == subdirectory_line:
                return True

    return False


def snake_to_camel_case(snake_str: str):
    return "".join(x.capitalize() for x in snake_str.lower().split("_"))


def create_file(path_file: Path, content: str):
    print(f"Creating file: {path_file}")
    with open(path_file, "w") as f:
        f.write(content)


def append_file(path_file: Path, content: str):
    print(f"Appending file: {path_file}")
    with open(path_file, "a") as f:
        f.write(content)


def create_dir(path_dir: Path):
    print(f"Creating dir: {path_dir}")
    path_dir.mkdir()


def create_api(api_name: str, path_api_dir: Path):
    path_api_header = path_api_dir / f"{api_name}.h"
    if path_api_header.exists():
        print(
            f"{api_name} already has a header, skipping header creation: {str(path_api_header)}"
        )
        return

    create_file(
        path_api_header,
        f"""#pragma once

#pragma pack(push, 8)

struct {snake_to_camel_case(api_name)}Context;

typedef struct {snake_to_camel_case(api_name)}
{{
    struct {snake_to_camel_case(api_name)}Context *context;
}} {snake_to_camel_case(api_name)};

#pragma pack(pop)
""",
    )


def write_plugin_register_header(api_name: str, plugin_name: str, path_file):
    create_file(
        path_file,
        f"""#pragma once

#pragma pack(push, 8)

struct LoggerApi;

typedef struct {snake_to_camel_case(api_name)}Context
{{
    struct LoggerApi *logger_api;
}} {snake_to_camel_case(api_name)}Context;

#pragma pack(pop)""",
    )


def write_plugin_register_source(api_name: str, plugin_name: str, path_file):
    create_file(
        path_file,
        f"""#include "{plugin_name}_register.h"

#include <stdint.h>

#include <logger_api.h>
#include <plugin_manager_impl.h>
#include <{api_name}.h>

#include "{plugin_name}.h"

#define PLUGIN_API_NAME {api_name}

#define REGISTER_DEPENDENCIES(X) \\
    X(LoggerApi, logger_api, logger_api)

PLUGIN_REGISTER_DEPENDENCIES({snake_to_camel_case(api_name)}Context, REGISTER_DEPENDENCIES);

{snake_to_camel_case(api_name)} *get_api()
{{
    static {snake_to_camel_case(api_name)}Context context = {{0}};

    static {snake_to_camel_case(api_name)} api = {{
        .context = &context,
    }};

    return &api;
}}

PLUGIN_REGISTER_API(get_api, {snake_to_camel_case(api_name)});""",
    )


def write_plugin_header(plugin_name: str, path_file):
    create_file(
        path_file,
        f"""#pragma once
""",
    )


def write_plugin_source(plugin_name: str, path_file):
    create_file(
        path_file,
        f"""#include "{plugin_name}.h"

#include <logger_api.h>
LOGGER_API_REGISTER({plugin_name}, LOG_LEVEL_DEBUG);

#include "{plugin_name}_register.h" """,
    )


def write_plugin_cmakelists(plugin_name: str, path_file):
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
        plugin_manager_common
)""",
    )


def create_plugin(api_name: str, plugin_name: str, path_plugins_dir: Path):
    path_plugin_dir = path_plugins_dir / f"{plugin_name}"

    create_dir(path_plugin_dir)

    path_plugins_cmakelists = path_plugins_dir / f"CMakeLists.txt"

    append_file(path_plugins_cmakelists, f"\nadd_subdirectory({plugin_name})")

    write_plugin_register_header(
        api_name, plugin_name, path_plugin_dir / f"{plugin_name}_register.h"
    )
    write_plugin_register_source(
        api_name, plugin_name, path_plugin_dir / f"{plugin_name}_register.c"
    )
    write_plugin_header(plugin_name, path_plugin_dir / f"{plugin_name}.h")
    write_plugin_source(plugin_name, path_plugin_dir / f"{plugin_name}.c")
    write_plugin_cmakelists(plugin_name, path_plugin_dir / f"CMakeLists.txt")


def parse_arguments() -> tuple[str, str]:
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--api_name", required=True, help="Api name (e.g. renderer_api)"
    )
    parser.add_argument(
        "--plugin_name", required=True, help="Plugin name (e.g. renderer_vulkan_api)"
    )

    args = parser.parse_args()

    return args.api_name, args.plugin_name


def main():
    api_name, plugin_name = parse_arguments()

    # Assuming this file is placed at root/tools
    path_root_dir = Path(__file__).parent.resolve().parent

    path_api_dir = path_root_dir / "plugin_manager_common"
    if not path_api_dir.exists():
        raise FileNotFoundError(f"API directory not found at: {str(path_api_dir)}")

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

    create_api(api_name, path_api_dir)
    create_plugin(api_name, plugin_name, path_plugins_dir)


if __name__ == "__main__":
    main()
