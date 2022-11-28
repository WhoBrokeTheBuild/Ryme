#ifndef RYME_HPP
#define RYME_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Color.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/InitInfo.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Script.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Transform.hpp>
#include <Ryme/UTF.hpp>
#include <Ryme/Version.hpp>

namespace ryme {

RYME_API
void Init(const InitInfo& initInfo = {});

RYME_API
void Term();

RYME_API
void Run();

RYME_API
bool IsRunning();

RYME_API
void SetRunning(bool isRunning);

RYME_API
Version GetVersion();

RYME_API
String GetApplicationName();

RYME_API
Version GetApplicationVersion();

} // namespace ryme

#endif // RYME_HPP