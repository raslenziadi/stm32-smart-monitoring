cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(stm32_smart_monitoring)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensors
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alarms
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulator
    ${CMAKE_CURRENT_SOURCE_DIR}/src/monitoring
    ${CMAKE_CURRENT_SOURCE_DIR}/src/diagnostics
    ${CMAKE_CURRENT_SOURCE_DIR}/src/output
)

target_sources(app PRIVATE
    src/main.c
    src/sensors/sensor_manager.c
    src/alarms/alarm_manager.c
    src/simulator/sensor_simulator.c
    src/monitoring/monitor_manager.c
    src/diagnostics/diagnostics.c
    src/output/output_manager.c
)
