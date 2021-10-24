#include <Ryme/Ryme.hpp>

namespace ryme {

RYME_API
Version GetVersion()
{
    return Version(RYME_VERSION_MAJOR, RYME_VERSION_MINOR, RYME_VERSION_PATCH);
}

} // namespace ryme
