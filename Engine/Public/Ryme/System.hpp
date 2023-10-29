#ifndef RYME_SYSTEM_HPP
#define RYME_SYSTEM_HPP

#include <Ryme/Config.hpp>
#include <Ryme/NonCopyable.hpp>

namespace ryme {

class RYME_API System : public NonCopyable
{
public:

    System() = default;
    
    virtual ~System() = default;

    

private:

}; // class System

} // namespace ryme

#endif // RYME_SYSTEM_HPP