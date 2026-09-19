//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShaderReflection.h"
#include "RTTI/B3DShaderReflectionRTTI.h"

namespace b3d
{
	RTTIType* ShaderReflection::GetRttiStatic()
	{
		return ShaderReflectionRTTI::Instance();
	}

	RTTIType* ShaderReflection::GetRtti() const
	{
		return GetRttiStatic();
	}
}
