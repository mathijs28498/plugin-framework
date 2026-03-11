from plugin_manager_types import *


def create_plugin_provider_from_manifest(
    plugin_manifest: PluginManifest,
) -> PluginProvider:
    function_name = f"{plugin_manifest.target_name}_get_interface"
    get_interface_fn_text = f"(PluginGetInterface_Fn){function_name}"
    framework_declarations = [f"void *{function_name}(void);"]

    init_fn_text = "NULL"
    shutdown_fn_text = "NULL"

    if plugin_manifest.init_fn:
        function_name = f"{plugin_manifest.target_name}_init"
        init_fn_text = f"(PluginInit_Fn){function_name}"
        framework_declarations.append(f"int32_t {function_name}(void *context);")

    if plugin_manifest.shutdown_fn:
        function_name = f"{plugin_manifest.target_name}_shutdown"
        shutdown_fn_text = f"(PluginShutdown_Fn){function_name}"
        framework_declarations.append(f"int32_t {function_name}(void *context);")

    plugin_provider_dependencies: list[PluginProviderDependency] = []
    for dependency in plugin_manifest.dependencies:
        function_name = f"{plugin_manifest.target_name}_set_{dependency.interface_name}"
        plugin_provider_dependencies.append(
            PluginProviderDependency(
                interface_name=dependency.interface_name,
                set_fn=f"(PluginSetDependency_Fn){function_name}",
            )
        )
        framework_declarations.append(
            f"void {function_name}(void *context, void *iface);"
        )

    # TODO: Make is explicit False and only true when is requested and also static only build
    return PluginProvider(
        plugin_manifest=plugin_manifest,
        framework_declarations=framework_declarations,
        dependencies=plugin_provider_dependencies,
        get_interface_fn_text=get_interface_fn_text,
        init_fn_text=init_fn_text,
        shutdown_fn_text=shutdown_fn_text,
        is_explicit=True,
        is_initialized=False,
    )


def create_static_plugin_providers(
    plugin_registry: PluginRegistry, build_dynamic_plugins: bool
) -> list[PluginProvider]:
    return [
        create_plugin_provider_from_manifest(manifest)
        for manifest in plugin_registry.plugin_manifests
        if not build_dynamic_plugins or manifest.static_only
    ]


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
