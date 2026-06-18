file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(PBR ${SOURCES})

if(WIN32)
    copy_agility_sdk_dlls(PBR)
endif()

target_link_libraries(PBR
    PRIVATE
        ExamplesCommon
)