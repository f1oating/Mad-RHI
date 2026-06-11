file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Shadows ${SOURCES})

target_link_libraries(Shadows
    PRIVATE
        ExamplesCommon
)