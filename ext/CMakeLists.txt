if (BUILD_GIT)
    macro(install)
    endmacro()
    set(BUILD_CLAR OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
    set(USE_SSH OFF CACHE BOOL "" FORCE)
    set(USE_HTTPS OFF CACHE BOOL "" FORCE)
    set(USE_GSSAPI OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(libgit2)
    link_directories(${PROJECT_SOURCE_DIR}/ext/libgit2)
    macro(install)
        _install(${ARGN})
    endmacro()
endif()

include_directories(${PROJECT_SOURCE_DIR}/ext)

add_library(ext pdjson stretchy_buffer)

if (BUILD_GIT)
    target_link_libraries(ext git2)
endif()