#include <Ryme/Ryme.hpp>

#include <Ryme/World.hpp>

using namespace ryme;

int main(int argc, char ** argv)
{
    try {
        Init({
            .ApplicationName = DEMO_NAME,
            .ApplicationVersion = GetVersion(),
            .WindowTitle = DEMO_NAME " (" RYME_VERSION_STRING ")",
            .WindowSize = { 1024, 768 },
        });
        
        py::eval_file("Assets/Test.py");
        fflush(stdout);

        Run();

        Term();
    }
    catch (const std::exception& e) {
        Log(RYME_ANCHOR, "Exception {}", e.what());
    }

    fflush(stdout);

    return 0;
}