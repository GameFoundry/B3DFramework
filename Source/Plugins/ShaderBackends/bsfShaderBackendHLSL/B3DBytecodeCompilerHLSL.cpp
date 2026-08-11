//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBytecodeCompilerHLSL.h"
#include "B3DHLSLToDXBC.h"

namespace b3d
{
	namespace render
	{
		TShared<IGpuBytecodeCompiler> CreateBytecodeCompilerhlsl()
		{
			return B3DMakeShared<HLSLToDXBC>(kHlslCompilerId, kHlslCompilerVersion);
		}
	} // namespace render
} // namespace b3d
