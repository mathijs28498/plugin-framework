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

- [ ] Fix all todos

- [ ] Change char[] to char* where static chars are
- [ ] Add configurations to plugins
- [ ] Add sub interfaces for logic and draw
- [ ] Add conjoined plugin structure to json
- [ ] Remove build to get a clean build

- [ ] Add logging to a file
- [ ] Add static loading
    - [ ] Figure out naming collisions
- [ ] Look into multiple threads
- [ ] Look into singleton vs scoped vs transient
