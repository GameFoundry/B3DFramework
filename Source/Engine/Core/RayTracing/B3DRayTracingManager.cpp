//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RayTracing/B3DRayTracingManager.h"

#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"

namespace b3d::render
{
	const TShared<GpuBuffer>& RayTracingManager::GetDefaultAttributes(GpuDevice& device)
	{
		ASSERT_IF_NOT_RENDER_THREAD;

		TShared<GpuBuffer>& defaultAttributes = mDefaultAttributes[&device];
		if(defaultAttributes != nullptr || device.GetBindlessHeaps() == nullptr)
			return defaultAttributes;

		// TODO: Device memory might be a better choice, but probably not worth the extra complexity
		const TShared<GpuBuffer> buffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(sizeof(RayTracingDefaultAttributes), 1,
			GpuBufferFlag::Bindless, GpuBufferMemory::Mapped));
		if(buffer == nullptr)
			return defaultAttributes;

		buffer->SetName("Ray tracing default attributes");

		{
			const GpuBufferMappedScope mapping = buffer->Map(GpuMapOption::Write);
			if(!B3D_ENSURE_LOG(mapping.IsValid(), "Couldn't map the default attributes of ray tracing geometry."))
				return defaultAttributes;

			const RayTracingDefaultAttributes values;
			std::memcpy(mapping.GetMappedMemory(), &values, sizeof(values));
		}

		defaultAttributes = buffer;
		return defaultAttributes;
	}

	void RayTracingManager::OnShutDown()
	{
		// The buffers are only used on the render thread, so they are released there
		GetRenderThread().PostCommand([this]() { mDefaultAttributes.clear(); }, "Release ray tracing default attributes", true);
	}
} // namespace b3d::render
