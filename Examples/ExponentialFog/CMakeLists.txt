file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(ExponentialFog ${SOURCES})

target_link_libraries(ExponentialFog
    PRIVATE
        ExamplesCommon
)