file(GLOB_RECURSE MAD_RHI_SOURCES
    "src/*.cpp"
    "src/*.c"
)

add_library(Mad-RHI SHARED ${MAD_RHI_SOURCES})

target_include_directories(Mad-RHI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/thirdparty/VMA/include
        $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/thirdparty/D3D12MA/include>
)

target_sources(Mad-RHI PRIVATE
    $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/thirdparty/D3D12MA/src/D3D12MemAlloc.cpp>
)

target_link_libraries(Mad-RHI
    PRIVATE
        volk
        spirv-reflect-static
        Vulkan::Headers
        $<$<PLATFORM_ID:Windows>:d3d12>
        $<$<PLATFORM_ID:Windows>:dxgi>
        $<$<PLATFORM_ID:Windows>:dxguid>
        $<$<PLATFORM_ID:Windows>:D3D12AgilitySDK>
)

target_compile_definitions(Mad-RHI
    PRIVATE
        VK_NO_PROTOTYPES
        MAD_RHI_BUILD_DLL
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
)
