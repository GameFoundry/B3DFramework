//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBytecodeCompilerVKSL.h"
#include "B3DGLSLToSPIRV.h"

namespace b3d
{
	namespace render
	{
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilervksl()
		{
			return B3DMakeShared<GLSLToSPIRV>(kVulkanCompilerId, kVulkanCompilerVersion);
		}
	} // namespace render
} // namespace b3d
