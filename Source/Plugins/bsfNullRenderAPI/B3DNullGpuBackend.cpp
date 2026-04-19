//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuBackend.h"
#include "B3DNullGpuDevice.h"
#include "B3DNullTextureManager.h"
#include "B3DNullRenderWindowManager.h"

namespace b3d
{
	void NullGpuBackend::OnStartUp()
	{
		// Create and initialize a single null device
		auto device = B3DMakeShared<render::NullGpuDevice>();
		device->Initialize();
		mDevices.Add(device);

		// Create the texture managers
		TextureManager::StartUp<NullTextureManager>();
		render::TextureManager::StartUp<render::NullTextureManager>(*mDevices[0]);

		// Create render window manager
		RenderWindowManager::StartUp<NullRenderWindowManager>();

		Super::OnStartUp();
	}

	void NullGpuBackend::OnShutDown()
	{
		RenderWindowManager::ShutDown();
		render::TextureManager::ShutDown();
		TextureManager::ShutDown();

		mDevices.clear();

		Super::OnShutDown();
	}

	NullGpuBackend& GetNullGpuBackend()
	{
		return static_cast<NullGpuBackend&>(GpuBackend::Instance());
	}
} // namespace b3d
