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
#include <Ryme/Version.hpp>

namespace ryme {

struct RYME_API InitInfo
{
    Script::InitInfo Script;

    Graphics::InitInfo Graphics;

}; // struct InitInfo

RYME_API
void Init(InitInfo initInfo = {});

RYME_API
void Term();

} // namespace ryme

#endif // RYME_HPP