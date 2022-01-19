#ifndef RYME_JSON_HPP
#define RYME_JSON_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Math.hpp>

#include <Ryme/ThirdParty/nlohmann_json.hpp>

namespace ryme {

typedef nlohmann::basic_json<
    std::map,       // ObjectType
    std::vector,    // ArrayType
    std::string,    // StringType
    bool,           // BooleanType
    int,            // NumberIntegerType
    unsigned,       // NumberUnsignedType
    float           // NumberFloatType
    > JSON;

inline void from_json(const JSON& array, Vec2& value)
{
    if (array.is_array() && array.size() == 2) {
        value = Vec2(
            array[0].get<float>(),
            array[1].get<float>()
        );
    }
}

inline void from_json(const JSON& array, Vec3& value)
{
    if (array.is_array() && array.size() == 3) {
        value = Vec3(
            array[0].get<float>(),
            array[1].get<float>(),
            array[2].get<float>()
        );
    }
}

inline void from_json(const JSON& array, Vec4& value)
{
    if (array.is_array() && array.size() == 4) {
        value = Vec4(
            array[0].get<float>(),
            array[1].get<float>(),
            array[2].get<float>(),
            array[3].get<float>()
        );
    }
}

inline void from_json(const JSON& array, Quat& value)
{
    if (array.is_array() && array.size() == 4) {
        value = Quat(
            array[3].get<float>(),
            array[0].get<float>(),
            array[1].get<float>(),
            array[2].get<float>()
        );
    }
}

} // namespace ryme

#endif // RYME_JSON_HPP