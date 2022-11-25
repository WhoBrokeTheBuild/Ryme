#include <Ryme/Ryme.hpp>

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
    }
    catch (const std::exception& e) {
        Log("Exception", "{}", e.what());
    }
    
    Term();

    fflush(stdout);

    return 0;
}