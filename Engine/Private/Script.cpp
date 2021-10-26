#include <Ryme/Script.hpp>

namespace ryme {

namespace Script {

RYME_API
void Init(InitInfo initInfo)
{
    py::initialize_interpreter();
    
    py::object pyVersion = py::module_::import("sys").attr("version");
    ryme::Log(RYME_ANCHOR, "Python Version: {}", py::str(pyVersion));
}

RYME_API
void Term()
{
    py::finalize_interpreter();
}

} // namespace Script

} // namespace ryme
