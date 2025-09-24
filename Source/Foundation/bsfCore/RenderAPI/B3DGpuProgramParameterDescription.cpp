//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuProgramParameterDescription.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

using namespace b3d;

constexpr u32 RTTIPlainType<GpuDataParameterBlockInformation>::kVersion;
constexpr u32 RTTIPlainType<GpuObjectParameterInformation>::kVersion;
constexpr u32 RTTIPlainType<GpuDataParameterInformation>::kVersion;

RTTIType* GpuProgramParameterDescription::GetRttiStatic()
{
	return GpuProgramParameterDescriptionRTTI::Instance();
}

RTTIType* GpuProgramParameterDescription::GetRtti() const
{
	return GpuProgramParameterDescription::GetRttiStatic();
}
