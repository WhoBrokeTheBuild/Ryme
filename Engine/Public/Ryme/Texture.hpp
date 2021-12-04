#ifndef RYME_TEXTURE_HPP
#define RYME_TEXTURE_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Asset.hpp>
#include <Ryme/Path.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Texture : public Asset
{
public:

    Texture(const Path& path, bool search = true);

    bool LoadFromFile(const Path& path, bool search = true);

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

    inline vk::ImageView& GetVkImageView() {
        return _vkImageView;
    }

private:

    Path _path;

    vk::Image _vkImage;

    vma::Allocation _vmaAllocation;

    vk::ImageView _vkImageView;

}; // class Texture

} // namespace ryme

#endif // RYME_TEXTURE_HPP