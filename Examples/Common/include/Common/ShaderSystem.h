#pragma once

#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>
#include <functional>
#include <filesystem>

namespace mad::common {

class ShaderSystem
{
struct WatchedFile
{
    std::filesystem::path FilePath;
    std::filesystem::file_time_type LastWrite;
};

private:
    static Slang::ComPtr<slang::IGlobalSession> s_GlobalSession;
    static std::vector<std::pair<WatchedFile, std::function<void()>>> s_WatchedShaders;

public:    
    static void WatchShader(std::vector<const char*> shaders, std::function<void()> callback);

    static void Poll();

    static std::vector<uint32_t> Compile(const char* pPath);
    
};

}