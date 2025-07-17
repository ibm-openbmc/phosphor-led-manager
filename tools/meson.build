tool_sources = ['main.cpp']

led_tools_exe = executable(
    'led-tool',
    tool_sources,
    dependencies: [sdbusplus_dep],
    include_directories: ['.'],
    install: true,
    install_dir: get_option('bindir')
)