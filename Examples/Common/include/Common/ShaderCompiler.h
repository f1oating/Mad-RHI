#pragma once

#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>

namespace mad::common {

struct ShaderSource
{
    const char* pPath;
};

class ShaderCompiler
{
private:
    static Slang::ComPtr<slang::IGlobalSession> s_GlobalSession;

public:
    static std::vector<uint32_t> Compile(ShaderSource source);
    
};

}