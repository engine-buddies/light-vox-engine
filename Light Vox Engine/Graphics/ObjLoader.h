#pragma once

#include "../stdafx.h"
#include "ShaderDefinitions.h"
#include <vector>

namespace Graphics
{

    //TODO - remove this because it's only used just once (it doesn't have to be a struct)
    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<UINT> indices;
    };

    class ObjLoader
    {
    public:
        static void LoadObj( std::vector<Vertex>* vertices, std::vector<uint16_t>* indices, std::string filePath );

        /// <summary>
        /// Generates a Full Screen Quad 
        /// used for deferred directional light calculations 
        /// </summary>
        void GenerateFullScreenQuad( MeshData& meshData );
    };
}
