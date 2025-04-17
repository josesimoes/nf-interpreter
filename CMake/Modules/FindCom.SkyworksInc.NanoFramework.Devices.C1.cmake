#
# Copyright (c) .NET Foundation and Contributors
# See LICENSE file in the project root for full license information.
#

# native code directory
set(BASE_PATH_FOR_THIS_MODULE ${BASE_PATH_FOR_CLASS_LIBRARIES_MODULES}/Com.SkyworksInc.NanoFramework.Devices.C1)


# set include directories
list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/CLR/Core)
list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/CLR/Include)
list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/HAL/Include)
list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/PAL/Include)
list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS ${BASE_PATH_FOR_THIS_MODULE})
list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/Com.SkyworksInc.NanoFramework.Devices.C1)


# source files
set(Com.SkyworksInc.NanoFramework.Devices.C1_SRCS

    Com_SkyworksInc_NanoFramework_Devices_C1.cpp
    Com_SkyworksInc_NanoFramework_Devices_C1_C1Instruction.cpp
    Com_SkyworksInc_NanoFramework_Devices_C1_Com_SkyworksInc_NanoFramework_Devices_C1_C1Bus_mshl.cpp
    Com_SkyworksInc_NanoFramework_Devices_C1_Com_SkyworksInc_NanoFramework_Devices_C1_C1Bus.cpp

)

foreach(SRC_FILE ${Com.SkyworksInc.NanoFramework.Devices.C1_SRCS})

    set(Com.SkyworksInc.NanoFramework.Devices.C1_SRC_FILE SRC_FILE-NOTFOUND)

    find_file(Com.SkyworksInc.NanoFramework.Devices.C1_SRC_FILE ${SRC_FILE}
        PATHS
	        ${BASE_PATH_FOR_THIS_MODULE}
	        ${TARGET_BASE_LOCATION}
            ${PROJECT_SOURCE_DIR}/src/Com.SkyworksInc.NanoFramework.Devices.C1

	    CMAKE_FIND_ROOT_PATH_BOTH
    )

    if (BUILD_VERBOSE)
        message("${SRC_FILE} >> ${Com.SkyworksInc.NanoFramework.Devices.C1_SRC_FILE}")
    endif()

    list(APPEND Com.SkyworksInc.NanoFramework.Devices.C1_SOURCES ${Com.SkyworksInc.NanoFramework.Devices.C1_SRC_FILE})

endforeach()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Com.SkyworksInc.NanoFramework.Devices.C1 DEFAULT_MSG Com.SkyworksInc.NanoFramework.Devices.C1_INCLUDE_DIRS Com.SkyworksInc.NanoFramework.Devices.C1_SOURCES)
