import json
import re
from pathlib import Path

from plugin_manager_types import *


# TODO: Add error handling
def parse_plugin_registry(
    plugin_registry_json, build_platform: str
) -> list[InterfaceDefinition]:
    interface_definitions = []

    interface_definitions_json = plugin_registry_json["interfaces"]
    for interface_definition_json in interface_definitions_json:
        interface_name = interface_definition_json["interface_name"]

        if "default_plugin" not in interface_definition_json:
            raise ValueError(
                f"Failed to parse default plugin for interface '{interface_name}' in registry"
            )

        default_plugin_obj = interface_definition_json["default_plugin"]
        default_plugin_name = ""

        if isinstance(default_plugin_obj, str):
            default_plugin_name = default_plugin_obj
        elif isinstance(default_plugin_obj, list):
            # TODO: Add more complex default selection
            if len(default_plugin_obj) == 0:
                raise ValueError(
                    f"No default plugin found for interface '{interface_name}' in registry"
                )

            for default_plugin_info_json in default_plugin_obj:
                target = default_plugin_info_json["target"]

                # TODO Add other platforms
                if target == "win32" and build_platform == "win32":
                    default_plugin_name = default_plugin_info_json["plugin_name"]
                    break

        if default_plugin_name == "":
            raise ValueError(f"unable to find a default for interface '{interface_name}'")

        plugin_definitions = []
        for plugin_definition in interface_definition_json["plugins"]:
            plugin_name = plugin_definition["plugin_name"]
            plugin_module_path = Path(plugin_definition["module_path"])
            plugin_source_path = Path(plugin_definition["source_path"])
            plugin_definitions.append(
                PluginDefinition(
                    plugin_name=plugin_name,
                    module_path=plugin_module_path,
                    source_path=plugin_source_path,
                )
            )

        interface_definitions.append(
            InterfaceDefinition(
                interface_name=interface_name,
                default_plugin_name=default_plugin_name,
                plugin_definitions=plugin_definitions,
            )
        )

    return interface_definitions


# TODO: Check if this file exists, otherwise give error
# TODO: Do this a better way than regex to get the methods that exist
def read_internal_plugin_register_source(
    source_path: Path, interface_name: str
) -> tuple[list[str], list[PluginDependency], str, str, str]:
    forward_declarations = []

    get_interface_fn_text = "NULL"
    init_fn_text = "NULL"
    shutdown_fn_text = "NULL"

    with open(source_path, "r", encoding="utf-8") as f:
        plugin_register_source = f.read()
        get_interface_match = re.search(
            r"^[ \t]*PLUGIN_REGISTER_INTERFACE", plugin_register_source, re.MULTILINE
        )
        if get_interface_match is None:
            # TODO Add correct error
            raise ValueError(f"Cannot find interface method for interface '{interface_name}'")

        function_name = f"{interface_name}_get_interface"
        get_interface_fn_text = f"(PluginGetInterface_Fn){function_name}"

        forward_declarations.append(f"void *{function_name}(void);")

        init_match = re.search(
            r"^[ \t]*PLUGIN_REGISTER_INIT", plugin_register_source, re.MULTILINE
        )
        if init_match is not None:
            function_name = f"{interface_name}_init"
            init_fn_text = f"(PluginInit_Fn){function_name}"
            forward_declarations.append(f"int32_t {function_name}(void *context);")

        shutdown_match = re.search(
            r"^[ \t]*PLUGIN_REGISTER_SHUTDOWN", plugin_register_source, re.MULTILINE
        )
        if shutdown_match is not None:
            function_name = f"{interface_name}_shutdown"
            shutdown_fn_text = f"(PluginShutdown_Fn){function_name}"
            forward_declarations.append(f"int32_t {function_name}(void *context);")

        # TODO: Handle dependencies
        return (
            forward_declarations,
            [],
            get_interface_fn_text,
            init_fn_text,
            shutdown_fn_text,
        )


def parse_internal_plugins(
    plugin_registry_json,
) -> list[InternalPlugin]:
    internal_plugins: list[InternalPlugin] = []

    for internal_plugin_json in plugin_registry_json["internal_plugins"]:

        interface_name: str = internal_plugin_json["interface_name"]
        plugin_name: str = internal_plugin_json["plugin_name"]
        plugin_source_path: str = internal_plugin_json["plugin_source_path"]
        plugin_target_name: str = internal_plugin_json["cmake_target_name"]

        forward_declarations: list[str] = internal_plugin_json.get(
            "plugin_forward_declarations",
            [],
        )

        plugin_register_path = Path(
            f"{plugin_source_path}/{interface_name}_{plugin_name}_register.c"
        )

        source_data = PluginSourceData(
            target_name=plugin_target_name,
            source_path=Path(plugin_source_path),
            register_path=plugin_register_path,
        )

        (
            source_forward_declarations,
            dependencies,
            get_interface_fn_text,
            init_fn_text,
            shutdown_fn_text,
        ) = read_internal_plugin_register_source(plugin_register_path, interface_name)

        forward_declarations.extend(source_forward_declarations)

        internal_plugins.append(
            InternalPlugin(
                source_data=source_data,
                forward_declarations=forward_declarations,
                interface_name=interface_name,
                plugin_name=plugin_name,
                dependencies=dependencies,
                get_interface_fn_text=get_interface_fn_text,
                init_fn_text=init_fn_text,
                shutdown_fn_text=shutdown_fn_text,
                is_explicit=False,
                is_initialized=False,
            )
        )

    return internal_plugins


def parse_plugin_list(plugin_list_json) -> PluginListData:
    # TODO: Add error handling
    requested_plugins = [
        RequestedPlugin(
            interface_name=requested_plugin["interface_name"],
            plugin_name=requested_plugin.get("plugin_name", ""),
            is_explicit=True,
            is_resolved=False,
        )
        for requested_plugin in plugin_list_json["plugins"]
    ]
    return PluginListData(
        requested_plugins=requested_plugins,
        sorted_plugin_providers_indices=[],
        plugin_providers=[],
    )


def read_json(source_path: Path):
    print(f"-- Reading: {source_path}")
    with open(source_path, "r", encoding="utf-8") as f:
        return json.load(f)
