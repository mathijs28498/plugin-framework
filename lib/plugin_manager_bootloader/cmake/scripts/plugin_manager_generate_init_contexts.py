from plugin_manager_types import *
from pathlib import Path
import textwrap
from plugin_manager_generate_templates import indent_prefix, configure_file


def get_formatted_plugin_provider_c_struct(plugin_provider: PluginProvider) -> str:
    manifest = plugin_provider.plugin_manifest
    plugin_dependencies_text = f"{{0}}"
    if plugin_provider.dependencies:
        dependency_text_list: list[str] = []
        for dependency in plugin_provider.dependencies:
            dependency_text_list.append(
                textwrap.dedent(
                    f"""\
                    {{
                        .interface_name = "{dependency.interface_name}",
                        .is_resolved = false,
                        .set = {dependency.set_fn},
                    }}"""
                )
            )

        dependency_definitions_text = ",\n".join(dependency_text_list)
        plugin_dependencies_text = textwrap.indent(
            f"\n{dependency_definitions_text},\n{indent_prefix * 1}",
            indent_prefix * 2,
        )

    dependency_placeholder = "@DEPENDENCY_DEFINITIONS_TEXT@"
    plugin_provider_text = textwrap.dedent(
        f"""\
        {{
            .interface_name = "{manifest.interface_name}",
            .plugin_name = "{manifest.plugin_name}",
            .get_interface = {plugin_provider.get_interface_fn_text},
            .init = {plugin_provider.init_fn_text},
            .shutdown = {plugin_provider.shutdown_fn_text},
            .dependencies = {{{dependency_placeholder}}},
            .dependencies_len = {str(len(manifest.dependencies))},
            .is_explicit = {str(plugin_provider.is_explicit).lower()},
            .is_initialized = {str(plugin_provider.is_initialized).lower()},
        }}"""
    )
    plugin_provider_text = plugin_provider_text.replace(
        dependency_placeholder,
        plugin_dependencies_text,
    )

    return plugin_provider_text


def get_plugin_provider_index_by_interface_name(
    plugin_providers: list[PluginProvider], interface_name: str
) -> int:
    index = next(
        (
            i
            for i, plugin_provider in enumerate(plugin_providers)
            if plugin_provider.plugin_manifest.interface_name == interface_name
        ),
        None,
    )

    if index is None:
        raise ValueError(
            f"plugin provider implementing interface '{interface_name}' not found, please provide '{interface_name}' statically"
        )

    return index


def create_requested_plugins_text(requested_plugins: list[RequestedPlugin]) -> str:
    if not requested_plugins:
        return f"{{0}}"

    text = ",\n".join(
        textwrap.dedent(
            f"""\
            {{
                .interface_name = "{requested_plugin.interface_name}",
                .plugin_name = "{requested_plugin.plugin_name or ""}",
                .is_explicit = true,
                .is_resolved = false,
            }}"""
        )
        for requested_plugin in requested_plugins
    )

    text = textwrap.indent(
        f"\n{text},\n{indent_prefix * 2}",
        indent_prefix * 3,
    )

    return text


def create_sorted_plugin_providers_indices_text(
    sorted_plugin_providers_indices: list[int],
) -> str:
    if not sorted_plugin_providers_indices:
        return f"{{0}}"

    return ", ".join(str(index) for index in sorted_plugin_providers_indices)


def create_plugin_providers_text(plugin_providers: list[PluginProvider]) -> str:
    if not plugin_providers:
        return f"{{0}}"

    text = ",\n".join(
        get_formatted_plugin_provider_c_struct(plugin_provider)
        for plugin_provider in plugin_providers
    )
    text = textwrap.indent(
        f"\n{text},\n{indent_prefix * 2}",
        indent_prefix * 3,
    )

    return text


def generate_init_contexts_src(
    source_path: Path,
    destination_path: Path,
    requested_plugins: list[RequestedPlugin],
    sorted_plugin_providers_indices: list[int],
    plugin_providers: list[PluginProvider],
):
    requested_plugins_text = create_requested_plugins_text(requested_plugins)
    sorted_plugin_providers_indices_text = create_sorted_plugin_providers_indices_text(
        sorted_plugin_providers_indices
    )
    plugin_providers_text = create_plugin_providers_text(plugin_providers)

    logger_plugin_provider_index = get_plugin_provider_index_by_interface_name(
        plugin_providers, "logger"
    )

    environment_plugin_provider_index = get_plugin_provider_index_by_interface_name(
        plugin_providers, "environment"
    )

    replacements = {
        "REQUESTED_PLUGINS_TEXT": requested_plugins_text,
        "REQUESTED_PLUGINS_LEN": str(len(requested_plugins)),
        "SORTED_PLUGIN_PROVIDERS_INDICES_TEXT": sorted_plugin_providers_indices_text,
        "PLUGIN_PROVIDERS_TEXT": plugin_providers_text,
        "PLUGIN_PROVIDERS_LEN": str(len(plugin_providers)),
        "LOGGER_PLUGIN_PROVIDER_INDEX": str(logger_plugin_provider_index),
        "ENVIRONMENT_PLUGIN_PROVIDER_INDEX": str(environment_plugin_provider_index),
    }

    configure_file(source_path, destination_path, replacements, False)
