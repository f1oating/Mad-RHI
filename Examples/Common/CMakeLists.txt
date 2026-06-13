file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")

add_library(ExamplesCommon STATIC ${COMMON_SOURCES})

target_include_directories(ExamplesCommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ExamplesCommon
    PUBLIC
        Mad-RHI
        SDL3::SDL3
        slang
        glm::glm

        $<$<PLATFORM_ID:Linux>:X11-xcb>
)