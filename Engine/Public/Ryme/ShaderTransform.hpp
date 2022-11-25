#ifndef RYME_SHADER_TRANSFORM_HPP
#define RYME_SHADER_TRANSFORM_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>

namespace ryme {

struct RYME_API ShaderTransform
{
public:

    static inline const uint32_t Binding = 1;

    // The Model Matrix
    alignas(64) Mat4 Model;
    
    // The View Matrix
    alignas(64) Mat4 View;
    
    // The Projection Matrix
    alignas(64) Mat4 Projection;

    // Projection * View * Model    
    alignas(64) Mat4 MVP;

    inline void UpdateMVP()
    {
        MVP = Projection * View * Model;
    }

}; // struct ShaderTransform

static_assert(
    sizeof(ShaderTransform) == 256,
    "sizeof(ShaderTransform) does not match GLSL layout std140"
);

} // namespace ryme

#endif // RYME_SHADER_TRANSFORM_HPP