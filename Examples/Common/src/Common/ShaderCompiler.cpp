#include "Common/ShaderCompiler.h"

namespace mad::common {

Slang::ComPtr<slang::IGlobalSession> ShaderCompiler::s_GlobalSession = nullptr;

std::vector<uint32_t> ShaderCompiler::Compile(ShaderSource source)
{
    if (!s_GlobalSession)
    {
        slang::createGlobalSession(s_GlobalSession.writeRef());
    }

    slang::SessionDesc sessionDesc{};
    slang::TargetDesc targetDesc{};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = s_GlobalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session;
    s_GlobalSession->createSession(sessionDesc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule(
        source.pPath,
        diagnostics.writeRef()
    );

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("main", entryPoint.writeRef());

    slang::IComponentType* components[] = { module, entryPoint };
    Slang::ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(components, 2, program.writeRef(), diagnostics.writeRef());

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    program->link(linkedProgram.writeRef(), diagnostics.writeRef());

    Slang::ComPtr<slang::IBlob> spirvBlob;
    linkedProgram->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnostics.writeRef());

    const uint32_t* ptr  = static_cast<const uint32_t*>(spirvBlob->getBufferPointer());
    const size_t    size = spirvBlob->getBufferSize() / sizeof(uint32_t);

    std::vector<uint32_t> spirvCode(ptr, ptr + size);

    return spirvCode;
}

}