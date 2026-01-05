#include "ppch.h"
#include "renderer/primitives.h"

namespace penumbra
{
	float* CPrimitives3D::GetCubeVerts(uint32_t& nArraySize)
	{
		static float verts[12 * 8] = {
			-0.5f, -0.5f, -0.5f,    // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, -0.5f, -0.5f,     // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, 0.5f, -0.5f,      // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 0.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			-0.5f, 0.5f, -0.5f,     // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 0.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			-0.5f, -0.5f, 0.5f,     // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, -0.5f, 0.5f,      // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, 0.5f, 0.5f,       // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 0.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			-0.5f, 0.5f, 0.5f,      // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 0.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL
		};

		nArraySize = sizeof(verts);

		return verts;
	}
	uint32_t* CPrimitives3D::GetCubeIndices(uint32_t& nArraySize)
	{
		static uint32_t indices[6 * 6] =
		{
			0, 1, 3, 3, 1, 2, // Front
			1, 5, 2, 2, 5, 6, // Right
			5, 4, 6, 6, 4, 7, // Back
			4, 0, 7, 7, 0, 3, // Left
			3, 2, 7, 7, 2, 6, // Top
			4, 5, 0, 0, 5, 1  // Bottom
		};

		nArraySize = sizeof(indices);

		return indices;
	}
	
	float* CPrimitives3D::GetPyramidVerts(uint32_t& nArraySize)
	{
		static float verts[12 * 5] = {
			-0.5f, -0.5f, 0.5f,     // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, -0.5f, 0.5f,      // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			-0.5f, -0.5f, -0.5f,    // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, -0.5f, -0.5f,     // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.0f, 0.5f, 0.0f,       // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.5f, 0.0f,              // UV
			0.0f, 0.0f, 0.0f,		// NORMAL
		};

		nArraySize = sizeof(verts);

		return verts;
	}
	uint32_t* CPrimitives3D::GetPyramidIndices(uint32_t& nArraySize)
	{
		static uint32_t indices[3 * 6] =
		{
			1, 2, 0,
			3, 2, 1,
			4, 1, 0,
			4, 3, 1,
			4, 2, 3,
			4, 0, 2,
		};

		nArraySize = sizeof(indices);

		return indices;
	}

	float* CPrimitives3D::GetPlaneVerts(uint32_t& nArraySize)
	{
		static float verts[12 * 4] = {
			-0.5f, 0.0f, 0.5f,      // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 0.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, 0.0f, 0.5f,       // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 0.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			-0.5f, 0.0f, -0.5f,     // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			0.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL

			0.5f, 0.0f, -0.5f,      // POSITION
			1.0f, 1.0f, 1.0f, 1.0f, // COLOR
			1.0f, 1.0f,             // UV
			0.0f, 0.0f, 0.0f,		// NORMAL
		};

		nArraySize = sizeof(verts);

		return verts;
	}
	uint32_t* CPrimitives3D::GetPlaneIndices(uint32_t& nArraySize)
	{
		static uint32_t indices[1 * 6] =
		{
			0, 2, 1,
			1, 2, 3,
		};

		nArraySize = sizeof(indices);

		return indices;
	}
	
	float* CPrimitives2D::GetQuadVerts(uint32_t& nArraySize)
	{
		static float verts[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		nArraySize = sizeof(verts);

		return verts;
	}
	uint32_t* CPrimitives2D::GetQuadIndices(uint32_t& nArraySize)
	{
		static uint32_t indices[6 * 1] =
		{
			0, 1, 2, 2, 3, 0
		};

		nArraySize = sizeof(indices);

		return indices;
	}
}