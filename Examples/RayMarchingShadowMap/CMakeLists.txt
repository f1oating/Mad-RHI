file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(RayMarchingShadowMap ${SOURCES})

target_link_libraries(RayMarchingShadowMap
    PRIVATE
        ExamplesCommon
)