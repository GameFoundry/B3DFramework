//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Allocators/B3DStaticAlloc.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d::render
{
	class D3D12GpuDevice;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/**
	 * Owns the lifetime of D3D12-side IGpuResource instances on a single device. Inherits leak tracking and the
	 * deferred-destroy free path from GpuResourceManager.
	 *
	 * @note Thread safe.
	 */
	class D3D12ResourceManager : public GpuResourceManager
	{
	public:
		D3D12ResourceManager(D3D12GpuDevice& device);

		/**
		 * Creates a new D3D12 resource of the specified type. User must call IGpuResource::Destroy() when done
		 * using the resource.
		 */
		template <class Type, class... Args>
		Type* Create(Args&&... args)
		{
			Type* resource = new(B3DAllocate(sizeof(Type))) Type(this, std::forward<Args>(args)...);
			RegisterResource(resource);
			return resource;
		}

		/** Returns the device that owns this manager. */
		D3D12GpuDevice& GetDevice() const;
	};

	/** @} */
} // namespace b3d::render
