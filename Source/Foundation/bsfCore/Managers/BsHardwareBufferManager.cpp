//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsGpuParams.h"

using namespace bs;

SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
{
	GpuParams* params = new(B3DAllocate<GpuParams>()) GpuParams(paramInfo);
	SPtr<GpuParams> paramsPtr = B3DMakeCoreFromExisting<GpuParams>(params);
	paramsPtr->SetShared(paramsPtr);
	paramsPtr->Initialize();

	return paramsPtr;
}

namespace bs { namespace ct
{

SPtr<GpuParams> HardwareBufferManager::CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	SPtr<GpuParams> params = CreateGpuParamsInternal(paramInfo, deviceMask);
	params->Initialize();

	return params;
}

SPtr<GpuParams> HardwareBufferManager::CreateGpuParamsInternal(
	const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	GpuParams* params = new(B3DAllocate<GpuParams>()) GpuParams(paramInfo, deviceMask);
	SPtr<GpuParams> paramsPtr = B3DMakeSharedFromExisting<GpuParams>(params);
	paramsPtr->SetShared(paramsPtr);

	return paramsPtr;
}
}}
