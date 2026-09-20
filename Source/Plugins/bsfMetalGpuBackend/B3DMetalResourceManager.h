//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "Allocators/B3DStaticAlloc.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d::render
{
	class MetalGpuDevice;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Owns the lifetime of Metal-side IGpuResource instances on a single device. Inherits leak tracking and the
	 * deferred-destroy free path from GpuResourceManager.
	 *
	 * @note Thread safe.
	 */
	class MetalResourceManager : public GpuResourceManager
	{
	public:
		MetalResourceManager(MetalGpuDevice& device);

		/**
		 * Creates a new Metal resource of the specified type. User must call IGpuResource::Destroy() when done
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
		MetalGpuDevice& GetDevice() const;
	};

	/** @} */
} // namespace b3d::render
