//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuParamDesc.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

using namespace bs;

constexpr u32 RTTIPlainType<GpuParamBlockDesc>::kVersion;
constexpr u32 RTTIPlainType<GpuParamObjectDesc>::kVersion;
constexpr u32 RTTIPlainType<GpuParamDataDesc>::kVersion;

RTTITypeBase* GpuParamDesc::GetRttiStatic()
{
	return GpuParamDescRTTI::Instance();
}

RTTITypeBase* GpuParamDesc::GetRtti() const
{
	return GpuParamDesc::GetRttiStatic();
}
