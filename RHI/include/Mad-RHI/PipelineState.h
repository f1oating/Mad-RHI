#pragma once

#include "Mad-RHI/Common.h"

namespace mad::rhi {
    
enum class ShaderType
{
    VERTEX, 
    FRAGMENT,
};

class Shader : public Object
{
public:
    virtual ~Shader() = default;

    virtual ShaderType GetType() = 0;

};

}