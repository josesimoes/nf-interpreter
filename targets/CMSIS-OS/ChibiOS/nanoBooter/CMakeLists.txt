# //
# // Copyright (c) 2017 The nanoFramework project contributors
# // See LICENSE file in the project root for full license information.
# //

# append target ChibiOS nanoBooter source files 
list(APPEND TARGET_CHIBIOS_NANOBOOTER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/WireProtocol_App_Interface.c")
list(APPEND TARGET_CHIBIOS_NANOBOOTER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/WireProtocol_MonitorCommands.c")
# make var global
set(TARGET_CHIBIOS_NANOBOOTER_SOURCES ${TARGET_CHIBIOS_NANOBOOTER_SOURCES} CACHE INTERNAL "make global")


# append target ChibiOS nanoBooter include directory
list(APPEND TARGET_CHIBIOS_NANOBOOTER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
# make var global
set(TARGET_CHIBIOS_NANOBOOTER_INCLUDE_DIRS ${TARGET_CHIBIOS_NANOBOOTER_INCLUDE_DIRS} CACHE INTERNAL "make global")
