//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d
{
	GpuResourceManager::GpuResourceManager(GpuDevice& device)
		: mDevice(device)
	{}

	GpuResourceManager::~GpuResourceManager()
	{
#if B3D_DEBUG
		Lock lock(mMutex);
		B3D_ASSERT(mResources.empty() && "Resource manager shutting down but not all resources were released.");
#endif
	}

	void GpuResourceManager::RegisterResource(IGpuResource* resource)
	{
#if B3D_DEBUG
		Lock lock(mMutex);
		mResources.insert(resource);
#else
		(void)resource;
#endif
	}

	void GpuResourceManager::Destroy(IGpuResource* resource)
	{
#if B3D_DEBUG
		{
			Lock lock(mMutex);
			mResources.erase(resource);
		}
#endif

		B3DDelete(resource);
	}
} // namespace b3d
