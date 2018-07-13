#include "Mesh.hpp"
#include <numeric>
#include <execution>
#include "Util.hpp"
#include "stb/stb_image.h"

Mesh::Mesh(aiMesh* assimpMesh, aiMaterial* assimpMat, const std::experimental::filesystem::path& rootPath)
{
    if (!assimpMesh->HasNormals() || !assimpMesh->HasFaces())
    {
        throw std::runtime_error("Mesh must have normals and faces");
    }

    // - - - VERTICES, NORMALS, UV_COORDS - - -

    std::thread vntThread([&]()
    {
        vertices.resize(assimpMesh->mNumVertices);
        normals.resize(assimpMesh->mNumVertices);
        uvs.resize(assimpMesh->mNumVertices);

#pragma omp parallel for
        for (int64_t i = 0; i < static_cast<int64_t>(assimpMesh->mNumVertices); ++i)
        {
            const auto aivec = assimpMesh->mVertices[i];
            vertices[i] = glm::vec4(aivec.x, aivec.y, aivec.z, 1.0f);

            const auto ainorm = assimpMesh->mNormals[i];
            normals[i] = glm::vec4(ainorm.x, ainorm.y, ainorm.z, 0.0f);

            if (assimpMesh->HasTextureCoords(0))
            {
                const aiVector3D aitex = assimpMesh->mTextureCoords[0][i];
                uvs[i] = glm::vec2(aitex.x, aitex.y);
            }
        }

        calculateBoundingBox();
    });

    // - - - I N D I C E S - - -

    std::thread indexThread([&]()
    {
        indices.resize(assimpMesh->mNumFaces * 3);

#pragma omp parallel for
        for (int64_t i = 0; i < static_cast<int64_t>(assimpMesh->mNumFaces); ++i)
        {
            const auto face = assimpMesh->mFaces[i];

            indices[i + 0] = face.mIndices[0];
            indices[i + 1] = face.mIndices[1];
            indices[i + 2] = face.mIndices[2];
        }
    });

    // - - - M A T E R I A L - - -



    aiString reltexPath;
    for (aiTextureType type : {aiTextureType_DIFFUSE, aiTextureType_OPACITY, aiTextureType_SPECULAR, aiTextureType_SHININESS, aiTextureType_NORMALS, aiTextureType_HEIGHT, aiTextureType_LIGHTMAP })
    {
        //has texture
        if (assimpMat->GetTextureCount(type) > 0)
        {
            assimpMat->GetTexture(type, 0, &reltexPath);
            auto absTexPath = std::experimental::filesystem::absolute(rootPath.parent_path() / std::experimental::filesystem::path(reltexPath.C_Str()));

            switch (type)
            {
            case aiTextureType_DIFFUSE: //albedo
                m_textures[aiTextureType_DIFFUSE] = std::make_shared<Texture>(absTexPath, 4);
                break;
            case aiTextureType_OPACITY: //albedo alpha
                if (assimpMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
                    copyToAlpha(absTexPath, m_textures[aiTextureType_DIFFUSE]);
                break;
            case aiTextureType_SPECULAR: //roughness
                m_textures[aiTextureType_SPECULAR] = std::make_shared<Texture>(absTexPath, 1);
                break;
            case aiTextureType_SHININESS: //metallic
                m_textures[aiTextureType_SHININESS] = std::make_shared<Texture>(absTexPath, 1);
                break;
            case aiTextureType_NORMALS: //normal
                m_textures[aiTextureType_NORMALS] = std::make_shared<Texture>(absTexPath, 4);
                break;
            case aiTextureType_HEIGHT: //height-to-normal or normal alpha
                if (assimpMat->GetTextureCount(aiTextureType_NORMALS) > 0)
                    copyToAlpha(absTexPath, m_textures[aiTextureType_NORMALS]);
                else
                    m_textures[aiTextureType_NORMALS] = generateNormalFromHeight(absTexPath);
                break;
            case aiTextureType_LIGHTMAP: //ambient occlusion
                m_textures[aiTextureType_LIGHTMAP] = std::make_shared<Texture>(absTexPath, 1);
                break;
            default:
                break;
            }
        }
        else //has no texture
        {
            switch (type)
            {
            case aiTextureType_DIFFUSE: //albedo
            {
                aiColor3D diffcolor(0.0f, 0.0f, 0.0f);
                assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffcolor);
                material.setColor(glm::vec4(diffcolor.r, diffcolor.g, diffcolor.b, 1.0f));
                break;
            }
            case aiTextureType_OPACITY: //albedo alpha
            {
                float op;
                assimpMat->Get(AI_MATKEY_OPACITY, op);
                material.setColor(glm::vec4(glm::vec3(material.getColor()), op));
                break;
            }
            case aiTextureType_SPECULAR: //roughness
            {
                aiColor3D spec(0.0f, 0.0f, 0.0f);
                assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, spec);
                material.setRoughness(spec.r);
                break;
            }
            case aiTextureType_SHININESS: //metallic
            {
                float shn;
                assimpMat->Get(AI_MATKEY_SHININESS, shn);
                material.setMetallic(shn);
                break;
            }
            //case aiTextureType_NORMALS: //normal
            //    break;
            //case aiTextureType_HEIGHT: //height-to-normal or normal alpha
            //    break;
            //case aiTextureType_LIGHTMAP: //ambient occlusion
            //    break;
            default:
                break;
            }
        }

        float ior;
        assimpMat->Get(AI_MATKEY_REFRACTI, ior);
        material.setIOR(ior);
    }

    vntThread.join();
    indexThread.join();

    calculateBoundingBox();
}

void Mesh::copyToAlpha(const std::experimental::filesystem::path& src, const std::shared_ptr<Texture>& dst) const
{
    int imageWidth, imageHeight, numChannels;

    stbi_set_flip_vertically_on_load(true);

    const auto img = stbi_load(src.string().c_str(), &imageWidth, &imageHeight, &numChannels, 1);

    assert(imageWidth == dst->getSize().x && imageHeight == dst->getSize().y && "Image sizes mismatch!");

    auto data = dst->data<float>(GL_RGBA);

#pragma omp parallel for
    for (int i = 0; i < imageWidth * imageHeight; ++i)
    {
        data[i * 4 + 3] = static_cast<float>(img[i]) / 255.0f;
    }

    dst->assign2D(GL_RGBA, GL_FLOAT, data.data());

    stbi_image_free(img);
    dst->generateMipmaps();
}

std::shared_ptr<Texture> Mesh::generateNormalFromHeight(const std::experimental::filesystem::path& src) const
{
    const glm::vec2 size = glm::vec2(0.5, 0.0); //"strength" of bump-mapping

    int imageWidth, imageHeight, numChannels;

    stbi_set_flip_vertically_on_load(true);

    const auto height = stbi_load(src.string().c_str(), &imageWidth, &imageHeight, &numChannels, 1);

    auto tex = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA16F, glm::ivec2(imageWidth, imageHeight));

    std::vector<glm::vec4> texData(imageWidth * imageHeight);

#pragma omp parallel for
    for (int y = 0; y < imageHeight; ++y)
        for (int x = 0; x < imageWidth; ++x)
        {
            const float s01 = static_cast<float>(height[y * imageWidth + ((x + imageWidth - 1) % imageWidth)]) / 255.0f;
            const float s21 = static_cast<float>(height[y * imageWidth + ((x + 1) % imageWidth)]) / 255.0f;
            const float s10 = static_cast<float>(height[((y + imageHeight - 1) % imageHeight) * imageWidth + x]) / 255.0f;
            const float s12 = static_cast<float>(height[((y + 1) % imageHeight) * imageWidth + x]) / 255.0f;
            const glm::vec3 va = glm::normalize(glm::vec3(size, s21 - s01));
            const glm::vec3 vb = glm::normalize(glm::vec3(glm::vec2(size.y, size.x), s12 - s10));
            const glm::vec3 tanSpaceNormal = cross(va, vb);
            texData[y * imageWidth + x] = glm::vec4(tanSpaceNormal, static_cast<float>(height[y * imageWidth + x]) / 255.0f);
        }

    tex->assign2D(GL_RGBA, GL_FLOAT, texData.data());
    stbi_image_free(height);
    tex->generateMipmaps();
    return tex;
}

Bounds& Mesh::calculateBoundingBox()
{
    struct Reduction
    {
        Bounds operator()(Bounds b, const glm::vec3& x) const { return b + x; }

        Bounds operator()(const glm::vec3& x, Bounds b) const { return b + x; }

        Bounds operator()(const glm::vec3& a, const glm::vec3& b) const
        {
            Bounds bounds;
            bounds = bounds + a;
            return bounds + b;
        }

        Bounds operator()(Bounds b, const Bounds& x) const { return b + x; }
    };

    bounds = std::reduce(std::execution::par_unseq, vertices.begin(), vertices.end(), Bounds(), Reduction());
    return bounds;
}
