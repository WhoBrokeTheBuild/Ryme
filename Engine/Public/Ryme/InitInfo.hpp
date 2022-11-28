#ifndef RYME_INIT_INFO_HPP
#define RYME_INIT_INFO_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Color.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Version.hpp>

namespace ryme {

struct RYME_API InitInfo
{
    String ApplicationName;

    Version ApplicationVersion;

    String WindowTitle = RYME_PROJECT_NAME " (" RYME_VERSION_STRING ")";

    Vec2i WindowSize = { 640, 480 };

    Color ClearColor = Color::CornflowerBlue;

}; // struct InitInfo

} // namespace ryme

#endif // RYME_INIT_INFO_HPP