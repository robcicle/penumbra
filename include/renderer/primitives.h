#pragma once

namespace penumbra
{
	class CPrimitives3D
	{
	public:
		static float* GetCubeVerts(uint32_t& nArraySize);
		static uint32_t* GetCubeIndices(uint32_t& nArraySize);

		static float* GetPyramidVerts(uint32_t& nArraySize);
		static uint32_t* GetPyramidIndices(uint32_t& nArraySize);

		static float* GetPlaneVerts(uint32_t& nArraySize);
		static uint32_t* GetPlaneIndices(uint32_t& nArraySize);
	};

	class CPrimitives2D
	{
	public:
		static float* GetQuadVerts(uint32_t& nArraySize);
		static uint32_t* GetQuadIndices(uint32_t& nArraySize);
	};
}