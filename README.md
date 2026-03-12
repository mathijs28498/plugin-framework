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

### 1
- [ ] Fix all todos

### 2
- [ ] Add sub interfaces for logic and draw
- [ ] Add loops in my .in template files 

### 3
- [ ] Figure out if exported declarations can also be supported dynamically where they are using dllexport so the framework can use them dynamically or if theyre purely for plugin_manager
- [ ] Get rid of all static, non relative paths
- [ ] Add configurations to plugins
- [ ] Create proper templating stuff in python with loops and everything
  - [ ] Look into string.Template
- [ ] Add logging to a file
- [ ] Add error handling in cmake
- [ ] Look into multiple threads
- [ ] Look into singleton vs scoped vs transient
    - [ ] This can now be described in the toml with different allocation methods for singleton vs the others possibly
- [ ] Create memory management plugins/interfaces
- [ ] figure out how to properly structure the code to be used outside this repo

### 4
- [ ] Create syntax highlighting for .in files
  - [ ] Make the lsp work, but add support for @...@ syntax
  - [ ] Maybe even allow for strongly typed inclusions
  - [ ] Add looping
- [ ] Create go to definition in templating in python somehow