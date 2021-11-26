#include <Ryme/Ryme.hpp>

namespace ryme {

bool _isRunning = false;

String _applicationName;

Version _applicationVersion;

RYME_API
void Init(InitInfo initInfo /*= {}*/)
{
    RYME_BENCHMARK_START();

    _applicationName = initInfo.ApplicationName;
    _applicationVersion = initInfo.ApplicationVersion;

    Log(RYME_ANCHOR, "Ryme Version: {}", ryme::GetVersion());
    
    Log(RYME_ANCHOR, "{{fmt}} Version: {}.{}.{}",
        FMT_VERSION / 10000,
        (FMT_VERSION % 10000) / 100,
        (FMT_VERSION % 10000) % 100);

    Log(RYME_ANCHOR, "GLM Version: {}.{}.{}.{}",
        GLM_VERSION_MAJOR,
        GLM_VERSION_MINOR,
        GLM_VERSION_PATCH,
        GLM_VERSION_REVISION);

    Script::Init();

    Graphics::Init(initInfo.WindowTitle, initInfo.WindowSize);

    RYME_BENCHMARK_END();
}

RYME_API
void Term()
{
    RYME_BENCHMARK_START();

    Graphics::Term();

    Script::Term();
    
    RYME_BENCHMARK_END();
}

RYME_API
void Run()
{
    _isRunning = true;

    SDL_Event e;
    while (_isRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                _isRunning = false;
            }
        }

    }

    _isRunning = false;
}

RYME_API
bool IsRunning()
{
    return _isRunning;
}

RYME_API
void SetRunning(bool isRunning)
{
    _isRunning = isRunning;
}

RYME_API
Version GetVersion()
{
    return Version(
        RYME_VERSION_MAJOR,
        RYME_VERSION_MINOR,
        RYME_VERSION_PATCH
    );
}

RYME_API
String GetApplicationName()
{
    return _applicationName;
}

RYME_API
Version GetApplicationVersion()
{
    return _applicationVersion;
}

} // namespace ryme
