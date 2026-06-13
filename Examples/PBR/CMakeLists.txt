file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(PBR ${SOURCES})

target_link_libraries(PBR
    PRIVATE
        ExamplesCommon
)