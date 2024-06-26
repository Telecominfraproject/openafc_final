# Allow includes relative to this "src" directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# Allow includes for generated headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_AUTOMOC ON)

# Helper functions for all target types
include(srcfunctions)
set(TARGET_LIBS "")
set(TARGET_BINS "")
set(TARGET_SBINS "")

# Build in selected sub-directories
if ("RatapiDebug" IN_LIST OPENAFC_BUILD_TYPE OR
    "RatapiRelease" IN_LIST OPENAFC_BUILD_TYPE)
    add_subdirectory(ratapi)
endif()
if ("WebDebug" IN_LIST OPENAFC_BUILD_TYPE OR
    "WebRelease" IN_LIST OPENAFC_BUILD_TYPE)
    add_subdirectory(web)
endif()
if ("EngineDebug" IN_LIST OPENAFC_BUILD_TYPE OR
    "EngineRelease" IN_LIST OPENAFC_BUILD_TYPE)
    message(STATUS "Make visible")
    add_subdirectory(ratcommon)
    add_subdirectory(afclogging)
    add_subdirectory(afcsql)
    if(BUILD_AFCENGINE)
        add_subdirectory(afc-engine)
    endif(BUILD_AFCENGINE)
endif()
if ("Ulsprocessor" IN_LIST OPENAFC_BUILD_TYPE)
    add_subdirectory(coalition_ulsprocessor)
    add_subdirectory(ratapi)
endif()

# Make visible to project scope
set(TARGET_LIBS "${TARGET_LIBS}" PARENT_SCOPE)
