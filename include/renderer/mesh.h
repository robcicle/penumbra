#pragma once

#include "math/math.h"

#include "renderer/vertex_array.h"
#include "renderer/texture.h"

namespace penumbra
{
    struct Vertex_t
    {
        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::vec3 m_Normal = glm::vec3(0.0f);
        glm::vec3 m_Tangent = glm::vec3(0.0f);
        //glm::vec3 Binormal;
        glm::vec2 m_Texcoord = glm::vec2(0.0f);
    };

    struct Index_t
    {
		uint32_t m_V1, m_V2, m_V3;
    };

    class CSubmesh
    {
    public:
        ~CSubmesh() = default;

        uint32_t m_nBaseVertex;
        uint32_t m_nBaseIndex;
        uint32_t m_nMaterialIndex;
        uint32_t m_nIndexCount;

        glm::mat4 m_matTransform;
        //AABB m_BoundingBox;

        std::string m_NodeName, m_MeshName;

		Ref<CTexture> m_spAlbedoTexture;
		Ref<CTexture> m_spSpecularTexture;
    };

    class CMesh
    {
    public:
        CMesh() = default;
        CMesh(std::vector<Vertex_t> vecVertices, std::vector<Index_t> vecIndices, std::vector<CSubmesh> vecSubmeshes);
        ~CMesh() = default;

        std::vector<CSubmesh>& GetSubmeshes() { return m_vecSubmeshes; }
		Ref<CVertexArray>& GetVertexArray() { return m_spVertexArray; }
    private:
        std::vector<CSubmesh> m_vecSubmeshes;

        Ref<CVertexArray> m_spVertexArray;
    };
}