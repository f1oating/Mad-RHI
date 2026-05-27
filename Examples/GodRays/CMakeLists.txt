file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(GodRays ${SOURCES})

target_link_libraries(GodRays
    PRIVATE
        ExamplesCommon
)