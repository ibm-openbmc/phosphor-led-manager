tool_sources = [
    'main.cpp',
]
configuration_inc = include_directories('.')

services = ['service/obmc-clear-all-fault-leds-and-remove-crit-association@.service',
            'service/obmc-set-guarded-frus-leds@.service']
            
systemd_system_unit_dir = dependency('systemd').get_variable(
        'systemdsystemunitdir')

install_data(services, install_dir: systemd_system_unit_dir)

led_tools_exe = executable(
                'led-tool',
                tool_sources,
                dependencies: [sdbusplus_dep],
                include_directories : configuration_inc,
                install: true,
                )