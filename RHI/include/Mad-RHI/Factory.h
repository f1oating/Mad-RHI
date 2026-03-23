#pragma once

namespace mad::rhi {

enum class FactoryBackend
{
    Vulkan
};

struct FactoryInitInfo
{
    FactoryBackend Backend;
    const char* pAppName;
    const char* pEngineName;
};

class Factory
{
protected:
    static Factory* s_Factory;
    static bool s_IsInitialized;

public:
    virtual ~Factory() = default;

    static void Init(FactoryInitInfo& info);
    static void Shutdown();

    static Factory* GetInstance();

};

}