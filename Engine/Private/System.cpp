#include <Ryme/System.hpp>

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