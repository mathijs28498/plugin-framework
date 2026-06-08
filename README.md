# Plugin Framework

A modular, dependency-aware, statically allocated plugin framework written in C, designed for desktop, web, and embedded systems. Plugins are either dynamically loaded at runtime, or statically linked at compile time. Based on a user configurated app.toml and plugin_registry.toml it determines which plugins to load and automatically resolves and injects dependencies. The framework and its plugins use purely static memory. Instead a statically sized memory pool (size determined by a configuration) gets passed to the plugin framework. Plugins get allocated from a slab partition of the memory pool, dynamic memory can be allocated via a buddy allocator plugin.

The framework knows 2 lifetimes, singleton and scoped. Singleton plugins are initialized at the start of the application and get shutdown when the application finishes. Scoped plugins live for the duration of their scope. During the initialization of singleton plugins, the buddy allocator acts as a permanent allocation for the lifetime of the application.

## Architecture

```
┌────────────────────────────────────────────────────┐
│                  Host Application                  │
│                                                    │
│  ┌───────────────────────────────────────────┐     │
│  │           Bootloader (library)            │     │
│  │                                           │     │
│  │  1. Allocates memory pool at startup      │     │
│  │  2. Launches plugin_manager plugin        │     │
│  └───────────────────┬───────────────────────┘     │
│                      │                             │
│  ┌───────────────────▼──────────────────────────┐  │
│  │            plugin_manager                    │  │
│  │            1 KiB slab                        │  │
│  │                                              │  │
│  │  - Initialises its own dependencies          │  │
│  │  - Resolves and creates requested plugins    │  │
│  │  - Topologically sorts and inits dependencies│  │
│  └──────┬────────────┬────────────┬─────────────┘  │
│         │            │            │                │
│  ┌──────▼──┐   ┌─────▼───┐  ┌─────▼───┐            │
│  │Plugin A │   │Plugin B │  │Plugin C │  ...       │
│  │ 1KiB    │   │ 1KiB    │  │ 1KiB    │            │
│  │ slab    │   │ slab    │  │ slab    │            │
│  └─────────┘   └─────────┘  └─────────┘            │
│                                                    │
│  ╔═════════════════════════════════════════╗       │
│  ║         Memory Pool (startup alloc)     ║       │
│  ║  [ 1KiB ][ 1KiB ][ 1KiB ][ 1KiB ] ...   ║       │
│  ║  [     Frozen permanent memory pool   ] ║       │
│  ║  [     Buddy allocator                ] ║       │
│  ╚═════════════════════════════════════════╝       │
└────────────────────────────────────────────────────┘
```

Each plugin exposes a standard interface. The host resolves the load order based on declared dependencies before any plugin is initialised. Missing or cyclic dependencies are caught in this step and result in a loud error.

## Features

- **Linux/Zephyr RTOS style subsystem** - plugins communicate via interfaces that expose a vtable with an opaqua context struct, similar to how drivers and subsystems are abstracted in Linux and Zephyr RTOS
- **Dynamic plugin loading** - plugins are discovered and loaded at runtime via shared libraries or at compile time using a combindation of CMake and Python
- **Dependency resolution** - automatic ordering and validation of plugin dependencies at load time
- **Buddy allocator** - each plugin can allocate dynamic memory using the buddy allocator plugin. This allows for fast dynamic memory without heap allocations
- **Isolated plugin contexts** - plugins cannot access each other's memory directly; communication goes through well-defined interfaces
- **Cross-platform** - targets Windows (MSVC), with Linux, web, and embedded systems in mind
- **CMake build system** - includes code generation scripts via Python integrated into the build pipeline

## Plugin Interface

Every plugin implements a small, well-defined C interface:

```c
typedef int32_t (*PluginInjectDependency_Fn)(void *context, const char *interface_name, void *iface);
typedef int32_t (*PluginInit_Fn)(void *context);
typedef int32_t (*PluginShutdown_Fn)(void *context);

typedef struct PluginProvider
{
    const void *vtable;
    PluginInjectDependency_Fn inject_dependency;
    PluginInit_Fn init;
    PluginShutdown_Fn shutdown;
    uint64_t context_size;
} PluginProvider;
```

This keeps the ABI minimal and stable. Plugins declare their own dependencies. At compile time a python script reads a manifest.toml and creates a symbol to get this struct. An example of a manifest.toml.

```toml
interface_name = "gui_application"
interface_version = "1"
plugin_name = "default"
module_path = "${build_dir}/gui_application_default.dll"
supported_lifetimes = ["singleton"]
init = true

[dependencies]
logger = {}
window = {}
draw = {}
input = {}
```

Each plugin implements a vtable. An Interface struct holds both a reference to an opaque Context struct and a pointer to a defined vtable struct. For each vtable member a companion inline function is created for ease of use, borrowing from the abstraction system of Linux and Zephyr RTOS.

```c
#pragma once

#include <stdint.h>

#pragma pack(push, 8)

struct GuiApplicationContext;
struct WindowInterfaceCreateWindowOptions;

typedef struct GuiApplicationVtable
{
    int32_t (*setup)(struct GuiApplicationContext *context, struct WindowInterfaceCreateWindowOptions *create_window_options);
    int32_t (*run)(struct GuiApplicationContext *context);
} GuiApplicationVtable;

typedef struct GuiApplicationInterface {
    struct GuiApplicationContext *context;
    GuiApplicationVtable *vtable;
} GuiApplicationInterface;

#pragma pack(pop)

static inline int32_t gui_application_setup(GuiApplicationInterface *iface, struct WindowInterfaceCreateWindowOptions *create_window_options)
{
    return iface->vtable->setup(iface->context, create_window_options);
}

static inline int32_t gui_application_run(GuiApplicationInterface *iface)
{
    return iface->vtable->run(iface->context);
}

```

## Memory Model

Each plugin receives a `PluginContext` backed by a 1 KiB O(1) **slab allocator**. On plugin shutdown, the slab is freed in a single operation. If a plugin requires more than 1KiB memory, or has configurable dynamic memory, an allocation can be executed during initialization which has to be returned at shutdown. Application should be able to run for long unattended times, so fragmentation has to be kept to a minimum.

## Build Instructions

This framework requires CMake and Python 3 for the build system and compile-time code generation.

### Prerequisites
* CMake (3.23+)
* Python (3.7+)
* A C11-compatible compiler (MSVC, GCC, or Clang)

### Building via Command Line
This project uses `CMakePresets.json` to standardize builds. You can choose to build the plugins as either dynamic shared libraries (default) or static libraries.

1. Clone the repository:
   ```bash
   git clone https://github.com/mathijs28498/plugin-framework.git
   ```
2. Configure the project using one of the presets, for example:
   ```bash
   # For a gui application with dynamic shared libraries
   cmake --preset dev-gui-dynamic 
   
   # For a gui application with static libraries 
   cmake --preset dev-gui-static
   ```
3. Build the project:
   ```bash
   cmake --build build
   ```

### Building via VS Code
If you are using Visual Studio Code, simply open the project folder, ensure the **CMake Tools** extension is installed, and select a default preset provided by the `CMakePresets.json` file.

## Project Goals

This framework is one component of a larger planned personal ecosystem:

- **Plugin Framework** (this repo) - modular runtime plugin loading in C
- **ACID** - a custom programming language designed to transpile to C and LLVM IR
- **BASE** - a custom SQL database engine
- **BUFFER** - a custom text editor

The goal is a fully self-authored stack, from memory allocator to language runtime, built with an emphasis on understanding every layer from first principles.

## Design Principles

- No hidden allocations - all memory is explicit and caller-owned
- No heap allocations - all memory is allocated statically or using strategies like bump arenas
- No global state - everything lives in a context
- Explicit error handling - no exceptions, no silent failures
- Follows NASA's Power of Ten coding rules for reliability in constrained environments
- Very limited headers allowed inside other headers to decrease incremental compilations 

## Plugins

| Plugin                 | Status                    |
| ---------------------- | ------------------------- |
| environment            | ✅ Complete                |
| input                  | ✅ Complete                |
| logger                 | ✅ Complete                |
| time                   | ✅ Complete - Windows only |
| allocator              | ✅ Complete                |
| draw                   | 🔨 In Progress             |
| - draw_background      | 🔨 In Progress             |
| - draw_2d              | 🔨 In Progress             |
| - draw_3d              | 📋 Planned                 |
| - draw_voxel           | 📋 Planned                 |
| - draw_post_processing | 📋 Planned                 |
| gui_application        | 🔨 In Progress             |
| plugin_manager         | 🔨 In Progress             |
| render_graph           | 🔨 In Progress             |
| renderer               | 🔨 In Progress             |
| window                 | 🔨 In Progress             |
| ui                     | 📋 Planned                 |
| physics                | 📋 Planned                 |
| - 2d physics           | 📋 Planned                 |
| - 3d physics           | 📋 Planned                 |
| - voxel physics        | 📋 Planned                 |

## Allocator Progress

| Feature                     | Status     |
| --------------------------- | ---------- |
| 1KiB context slabs          | ✅ Complete |
| Permanent memory allocation | ✅ Complete |
| Buddy allocator             | ✅ Complete |

## Vulkan Backend Progress

| Feature                                            | Status        |
| -------------------------------------------------- | ------------- |
| Bootstrap & initialisation                         | ✅ Complete    |
| Pipeline creation                                  | ✅ Complete    |
| Descriptor set creation, allocation & updates      | ✅ Complete    |
| Image creation                                     | ✅ Complete    |
| Graceful resource destruction                      | ✅ Complete    |
| API-agnostic abstract handles with generation      | ✅ Complete    |
| Abstracted command buffer commands                 | ✅ Complete    |
| Swapchain resizing                                 | ✅ Complete    |
| Deferred destruction queues                        | ✅ Complete    |
| Compile-time automatic shader compilation (Python) | ✅ Complete    |
| Buffer creation & usage                            | ✅ Complete    |
| Immediate submission command buffer                | ✅ Complete    |
| Render graph with automatic pass management        | 🔨 In Progress |
| Compile-time shader reflection                     | 🔨 In Progress |
| `VK_ERROR_DEVICE_LOST` recovery                    | 📋 Planned     |

## Platform Targets

| Platform | Status        |
| -------- | ------------- |
| Windows  | ✅ Supported   |
| Linux    | 🔨 In Progress |
| MacOS    | 📋 Planned     |
| Web      | 📋 Planned     |
| Embedded | 📋 Planned     |

## License
This project is licensed under the [MIT License](LICENSE).
