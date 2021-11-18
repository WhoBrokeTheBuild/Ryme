#ifndef RYME_HPP
#define RYME_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Script.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Transform.hpp>
#include <Ryme/Version.hpp>

namespace ryme {

struct RYME_API InitInfo
{
    String ApplicationName;

    Version ApplicationVersion;

    String WindowTitle = RYME_PROJECT_NAME " (" RYME_VERSION_STRING ")";

    Vec2i WindowSize = { 640, 480 };

}; // struct InitInfo

RYME_API
void Init(InitInfo initInfo = {});

RYME_API
void Term();

RYME_API
void Run();

RYME_API
bool IsRunning();

RYME_API
void SetRunning(bool isRunning);

RYME_API
inline Version GetVersion();

RYME_API
inline String GetApplicationName();

RYME_API
inline Version GetApplicationVersion();

} // namespace ryme

#endif // RYME_HPP