#include <Ryme/Script.hpp>
#include <Ryme/Ryme.hpp>

PYBIND11_EMBEDDED_MODULE(ryme, m) {
    using namespace ryme;

    auto pyRymeAnchor = [&]() {
        py::object pyFrame = py::module::import("inspect").attr("currentframe")();
        py::str pyFilename = pyFrame.attr("f_code").attr("co_filename");

        return fmt::format(
            "{}:{}",
            Path(pyFilename.cast<StringView>()).GetFilename(),
            py::str(pyFrame.attr("f_lineno"))
        );
    };

    Math::ScriptInit(m);
    Version::ScriptInit(m);
    Path::ScriptInit(m);
    Graphics::ScriptInit(m);

    // m.def("Init", Init);
    // m.def("Term", Term);
    m.def("Run", Run);
    m.def("IsRunning", IsRunning);
    m.def("SetRunning", SetRunning);
    m.def("GetVersion", GetVersion);
    m.def("GetApplicationName", GetApplicationName);
    m.def("GetApplicationVersion", GetApplicationVersion);

    m.def("Log", 
        [&](py::args args) {
            if (args.size() == 1) {
                ryme::LogMessage(pyRymeAnchor(), py::str(args[0]).cast<StringView>());
            }
            else {
                String message;
                for (const auto& arg : args) {
                    message += py::str(arg).cast<StringView>();
                    message += ' ';
                }
                ryme::LogMessage(pyRymeAnchor(), message);
            }
        },
        py::doc("Log a message with the tag file:line"));
}

namespace ryme {

namespace Script {

RYME_API
void Init()
{
    py::initialize_interpreter();

    py::object pyVersion = py::module::import("sys").attr("version");
    ryme::Log(RYME_ANCHOR, "Python Version: {}", py::str(pyVersion));
}

RYME_API
void Term()
{
    py::finalize_interpreter();
}

} // namespace Script

} // namespace ryme
