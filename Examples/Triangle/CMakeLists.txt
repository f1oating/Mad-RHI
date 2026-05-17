file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Triangle ${SOURCES})

target_link_libraries(Triangle
    PRIVATE
        ExamplesCommon
)