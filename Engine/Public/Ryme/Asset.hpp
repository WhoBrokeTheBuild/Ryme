#ifndef RYME_ASSET_HPP
#define RYME_ASSET_HPP

#include <Ryme/Config.hpp>

class RYME_API Asset
{
public:

    virtual ~Asset()
    {
        Free();
    }

    virtual void Free() { }

    virtual bool IsLoaded()
    {
        return _isLoaded;
    }

    virtual bool Reload()
    {
        return true;
    }

    virtual bool CanReload()
    {
        return false;
    }

    explicit operator bool() const
    {
      return _isLoaded;
    }

    bool operator!() const
    {
      return !_isLoaded;
    }

protected:

    bool _isLoaded = false;

}; // class Asset

#endif // RYME_ASSET_HPP