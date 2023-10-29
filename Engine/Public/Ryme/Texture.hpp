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

    Texture(const Path& path, vk::SamplerCreateInfo samplerCreateInfo = {}, bool search = true);

    virtual ~Texture();

    bool LoadFromFile(const Path& path, vk::SamplerCreateInfo samplerCreateInfo = {}, bool search = true);

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

    inline vk::ImageView& GetImageView() {
        return _imageView;
    }

private:

    Path _path;

    vk::SamplerCreateInfo _samplerCreateInfo;

    vk::Image _image = nullptr;

    VmaAllocation _allocation = nullptr;

    vk::ImageView _imageView = nullptr;

    vk::Sampler _sampler = nullptr;

}; // class Texture

} // namespace ryme

#endif // RYME_TEXTURE_HPP