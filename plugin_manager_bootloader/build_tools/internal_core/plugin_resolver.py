from plugin_sdk_core.datatypes import PluginManifest, PluginLifetime
from internal_core.datatypes import AppConfig, PluginRegistry, RequestedPlugin, InterfaceDefinition
from typing import Optional
from collections import deque

RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH = 256


def resolve_requested_plugins(
    plugin_registry: PluginRegistry, requested_plugins: list[RequestedPlugin]
) -> list[PluginManifest]:
    registered_plugin_manifests: list[PluginManifest] = []

    for requested_plugin in requested_plugins:
        interface_name = requested_plugin.interface_name
        plugin_name = requested_plugin.plugin_name
        if not plugin_name:
            plugin_name = next(
                (
                    interface.default_plugin_name
                    for interface in plugin_registry.interfaces
                    if interface.interface_name == interface_name
                ),
                None,
            )

        if not plugin_name:
            raise ValueError(
                f"No default plugin found for interface '{interface_name}'"
            )

        found_plugin_manifest: Optional[PluginManifest] = None
        for plugin_manifest in plugin_registry.plugin_manifests:
            if (
                interface_name == plugin_manifest.interface_name
                and plugin_name == plugin_manifest.plugin_name
            ):
                found_plugin_manifest = plugin_manifest
                break

        if not found_plugin_manifest:
            raise ValueError(f"Plugin '{interface_name}' - '{plugin_name}'")

        registered_plugin_manifests.append(found_plugin_manifest)

    return registered_plugin_manifests


def dependency_already_requested(
    plugin_manifests: list[PluginManifest],
    requested_plugins: list[RequestedPlugin],
    dependency_interface_name: str,
) -> bool:
    return any(
        plugin_manifest.interface_name == dependency_interface_name
        for plugin_manifest in plugin_manifests
    ) or any(
        requested_plugin.interface_name == dependency_interface_name
        for requested_plugin in requested_plugins
    )


def resolve_plugin_dependencies(
    plugin_manifests: list[PluginManifest], plugin_manifests_offset: int
) -> list[RequestedPlugin]:
    requested_plugins: list[RequestedPlugin] = []

    for plugin_manifest in plugin_manifests[plugin_manifests_offset:]:
        for dependency_interface_name in plugin_manifest.dependencies:
            if dependency_already_requested(
                plugin_manifests, requested_plugins, dependency_interface_name
            ):
                continue

            requested_plugins.append(
                RequestedPlugin(
                    interface_name=dependency_interface_name,
                    plugin_name=None,
                    lifetime=PluginLifetime.UNKNOWN,
                    is_explicit=False,
                )
            )

    return requested_plugins


def resolve_compile_time_plugins(
    app_config: AppConfig, plugin_registry: PluginRegistry
) -> list[PluginManifest]:
    plugin_manifests: list[PluginManifest] = []
    plugin_manifests_offset: int = 0
    requested_plugins = list(app_config.requested_plugins)
    if not any(
        requested_plugin.interface_name == "plugin_manager"
        for requested_plugin in requested_plugins
    ):
        requested_plugins.append(
            RequestedPlugin(
                interface_name="plugin_manager",
                plugin_name=None,
                lifetime=PluginLifetime.UNKNOWN,
                is_explicit=False,
            )
        )

    for _ in range(RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH):
        if not requested_plugins:
            break

        plugin_manifests.extend(
            resolve_requested_plugins(plugin_registry, requested_plugins)
        )

        requested_plugins = resolve_plugin_dependencies(
            plugin_manifests, plugin_manifests_offset
        )
        plugin_manifests_offset = len(plugin_manifests)

    else:
        raise RuntimeError(
            f"Hit maximum recursive dependency solver depth, maximum ({RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH})"
        )

    return plugin_manifests


def plugin_manifest_depends_on(
    plugin_manifest: PluginManifest, interface_name: str
) -> bool:
    return any(
        dependency_interface_name == interface_name
        for dependency_interface_name in plugin_manifest.dependencies.keys()
    )


def sort_plugin_manifests(
    plugin_manifests: list[PluginManifest],
) -> list[PluginManifest]:
    indegrees = [
        len(plugin_manifest.dependencies) for plugin_manifest in plugin_manifests
    ]
    sorted_plugin_manifests_indices_queue = deque()

    for i, indegree in enumerate(indegrees):
        if indegree == 0:
            sorted_plugin_manifests_indices_queue.append(i)

    processed_count = 0
    sorted_plugin_manifests: list[PluginManifest] = []

    for _ in range(len(plugin_manifests) + 1):
        if not sorted_plugin_manifests_indices_queue:
            break

        current_idx = sorted_plugin_manifests_indices_queue.popleft()
        current_plugin_manifest = plugin_manifests[current_idx]
        sorted_plugin_manifests.append(current_plugin_manifest)
        processed_count += 1

        for i, dependent_plugin_manifest in enumerate(plugin_manifests):
            if indegrees[i] == 0:
                continue

            if plugin_manifest_depends_on(
                dependent_plugin_manifest,
                current_plugin_manifest.interface_name,
            ):
                indegrees[i] -= 1

                if indegrees[i] == 0:
                    sorted_plugin_manifests_indices_queue.append(i)
    else:
        raise ValueError(
            f"Plugin topological sort exceeded max iterations ({len(plugin_manifests) + 1})"
        )

    if processed_count != len(plugin_manifests):
        raise ValueError(
            f"Cyclic dependency detected! {processed_count} times processed, should be {len(plugin_manifests)}"
        )

    return sorted_plugin_manifests


def create_interface_definitions(
    plugin_registry: PluginRegistry,
) -> list[InterfaceDefinition]:
    interface_definitions: list[InterfaceDefinition] = []
    for interface in plugin_registry.interfaces:
        default_plugin_manifest: Optional[PluginManifest] = None
        plugin_manifests: list[PluginManifest] = []
        for plugin_manifest in plugin_registry.plugin_manifests:
            if interface.interface_name != plugin_manifest.interface_name:
                continue
            plugin_manifests.append(plugin_manifest)
            if plugin_manifest.plugin_name == interface.default_plugin_name:
                default_plugin_manifest = plugin_manifest

        if default_plugin_manifest is None:
            raise ValueError(
                f"Error in interface '{interface.interface_name}': default plugin '{interface.default_plugin_name}' not found"
            )
        interface_definitions.append(
            InterfaceDefinition(
                interface_name=interface.interface_name,
                default_plugin_manifest=default_plugin_manifest,
                plugin_manifests=plugin_manifests,
            )
        )

    return interface_definitions
