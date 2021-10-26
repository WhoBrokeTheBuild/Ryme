#include <Ryme/Ryme.hpp>

int main(int argc, char ** argv)
{
    try {
        ryme::Init({
            .Script = {

            },
            .Graphics = {
                .WindowTitle = "HelloWorld",
                .WindowSize = { 1024, 768 },
            },
        });

        ryme::Run();

        ryme::Term();
    }
    catch (const std::exception& e) {
        ryme::Log(RYME_ANCHOR, "Exception {}", e.what());
    }

    return 0;
}