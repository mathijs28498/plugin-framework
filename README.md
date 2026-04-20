## TODO
- [x] Change api name to interface
- [x] Change plugin_manager_common.h to plugin_utils.h
- [x] Change plugin_manager_impl.h to plugin_sdk.h
- [x] Add retaining window at end of program exit
- [x] Remove test plugins 
- [x] Rename plugin_manager_api.h to plugin_framework.h
- [x] Remove _plugin suffix to plugin names
- [x] Properly destroy the window at end of execution
- [x] Make gui_application initialize the screen with its own method
- [x] Clean up internal apis and make them better
    - [x] Make internal apis proper plugins that are just linked statically
- [x] Add register plugin macro (rather than the define from now) goes in cmake now
- [x] Add shutdown methods to plugins
    - [x] Make the plugins shutdown in opposite order as initialization
- [x] Improve plugin_registry json
- [x] Add compile step to create plugin registry from json
- [x] Json registration of plugins
- [x] Remove build to get a clean build
- [x] Look into json parsing code generation inside of python rather than cmake
- [x] Separate array c generation and parsing
- [x] Split python code generator between generating compile time/ configure time (only make cmake at configure time)
- [x] Change char[] to char* where static chars are
- [x] Change json to toml for configs
- [x] Make logger and environment part of setup_context and make them statically loaded in the get_context method rather than at runtime in the init function
- [x] Remove the internal plugin name and change it to initialized plugins/core in python
- [x] Combine internal and external plugins in registry and have the toml decide if a plugin is purely static or not
- [x] Add static loading
    - [x] Figure out naming collisions
- [x] Fix static dependencies
- [x] Fix gen_plugin.py
- [x] Make interface context dependencies automatically added to context
- [x] Create inline functions for interfaces
- [x] Change register macros to a vtable (sigh)
- [x] Decouple vtable from interface
- [x] Make plugin manager into an interface
- [x] Create minimal plugin manager bootloader
- [x] Make plugin manager own the lifetime of context and interface
- [x] allow for plugins to create and destroy elements with malloc and free
- [x] Figure out how it binds the methods
- [x] Figure out how the plugin_manager does not get loaded to its scope
- [x] dont force "core" in manifests, have a compile time check that at least the logger and environment are added, if not add them as interfaces with the default
- [x] Look into using extern const PluginMetadata instead of a getter function for static analyzation
- [x] Figure out if exported declarations can also be supported dynamically where they are using dllexport so the framework can use them dynamically or if theyre purely for plugin_manager
- [x] Figure out how to handle capacity in dynamic plugin resolution
- [x] Implement scope shutdown

### 1
- [ ] Fix all todos

### 2
- [ ] Automatic python dependency tracking: https://www.perplexity.ai/search/i-have-this-cmake-code-message-s.fOhSFMS_Ssx0ouZ9YzjQ?sm=d
  - [ ] custom commands create their own dependency .d depfile
  - [ ] configure needs to add the cmake
    - [ ] 2 stage configure where the first stage creates the cmake file dependencies for the second stage
- [ ] Create depfile for python dependencies dynamically
- [ ] Make get in app fail if it is not explicitly requested plugin
- [ ] Make dependencies a struct created called PluginDependencies and add it like "PluginDependencies dep;"
- [ ] Change to glsl
- [ ] Link vulkan indirectly for faster performance: https://docs.vulkan.org/guide/latest/loader.html#loader
- [ ] Add project wide debug level
- [ ] Add allocator plugin
  - [ ] Add arena allocator with compile time settings for arenas
    - [ ] Have compile time settings to allow for x for y size arenas
    - [ ] Have the plugins using it register, then get back a handle to use
      - [ ] This registering should say if it needs global/temporary, and the amount and sizes
    - [ ] When allocating use the handle and receive an arena handle to use for allocation
    - [ ] Never give the raw data
    - [ ] When freeing either return the memory, or give back the handle (allow for both)
    - [ ] If someone that has 1 temporary handle registered asks for a second one it will error
    - [ ] Add memory allcoation tracker
  - [ ] Check for other allocation strategies that might be necessary
- [ ] Figure out valgrind
- [ ] Put dependencies into own struct called PluginDependencies created by the header and add this to the context struct with a small name
- [ ] Add a way to get custom text inserted in log statements for error codes in plugin_utils
- [ ] Add a composite Graphics/GPU interface that does the GPU initialization like vulkan. Then the renderer/ gpu_compute interface plugins can depend on this somehow. They need to depend specifically on the vulkan version however. This way you can specify a renderer and the composite one gets added right away.
- [ ] Add more robust checks for user facing plugins (like checking if context != NULL)
- [ ] Make static plugins work again
- [ ] Rename whole project to something acidy from framework
- [ ] Determine if logger macro should take the logger argument or not
- [ ] Make it so that plugins have access to the plugin_manager for scoped plugins for example (allow for scoped plugin dependencies)
- [ ] Change calloc for context to use a static allocator
  - [ ] Also make it work with scopes
- [ ] Figure out singleton and scoped and transient 
  - [ ] Create a macro for a scope that automatically destroys the scope (using the macro forloop trick)
- [ ] Create logger fallback for initial plugin_manager dependency resolver
- [ ] Make plugin_manager work static and dynamic (or always static)
- [ ] Add attachments that are defined in .toml
  - [ ] attachments are just interfaces
  - [ ] if attachment wants to be callable it has to have own interface and needs to embed the attachment interface
  - [ ] Create logger to file and console at same time
  - [ ] Allow for requested toml to specify attachments
  - [ ] Allow attachments to have same interface as the plugin
- [ ] Add compile time configurations to plugins via toml
  - [ ] figure out good structure for this
- [ ] add option for requested plugins to be created at statically
- [ ] Add attachments for logic and draw
- [ ] Add loops in my .in template files 
- [ ] Change plugin_name meaning to be the entire plugin_name (eg. renderer_vulkan)
  - [ ] Think of a new term for the former plugin_name (variant)
- [ ] Create different cmake target that houses the pm_interface headers and remove these from the general include
  - [ ] Think if the include target has the correct name
- [ ] Implement hash function for interface_name

### 3
- [ ] Add proper error numbers
- [ ] Get rid of all static, non relative paths
- [ ] Add configurations to plugins
- [ ] Create proper templating stuff in python with loops and everything
  - [ ] Look into string.Template
- [ ] Add logging to a file
- [ ] Add error handling in cmake
- [ ] Look into multiple threads
  - [ ] Add a job interface, this interface hides the type of system like pthreads vs fibers vs ...
  - [ ] Make singleton plugins either thread safe or not
  - [ ] If not thread safe singleton, error on grabbibng form other thread than creatred
- [ ] Look into singleton vs scoped vs transient
    - [ ] This can now be described in the toml with different allocation methods for singleton vs the others possibly
- [ ] Create memory management plugins/interfaces
- [ ] figure out how to properly structure the code to be used outside this repo
- [ ] Add docker
- [ ] Add ecs
- [ ] Add game objects scene logic

### 4
- [ ] Create syntax highlighting for .in files
  - [ ] Make the lsp work, but add support for @...@ syntax
  - [ ] Maybe even allow for strongly typed inclusions
  - [ ] Add looping
- [ ] Create go to definition in templating in python somehow
- [ ] Hot reloading

### 5
- [ ] Create C dutch to C transpiler in framework (for ACID lang)
  - [ ] Aslang :^ 

### 6
- [ ] Create custom vma allocator 
- [ ] ACID (Another C Inspired Design) programming language
- [ ] BASE (Barely Another Storage Engine) sql database, add ACID syntax with it
- [ ] BUFFER (...) text editor
- [ ] ... (...) plugin framework

# other
interface inline regex creator:
.* ([a-z].*) \(\*(.*)\)\(.* (.*)context \*context(.*);/static inline \1 _\2(\3 *iface \4\n{\n\treturn iface->\2(iface->context\4;\n}\n

calculate lines of code:
Get-ChildItem -Path . -Recurse -File | Where-Object {
($_.Extension -in @('.c', '.h', '.txt', '.cmake', '.py')) -and
($_.FullName -notmatch '\\lib\\' -or $_.FullName -match '\\lib\\plugin_manager_bootloader\\' -or $_.FullName -match '\\lib\\static_alloc\\')
} | Get-Content | Measure-Object -Line