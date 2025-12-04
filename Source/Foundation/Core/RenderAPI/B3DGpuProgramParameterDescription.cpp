//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuProgramParameterDescription.h"
#include "RTTI/B3DGpuProgramRTTI.h"

using namespace b3d;

constexpr u32 RTTIPlainType<GpuUniformBufferMemberInformation>::kVersion;

RTTIType* GpuProgramParameterDescription::GetRttiStatic()
{
	return GpuProgramParameterDescriptionRTTI::Instance();
}

RTTIType* GpuProgramParameterDescription::GetRtti() const
{
	return GpuProgramParameterDescription::GetRttiStatic();
}
