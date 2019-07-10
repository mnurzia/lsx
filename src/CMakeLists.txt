include_directories(${PROJECT_SOURCE_DIR}/ext)

if (BUILD_GIT)
    include_directories(${PROJECT_SOURCE_DIR}/ext/libgit2/include)
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

add_executable(
    lsx
    ls_color
    ls_dir
    ls_dir_table
    ls_format
    ls_main
    ls_options
    ls_preproc
    ls_printer
    ls_state
    ls_theme
    ls_theme_parse
    ls_uid_gid
)

if(MSVC)
    target_compile_options(lsx PRIVATE /W4 /WX)
else()
    target_compile_options(lsx PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

target_link_libraries(lsx ext)

install(TARGETS lsx DESTINATION bin)