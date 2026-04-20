from internal_core.datatypes import *
from plugin_sdk_core.datatypes import *
from collections import deque


# def create_plugin_provider_from_manifest(
#     plugin_manifest: PluginManifest,
# ) -> PluginProvider:
#     function_name = f"{plugin_manifest.target_name}_get_interface"
#     get_interface_fn_text = f"(PluginGetInterface_Fn){function_name}"
#     framework_declarations = [f"void *{function_name}(void);"]

#     init_fn_text = "NULL"
#     shutdown_fn_text = "NULL"

#     if plugin_manifest.has_init:
#         function_name = f"{plugin_manifest.target_name}_init"
#         init_fn_text = f"(PluginInit_Fn){function_name}"
#         framework_declarations.append(f"int32_t {function_name}(void *context);")

#     if plugin_manifest.has_shutdown:
#         function_name = f"{plugin_manifest.target_name}_shutdown"
#         shutdown_fn_text = f"(PluginShutdown_Fn){function_name}"
#         framework_declarations.append(f"int32_t {function_name}(void *context);")

#     plugin_provider_dependencies: list[PluginProviderDependency] = []
#     for dependency in plugin_manifest.dependencies:
#         function_name = f"{plugin_manifest.target_name}_set_{dependency.interface_name}"
#         plugin_provider_dependencies.append(
#             PluginProviderDependency(
#                 interface_name=dependency.interface_name,
#                 set_fn=f"(PluginSetDependency_Fn){function_name}",
#             )
#         )
#         framework_declarations.append(
#             f"void {function_name}(void *context, void *iface);"
#         )

#     return PluginProvider(
#         plugin_manifest=plugin_manifest,
#         framework_declarations=framework_declarations,
#         dependencies=plugin_provider_dependencies,
#         get_interface_fn_text=get_interface_fn_text,
#         init_fn_text=init_fn_text,
#         shutdown_fn_text=shutdown_fn_text,
#         is_explicit=False,
#         is_initialized=False,
#     )


# def create_static_plugin_providers(
#     plugin_registry: PluginRegistry, build_dynamic_plugins: bool
# ) -> list[PluginProvider]:
#     return [
#         create_plugin_provider_from_manifest(manifest)
#         for manifest in plugin_registry.plugin_manifests
#         if not build_dynamic_plugins
#     ]



# def plugin_provider_is_requested(
#     plugin_provider: PluginProvider, requested_plugin: RequestedPlugin
# ) -> bool:
#     return (
#         requested_plugin.plugin_name
#         and plugin_provider.plugin_manifest.plugin_name == requested_plugin.plugin_name
#     ) or (
#         not requested_plugin.plugin_name
#         and plugin_provider.plugin_manifest.interface_name
#         == requested_plugin.interface_name
#     )


# def is_dependency_already_requested(
#     dependency: PluginProviderDependency,
#     requested_plugins: list[RequestedPlugin],
#     new_requested_plugins: list[RequestedPlugin],
#     requested_plugin_providers: list[PluginProvider],
# ) -> bool:
#     return (
#         any(
#             requested_plugin.interface_name == dependency.interface_name
#             for requested_plugin in requested_plugins
#         )
#         or any(
#             requested_plugin.interface_name == dependency.interface_name
#             for requested_plugin in new_requested_plugins
#         )
#         or any(
#             plugin_provider.plugin_manifest.interface_name == dependency.interface_name
#             for plugin_provider in requested_plugin_providers
#         )
#     )


# def resolve_requested_plugin_providers(
#     all_plugin_providers: list[PluginProvider],
#     requested_plugins: list[RequestedPlugin],
# ) -> list[PluginProvider]:
#     requested_plugin_providers: list[PluginProvider] = []
#     for requested_plugin in requested_plugins:
#         plugin_provider_match = next(
#             (
#                 plugin_provider
#                 for plugin_provider in all_plugin_providers
#                 if plugin_provider_is_requested(plugin_provider, requested_plugin)
#             ),
#             None,
#         )

#         if plugin_provider_match is None:
#             raise ValueError(
#                 f"Requested plugin '{requested_plugin.interface_name}' - '{requested_plugin.plugin_name}' not found in registry."
#                 f"Check for typos in the interface/plugin name or ensure plugin is registered."
#             )

#         plugin_provider_match.is_explicit = requested_plugin.is_explicit
#         requested_plugin_providers.append(plugin_provider_match)

#     return requested_plugin_providers


# def check_resolved_requested_plugin_providers(
#     new_requested_plugin_providers: list[PluginProvider],
#     requested_plugin_providers: list[PluginProvider],
#     requested_plugins: list[RequestedPlugin],
# ) -> list[RequestedPlugin]:
#     new_requested_plugins: list[RequestedPlugin] = []
#     for plugin_provider in new_requested_plugin_providers:
#         for dependency in plugin_provider.dependencies:
#             if is_dependency_already_requested(
#                 dependency,
#                 requested_plugins,
#                 new_requested_plugins,
#                 requested_plugin_providers,
#             ):
#                 continue

#             new_requested_plugins.append(
#                 RequestedPlugin(
#                     interface_name=dependency.interface_name,
#                     plugin_name="",
#                     is_explicit=False,
#                 )
#             )

#     return new_requested_plugins



