//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuParameterSetPool.h"

namespace b3d
{
	GpuParameterSetPool::GpuParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
		: mInformation(createInformation), mOwnerThread(B3D_CURRENT_THREAD_ID)
	{
	}
} // namespace b3d
