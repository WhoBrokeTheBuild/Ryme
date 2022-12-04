#include <Ryme/Mesh.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Primitive.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Vertex.hpp>
#include <Ryme/UTF.hpp>

#include <cstdio>

namespace ryme {

bool Mesh::LoadOBJ(const Path& path, bool search)
{
    // https://github.com/blender/blender-addons/blob/master/io_scene_obj/export_obj.py

    struct _Material
    {
        String Name;

        Vec3 BaseColorFactor = Vec3(1.0f);  // Kd
        Vec3 EmissiveFactor = Vec3(0.0f);   // Ke
        float MetallicFactor = 0.0f;        // Ka (repeated)
        float SpecularFactor = 0.5f;        // Ks (repeated)
        float IOR = 1.5f;                   // Ni
        float RoughnessFactor = 0.5f;       // Ns = ((1-roughness) * 30)^2
        float AlphaScale = 1.0f;            // d
        int Illum = 2;                      // illum

        Path AlphaMap;              // map_d
        Path BaseColorMap;          // map_Kd
        Path EmissionMap;           // map_Ke
        Path MetallicMap;           // refl, map_refl
        Path NormalMap;             // map_bump, map_Bump
        Path RoughnessMap;          // map_Ns
        Path SpecularMap;           // map_Ks

        _Material(StringView name)
            : Name(name)
        { }
    };

    struct _Object
    {
        String Name;
        
        String MaterialName;

        List<Vertex> VertexList;
        
        _Object(StringView name)
            : Name(name)
        { }
    };

    FILE * objFile = nullptr;
    Path fullPath = path;

    if (search) {
        for (const auto& assetPath : GetAssetPathList()) {
            fullPath = assetPath / path;

            objFile = fopen(fullPath.ToCString(), "rt");
            if (objFile) {
                break;
            }
        }
    }
    else {
        objFile = fopen(fullPath.ToCString(), "rt");
    }

    if (!objFile) {
        return false;
    }

    _path = fullPath;

    List<_Material> materialList;
    List<_Object> objectList;

    List<Vec3> positionList;
    List<Vec3> normalList;
    List<Vec2> texCoordList;

    int scanned = 0;

    int objLineNumber = -1;

    auto objError = [&]() {
        throw Exception("Malformed OBJ file at '{}:{}'", fullPath, objLineNumber);
    };
    
    Path mtlPath;
    int mtlLineNumber;

    auto mtlError = [&]() {
        throw Exception("Malformed MTL file at '{}:{}'", mtlPath, mtlLineNumber);
    };

    List<char> buffer(1024);
    while (fgets(buffer.data(), buffer.size(), objFile) != nullptr) {
        ++objLineNumber;

        StringView line(buffer.data());

        size_t comment = line.find('#');
        if (comment != StringView::npos) {
            line = line.substr(0, comment);
        }

        line = Strip(line);
        
        size_t firstWhitespace = StringView::npos;
        for (size_t i = 0; i < line.size(); ++i) {
            if (std::isspace(line[i])) {
                firstWhitespace = i;
                break;
            }
        }

        if (firstWhitespace == StringView::npos) {
            continue;
        }

        StringView key = line.substr(0, firstWhitespace);
        StringView value = StripLeft(line.substr(firstWhitespace));

        if (key == "v") {
            auto& position = positionList.emplace_back();

            scanned = sscanf(value.data(), "%f %f %f",
                &position.x, &position.y, &position.z
            );

            if (scanned != 3) {
                objError();
            }
        }
        else if (key == "vn") {
            auto& normal = normalList.emplace_back();

            scanned = sscanf(value.data(), "%f %f %f",
                &normal.x, &normal.y, &normal.z
            );

            if (scanned != 3) {
                objError();
            }
        }
        else if (key == "vt") {
            auto& texCoord = texCoordList.emplace_back();

            scanned = sscanf(value.data(), "%f %f",
                &texCoord.s, &texCoord.t
            );

            if (scanned != 2) {
                objError();
            }
        }
        else if (key == "f") {
            bool hasNormal = false;
            bool hasTexCoord = false;

            int positionIndex[3];
            int normalIndex[3];
            int texCoordIndex[3];

            if (StringContains(value, "//")) {
                scanned = sscanf(value.data(), "%d//%d %d//%d %d//%d",
                    &positionIndex[0], &normalIndex[0],
                    &positionIndex[1], &normalIndex[1],
                    &positionIndex[2], &normalIndex[2]
                );

                if (scanned != 6) {
                    objError();
                }

                hasNormal = true;
            }
            else if (StringContains(value, "/")) {
                scanned = sscanf(value.data(), "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &positionIndex[0], &texCoordIndex[0], &normalIndex[0],
                    &positionIndex[1], &texCoordIndex[1], &normalIndex[1],
                    &positionIndex[2], &texCoordIndex[2], &normalIndex[2]
                );

                if (scanned != 9) {
                    objError();
                }

                hasNormal = true;
                hasTexCoord = true;
            }
            else {
                scanned = sscanf(value.data(), "%d %d %d",
                    &positionIndex[0],
                    &positionIndex[1],
                    &positionIndex[2]
                );

                if (scanned != 3) {
                    objError();
                }
            }

            for (unsigned i = 0; i < 3; ++i) {
                
                if (positionIndex[i] < 0) {
                    positionIndex[i] += positionList.size();
                }
                
                if (hasNormal && normalIndex[i] < 0) {
                    normalIndex[i] += normalList.size();
                }
                
                if (hasTexCoord && texCoordIndex[i] < 0) {
                    texCoordIndex[i] += texCoordList.size();
                }

                objectList.back().VertexList.emplace_back(
                    Vertex{
                        .Position = Vec4(positionList[positionIndex[i] - 1], 1.0f),
                        .Normal = (
                            hasNormal
                            ? Vec4(normalList[normalIndex[i] - 1], 1.0f)
                            : Vec4(0.0f, 0.0f, 0.0f, 1.0f)
                        ),
                        .TexCoord = (
                            hasTexCoord
                            ? texCoordList[texCoordIndex[i] - 1]
                            : Vec2()
                        ),
                    }
                );
            }
        }
        else if (key == "o") {
            objectList.emplace_back(value);
        }
        else if (key == "s") {
            // Smooth Shading toggle, not implemented
        }
        else if (key == "illum") {
            // Illumination mode
        }
        else if (key == "usemtl") {
            objectList.back().MaterialName = value;
        }
        else if (key == "mtllib") {
            mtlPath = value;
            if (mtlPath.IsRelative()) {
                mtlPath = fullPath.GetParentPath() / mtlPath;
            }

            FILE * mtlFile = fopen(mtlPath.ToCString(), "rt");
            if (!mtlFile) {
                throw Exception("Failed to load MTL file '{}'", mtlPath);
            }

            mtlLineNumber = -1;

            while (fgets(buffer.data(), buffer.size(), mtlFile) != nullptr) {
                ++mtlLineNumber;

                line = StringView(buffer.data());

                comment = line.find('#');
                if (comment != StringView::npos) {
                    line = line.substr(0, comment);
                }

                line = Strip(line);

                firstWhitespace = StringView::npos;
                for (size_t i = 0; i < line.size(); ++i) {
                    if (std::isspace(line[i])) {
                        firstWhitespace = i;
                        break;
                    }
                }

                if (firstWhitespace == StringView::npos) {
                    continue;
                }

                key = line.substr(0, firstWhitespace);
                value = StripLeft(line.substr(firstWhitespace));

                if (key == "newmtl") {
                    materialList.emplace_back(value);
                }
                else if (key == "Kd") {
                    Vec3& factor = materialList.back().BaseColorFactor;
                    scanned = sscanf(value.data(), "%f %f %f", &factor.r, &factor.g, &factor.b);

                    if (scanned != 3) {
                        mtlError();
                    }
                }
                else if (key == "Ke") {
                    Vec3& factor = materialList.back().EmissiveFactor;
                    scanned = sscanf(value.data(), "%f %f %f", &factor.r, &factor.g, &factor.b);

                    if (scanned != 3) {
                        mtlError();
                    }
                }
                else if (key == "Ka") {
                    float& factor = materialList.back().MetallicFactor;
                    scanned = sscanf(value.data(), "%f %*f %*f", &factor);

                    if (scanned != 1) {
                        mtlError();
                    }
                }
                else if (key == "Ks") {
                    float& factor = materialList.back().SpecularFactor;
                    scanned = sscanf(value.data(), "%f %*f %*f", &factor);

                    if (scanned != 1) {
                        mtlError();
                    }
                }
                else if (key == "Ni") {
                    float& ior = materialList.back().IOR;
                    scanned = sscanf(value.data(), "%f", &ior);

                    if (scanned != 1) {
                        mtlError();
                    }
                }
                else if (key == "Ns") {
                    float& factor = materialList.back().RoughnessFactor;
                    scanned = sscanf(value.data(), "%f", &factor);

                    // Blender encodes the Principled BSDF range [0.0, 1.0] into the OBJ Specular Exponent range [0.0, 1000.0]
                    factor = 1.0f - sqrtf(glm::clamp(factor, 0.0f, 1000.0f) / 1000.0f);

                    if (scanned != 1) {
                        mtlError();
                    }
                }
                else if (key == "d") {
                    float& factor = materialList.back().SpecularFactor;
                    scanned = sscanf(value.data(), "%f", &factor);

                    if (scanned != 1) {
                        mtlError();
                    }
                }
                else if (key == "d") {
                    int& illum = materialList.back().Illum;
                    scanned = sscanf(value.data(), "%d", &illum);

                    if (scanned != 1) {
                        mtlError();
                    }
                }
                else if (key == "map_d") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().AlphaMap = texturePath;
                }
                else if (key == "map_Kd") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().BaseColorMap = texturePath;
                }
                else if (key == "map_Ks") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().SpecularMap = texturePath;
                }
                else if (key == "map_Ke") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().EmissionMap = texturePath;
                }
                else if (key == "map_Ns") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().RoughnessMap = texturePath;
                }
                else if (key == "refl" || key == "map_refl") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().MetallicMap = texturePath;
                }
                else if (key == "map_Bump" || key == "map_bump") {
                    Path texturePath = value;
                    if (texturePath.IsRelative()) {
                        texturePath = mtlPath.GetParentPath() / texturePath;    
                    }

                    materialList.back().NormalMap = texturePath;
                }
                else {
                    objError();
                }
            }

            fclose(mtlFile);
            
            Log(RYME_ANCHOR, "Loaded '{}'", mtlPath);
        }
    }

    List<Primitive> primitiveList;

    for (auto& object : objectList) {
        primitiveList.emplace_back(Primitive{
            .VertexList = std::move(object.VertexList),
        });

        primitiveList.back().CalculateTangents();
    }

    fclose(objFile);
    
    Log(RYME_ANCHOR, "Loaded '{}'", _path);

    return true;
}

} // namespace ryme