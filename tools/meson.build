tool_sources = [
    'tools-main.cpp',
]
configuration_inc = include_directories('.')

led_tools_exe = executable(
                'led-tool',
                tool_sources,
                dependencies: [sdbusplus_dep],
                include_directories : configuration_inc,
                install: true,
                )
