#include "Common/ShaderCompiler.h"
#include <iostream>

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

    auto printDiagnostics = [](Slang::ComPtr<slang::IBlob>& diag, const char* stage)
    {
        if (diag && diag->getBufferSize() > 0)
        {
            std::cerr << "[Slang][" << stage << "]: "
                      << (const char*)diag->getBufferPointer() << std::endl;
            diag = nullptr;
        }
    };

    Slang::ComPtr<slang::IBlob> diagnostics;

    slang::IModule* module = session->loadModule(source.pPath, diagnostics.writeRef());
    printDiagnostics(diagnostics, "loadModule");

    if (!module)
    {
        std::cerr << "[Slang] Failed to load module: " << source.pPath << std::endl;
        return {};
    }

    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("main", entryPoint.writeRef());

    if (!entryPoint)
    {
        std::cerr << "[Slang] Entry point 'main' not found in: " << source.pPath << std::endl;
        return {};
    }

    slang::IComponentType* components[] = { module, entryPoint };
    Slang::ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(components, 2, program.writeRef(), diagnostics.writeRef());
    printDiagnostics(diagnostics, "createComposite");

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    program->link(linkedProgram.writeRef(), diagnostics.writeRef());
    printDiagnostics(diagnostics, "link");

    Slang::ComPtr<slang::IBlob> spirvBlob;
    SlangResult res = linkedProgram->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnostics.writeRef());
    printDiagnostics(diagnostics, "getEntryPointCode");

    if (SLANG_FAILED(res) || !spirvBlob)
    {
        std::cerr << "[Slang] Compilation failed for: " << source.pPath << std::endl;
        return {};
    }

    const uint32_t* ptr  = static_cast<const uint32_t*>(spirvBlob->getBufferPointer());
    const size_t    size = spirvBlob->getBufferSize() / sizeof(uint32_t);

    return std::vector<uint32_t>(ptr, ptr + size);
}

}