//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	class GpuPipelineParameterSetLayout;

	namespace render
	{
		class GpuParameterSet;
	}

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Mode for parameter set pool operation. */
	enum class GpuParameterSetPoolMode
	{
		/** Used for per-frame allocations. All parameter sets allocated from the pool are deallocated at once. */
		Transient,
		/** Used for persistent (multi-frame) allocations. Individual sets must be deallocated via Free(). */
		Persistent
	};

	/** Information describing GpuParameterSetPool. */
	struct GpuParameterSetPoolInformation
	{
		GpuParameterSetPoolMode Mode = GpuParameterSetPoolMode::Transient;
		u32 MaxSets = 8192;
		u32 MaxSampledImages = 4096;
		u32 MaxStorageImages = 2048;
		u32 MaxSampledBuffers = 2048;
		u32 MaxStorageBuffers = 2048;
		u32 MaxUniformBuffers = 2048;
		u32 MaxSamplers = 2048;
		u32 MaxCombinedImageSamplers = 2048;
		u32 MaxUniformBuffersDynamic = 1024;
	};

	/** Creation information for GpuParameterSetPool. */
	struct GpuParameterSetPoolCreateInformation : GpuParameterSetPoolInformation
	{
		GpuParameterSetPoolCreateInformation() = default;
		GpuParameterSetPoolCreateInformation(const GpuParameterSetPoolInformation& other)
			:GpuParameterSetPoolInformation(other)
		{ }
	};

	/**
	 * A pool for allocating GPU parameter sets.
	 *
	 * There are two modes of operation:
	 * - Transient: Used for per-frame allocations. All parameter sets allocated from the pool are deallocated at once.
	 * - Persistent: Used for persistent (multi-frame) allocations. Individual sets are deallocated as the go out of scope.
	 *
	 * @note	Not thread safe. The pool, and the parameter sets allocated from it, are used on the thread that created the pool. Other
	 *			threads may bind sets that were flushed for the render pass they are bound in (see render::GpuParameterSet::Flush()).
	 */
	class B3D_EXPORT GpuParameterSetPool
	{
	public:
		virtual ~GpuParameterSetPool() = default;

		/**
		 * Creates a new parameter set from this pool.
		 *
		 * @param layout				Layout that describes the GPU parameters for the set.
		 * @param setIndex				Index of the parameter set that the object will be used for binding parameters for.
		 * @param flags					Creation flags. @see GpuObjectCreateFlag
		 * @return						The created parameter set, or nullptr if allocation failed (e.g., pool exhausted).
		 */
		virtual TShared<render::GpuParameterSet> Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Resets the pool, freeing all allocated sets at once. Only relevant for transient mode pools.
		 * After calling Reset(), all previously allocated parameter sets from this pool become invalid and must not be used.
		 */
		virtual void Reset() = 0;

		/** Returns the mode this pool operates in. */
		GpuParameterSetPoolMode GetMode() const { return mInformation.Mode; }

		/** Returns the thread the pool was created on, which owns the pool and its parameter sets. */
		ThreadId GetOwnerThread() const { return mOwnerThread; }

	protected:
		GpuParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation);

		GpuParameterSetPoolInformation mInformation;
		const ThreadId mOwnerThread;
	};

	/** @} */
} // namespace b3d
