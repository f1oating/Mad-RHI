cmake_minimum_required(VERSION 3.25)
project(Mad-RHI VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

if(WIN32)
    include(FetchContent)
    FetchContent_Declare(
        agility_sdk
        URL "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.619.1"
    )
    FetchContent_MakeAvailable(agility_sdk)

    add_library(D3D12AgilitySDK INTERFACE)
    target_include_directories(D3D12AgilitySDK INTERFACE
        ${agility_sdk_SOURCE_DIR}/build/native/include
    )
endif()

add_subdirectory(thirdparty/glm)

set(LIBC_HAS_ITOA "" CACHE INTERNAL "Have symbol itoa" FORCE)
set(SDL_DISABLE_MSVCRT ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/SDL)

set(SLANG_LIB_TYPE STATIC CACHE STRING "" FORCE)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/slang)

set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/SPIRV-Reflect)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XCB_KHR)
endif()
add_subdirectory(thirdparty/volk)

add_subdirectory(RHI)
add_subdirectory(Examples)