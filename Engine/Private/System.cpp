#include <Ryme/System.hpp>
#include <Ryme/World.hpp>

namespace ryme {

void System::Update()
{
    OnUpdate();
}

void System::Render()
{
    OnRender();
}

} // namespace ryme