#include "ObjLoader.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace DirectX;

//thanks chris <3
void ObjLoader::LoadObj(std::vector<Vertex>* vertices, std::vector<uint16_t>* indices, std::string filePath)
{
	std::cout << "Loading file named: " << filePath << std::endl;
	// File input object
	std::ifstream obj(filePath);

	// Check for successful open
	if (!obj.is_open())
	{
		std::cout << "Unsuccesful at opening the file" << std::endl;
		return;
	}

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	int16_t vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

										 // Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			int16_t i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.position = positions[i[0] - 1];
			v1.uv = uvs[i[1] - 1];
			v1.normal = normals[i[2] - 1];

			Vertex v2;
			v2.position = positions[i[3] - 1];
			v2.uv = uvs[i[4] - 1];
			v2.normal = normals[i[5] - 1];

			Vertex v3;
			v3.position = positions[i[6] - 1];
			v3.uv = uvs[i[7] - 1];
			v3.normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.uv.y = 1.0f - v1.uv.y;
			v2.uv.y = 1.0f - v2.uv.y;
			v3.uv.y = 1.0f - v3.uv.y;

			// Flip Z (LH vs. RH)
			v1.position.z *= -1.0f;
			v2.position.z *= -1.0f;
			v3.position.z *= -1.0f;

			// flip normal z
			v1.normal.z *= -1.0f;
			v2.normal.z *= -1.0f;
			v3.normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			vertices->push_back(v1);
			vertices->push_back(v3);
			vertices->push_back(v2);

			// Add three more indices
			indices->push_back(vertCounter); vertCounter += 1;
			indices->push_back(vertCounter); vertCounter += 1;
			indices->push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.position = positions[i[9] - 1];
				v4.uv = uvs[i[10] - 1];
				v4.normal = normals[i[11] - 1];

				// flip the uv, z pos and normal
				v4.uv.y = 1.0f - v4.uv.y;
				v4.position.z *= -1.0f;
				v4.normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				vertices->push_back(v1);
				vertices->push_back(v4);
				vertices->push_back(v3);

				// Add three more indices
				indices->push_back(vertCounter); vertCounter += 1;
				indices->push_back(vertCounter); vertCounter += 1;
				indices->push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();
}

void ObjLoader::GenerateFullScreenQuad(MeshData & meshData)
{
    meshData.vertices.resize(4);
    meshData.indices.resize(6);

    meshData.vertices[0] = { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
    meshData.vertices[1] = { { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
    meshData.vertices[2] = { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
    meshData.vertices[3] = { {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };

    meshData.indices[0] = 0;
    meshData.indices[1] = 1;
    meshData.indices[2] = 2;

    meshData.indices[3] = 0;
    meshData.indices[4] = 2;
    meshData.indices[5] = 3;
}
