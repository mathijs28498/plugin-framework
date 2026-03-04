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

### 1
- [ ] Fix all todos

### 2
- [ ] Change char[] to char* where static chars are
- [ ] Add configurations to plugins
- [ ] Add sub interfaces for logic and draw
- [ ] Add loops in my .in template files 
- [ ] Separate array c generation and parsing
- [ ] Split python code generator between generating compile time/ configure time (only make cmake at configure time)

### 3
- [ ] Create proper templating stuff with loops and everything
  - [ ] Look into string.Template
- [ ] Add logging to a file
- [ ] Add error handling in cmake
- [ ] Add error handling for each part of json parsing in cmake
- [ ] Add static loading
    - [ ] Figure out naming collisions
- [ ] Look into multiple threads
- [ ] Look into singleton vs scoped vs transient
