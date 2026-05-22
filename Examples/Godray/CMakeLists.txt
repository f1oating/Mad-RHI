file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Godray ${SOURCES})

target_link_libraries(Godray
    PRIVATE
        ExamplesCommon
)