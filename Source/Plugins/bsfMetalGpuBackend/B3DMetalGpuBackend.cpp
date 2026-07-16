//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuBackend.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalTextureManager.h"
#include "B3DMetalRenderWindowManager.h"
#include "B3DMetalGpuFrameCapture.h"

namespace b3d
{
	void MetalGpuBackend::OnStartUp()
	{
		auto device = B3DMakeShared<render::MetalGpuDevice>();
		device->Initialize();
		mDevices.Add(device);

		// Create the texture managers
		TextureManager::StartUp<MetalTextureManager>();
		render::TextureManager::StartUp<render::MetalTextureManager>(*mDevices[0]);

#if B3D_BUILD_TYPE_DEVELOPMENT
		mFrameCapture = B3DMakeShared<MetalGpuFrameCapture>(static_cast<render::MetalGpuDevice&>(*mDevices[0]));
#endif

		// Create render window manager
		RenderWindowManager::StartUp<MetalRenderWindowManager>();

		Super::OnStartUp();
	}

	void MetalGpuBackend::OnShutDown()
	{
		// Drain every initialized device before tearing down engine-side managers or dropping the
		// device TShareds. Without this the Metal queues may still hold scheduled @c MTL4CommandBuffers
		// that reference resources owned by higher-level managers (textures, render windows) — if we
		// destroy those managers while the GPU is mid-frame, backing MTLResources get released out
		// from under in-flight commands and the driver flags a residency hazard at submit time.
		// Mirrors VulkanGpuBackend::OnShutDown.
		for (const auto& device : mDevices)
		{
			if (!device->IsInitialized())
				continue;

			device->WaitUntilIdle();
		}

		RenderWindowManager::ShutDown();
		render::TextureManager::ShutDown();
		TextureManager::ShutDown();

		mFrameCapture = nullptr;
		mDevices.clear();

		Super::OnShutDown();
	}

	MetalGpuBackend& GetMetalGpuBackend()
	{
		return static_cast<MetalGpuBackend&>(GpuBackend::Instance());
	}
} // namespace b3d
