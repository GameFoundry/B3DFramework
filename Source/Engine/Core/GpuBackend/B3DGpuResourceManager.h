//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d
{
	class GpuDevice;

	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Owns the lifetime of IGpuResource instances on a single device. Provides leak tracking in debug
	 * builds and the deferred-destruction free path that matches the manager-allocated construction performed
	 * by backend-specific Create<T> templates.
	 *
	 * Symmetry contract: any IGpuResource queued through RegisterResource (typically via a backend
	 * manager's Create<T>) must eventually be released through Destroy, which performs the matching
	 * B3DDelete. The base class never allocates resources itself — backend subclasses do that, since they
	 * know the typed manager pointer to pass into the resource constructor.
	 *
	 * @note Thread safe.
	 */
	class B3D_EXPORT GpuResourceManager
	{
	public:
		GpuResourceManager(GpuDevice& device);
		virtual ~GpuResourceManager();

		/** Returns the device that owns this manager. */
		GpuDevice& GetDevice() const { return mDevice; }

		/**
		 * Frees the given resource and removes it from the leak-tracking set. Callers should not invoke this
		 * directly — use IGpuResource::Destroy instead, which routes through the deferred-destroy path.
		 */
		void Destroy(IGpuResource* resource);

	protected:
		/**
		 * Registers a freshly constructed resource for leak tracking (debug builds only). Backend managers
		 * call this from their Create<T> templates after construction.
		 */
		void RegisterResource(IGpuResource* resource);

		GpuDevice& mDevice;

#if B3D_DEBUG
		UnorderedSet<IGpuResource*> mResources;
		Mutex mMutex;
#endif
	};

	/** @} */
} // namespace b3d
