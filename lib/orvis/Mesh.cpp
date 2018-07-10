#include "Mesh.hpp"
#include <numeric>
#include <execution>

Mesh::Mesh(aiMesh* assimpMesh, aiMaterial* assimpMat)
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
            switch (type)
            {
            case aiTextureType_DIFFUSE: //albedo

                break;
            case aiTextureType_OPACITY: //albedo alpha
                //if (assimpMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
                    //copyToAlpha();
                break;
            case aiTextureType_SPECULAR: //roughness

                break;
            case aiTextureType_SHININESS: //metallic

                break;
            case aiTextureType_NORMALS: //normal

                break;
            case aiTextureType_HEIGHT: //height-to-normal or normal alpha
                //if (assimpMat->GetTextureCount(aiTextureType_NORMALS) > 0)
                    //copyToAlpha();
                //else
                    //generateNormalFromHeight();
                break;
            case aiTextureType_LIGHTMAP: //ambient occlusion

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
                aiColor3D diffcolor(0.0f, 0.0f, 0.0f);
                assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffcolor);
                material.setColor(glm::vec4(diffcolor.r, diffcolor.g, diffcolor.b, 1.0f));
                break;
            case aiTextureType_OPACITY: //albedo alpha
                float op;
                assimpMat->Get(AI_MATKEY_OPACITY, op);
                material.setColor(glm::vec4(glm::vec3(material.getColor()), op));
                break;
            case aiTextureType_SPECULAR: //roughness
                aiColor3D spec(0.0f, 0.0f, 0.0f);
                assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, spec);
                material.setRoughness(spec.r);
                break;
            case aiTextureType_SHININESS: //metallic
                float shn;
                assimpMat->Get(AI_MATKEY_SHININESS, shn);
                material.setMetallic(shn);
                break;
            case aiTextureType_NORMALS: //normal
                break;
            case aiTextureType_HEIGHT: //height-to-normal or normal alpha
                break;
            case aiTextureType_LIGHTMAP: //ambient occlusion
                break;
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
}

Bounds& Mesh::calculateBoundingBox()
{
    bounds = std::reduce(std::execution::par, vertices.begin(), vertices.end(), Bounds());
    return bounds;
}
