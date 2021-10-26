#include <Ryme/Ryme.hpp>

namespace ryme {

RYME_API
void Init(InitInfo initInfo)
{
    RYME_BENCHMARK_START();

    Log(RYME_ANCHOR, "Ryme Version: {}", ryme::GetVersion());

    Script::Init(initInfo.Script);
    Graphics::Init(initInfo.Graphics);
    
    Log(RYME_ANCHOR, "{{fmt}} Version: {}.{}.{}",
        FMT_VERSION / 10000,
        (FMT_VERSION % 10000) / 100,
        (FMT_VERSION % 10000) % 100);

    Log(RYME_ANCHOR, "GLM Version: {}.{}.{}.{}",
        GLM_VERSION_MAJOR,
        GLM_VERSION_MINOR,
        GLM_VERSION_PATCH,
        GLM_VERSION_REVISION);

    RYME_BENCHMARK_END();
}

RYME_API
void Term()
{

    Graphics::Term();
    Script::Term();
}

} // namespace ryme
