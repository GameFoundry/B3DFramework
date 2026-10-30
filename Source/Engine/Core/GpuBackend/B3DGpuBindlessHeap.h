//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	class GpuDevice;
	class IGpuResource;
	class IGpuCompletionTracker;
	class SamplerState;

	namespace render
	{
		class Texture;
		class GpuBuffer;
	}

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Descriptor set (register space on D3D12) at which the bindless heaps are bound for pipelines whose programs access
	 * ResourceDescriptorHeap or SamplerDescriptorHeap. Programs that access the heaps must not declare their own parameters
	 * in this set.
	 */
	inline constexpr u32 kGpuBindlessParameterSet = 7;

	/** Bindless index that refers to no slot. Never allocated, in either heap. */
	inline constexpr u32 kGpuBindlessInvalidIndex = 0;

	/** Heaps that bindless indices refer to. Each heap has its own index space. */
	enum class GpuBindlessHeapType : u8
	{
		Resource, /**< Read-only views of textures and buffers, indexed through ResourceDescriptorHeap. */
		Sampler, /**< Samplers, indexed through SamplerDescriptorHeap. */
		Count
	};

	/** Filtering of a global sampler. See GetGlobalSamplerIndex(). */
	enum class GpuGlobalSamplerFilter : u8
	{
		Point, /**< Point filtering, including between mip levels. */
		Bilinear, /**< Linear filtering within a mip level, point filtering between mip levels. */
		Trilinear, /**< Linear filtering within and between mip levels. */
		Count
	};

	/** Addressing of a global sampler, on all axes. See GetGlobalSamplerIndex(). */
	enum class GpuGlobalSamplerAddress : u8
	{
		Wrap,
		Clamp,
		Count
	};

	/** Number of global samplers, permanently resident in the sampler heap at indices 1 to kGpuGlobalSamplerCount. */
	inline constexpr u32 kGpuGlobalSamplerCount = (u32)GpuGlobalSamplerFilter::Count * (u32)GpuGlobalSamplerAddress::Count;

	/**
	 * Returns the sampler heap index of a global sampler. Global samplers are available on every device that supports bindless
	 * resources. Shaders can use the matching constants from BindlessSamplers.bslinc instead of passing the index.
	 */
	constexpr u32 GetGlobalSamplerIndex(GpuGlobalSamplerFilter filter, GpuGlobalSamplerAddress address)
	{
		return 1 + (u32)filter * (u32)GpuGlobalSamplerAddress::Count + (u32)address;
	}

	/** Read-only view of a texture or a buffer, written into a resource heap slot. */
	struct B3D_EXPORT GpuBindlessResourceView
	{
		GpuBindlessResourceView() = default;

		/** View of the @p surface of a texture. */
		GpuBindlessResourceView(render::Texture& texture, const TextureSurface& surface);

		/**
		 * View of a buffer. Structured buffers are viewed as structured, and simple storage buffers as typed, using @p format or
		 * their own format if BF_UNKNOWN.
		 */
		GpuBindlessResourceView(render::GpuBuffer& buffer, GpuBufferFormat format);

		bool operator==(const GpuBindlessResourceView& other) const;

		render::Texture* Texture = nullptr; /**< Viewed texture, or null for a buffer view. */
		TextureSurface Surface; /**< Viewed surface of the texture. */

		render::GpuBuffer* Buffer = nullptr; /**< Viewed buffer, or null for a texture view. */
		GpuBufferFormat Format = BF_UNKNOWN; /**< Element format of a typed buffer view. BF_UNKNOWN for the buffer's own format. */
	};

	/** Usage counters of a bindless heap. */
	struct GpuBindlessHeapStatistics
	{
		GpuBindlessHeapStatistics() = default;

		u32 Capacity = 0; /**< Number of slots in the heap, including the invalid slot and permanent slots. */
		u32 AllocatedCount = 0; /**< Number of slots allocated and not released. */
		u32 HighWaterMark = 0; /**< Highest AllocatedCount reached. */
		u32 PendingReleaseCount = 0; /**< Number of released slots waiting for the GPU before they can be reused. */
		u32 EvictionCount = 0; /**< Number of unused cached samplers evicted to free sampler slots. */
	};

	/**
	 * Native implementation of the bindless heaps, implemented by the GPU device of each backend that supports bindless resources.
	 * See GpuBindlessHeaps.
	 */
	class B3D_EXPORT IGpuBindlessHeap
	{
	public:
		virtual ~IGpuBindlessHeap() = default;

		/** Writes the descriptor of @p view into resource heap slot @p slotIndex. Never called for slots that GPU work may still read. */
		virtual void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) = 0;

		/** Writes the descriptor of @p sampler into sampler heap slot @p slotIndex. Never called for slots that GPU work may still read. */
		virtual void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) = 0;

		/** Writes a null descriptor into slot @p slotIndex of heap @p heap. Never called for slots that GPU work may still read. */
		virtual void ClearSlot(GpuBindlessHeapType heap, u32 slotIndex) = 0;
	};

	/**
	 * Allocates slots in the device-global bindless heaps and writes descriptors into them. Owned by GpuDevice, on devices that support
	 * bindless resources.
	 *
	 * A released slot, and the native resource pinned with it, stay alive until the frame of the release completes on the GPU, so work
	 * recorded before the release can still read it.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuBindlessHeaps
	{
	public:
		/**
		 * @param	device				Device that owns the heaps.
		 * @param	backend				Native implementation of the heaps. Must be ready to write slots.
		 * @param	completionTracker	Tracker whose current marker is stamped onto released slots. Slots return to use once it reports the marker complete.
		 * @param	resourceCapacity	Number of slots in the resource heap, including the invalid slot.
		 * @param	samplerCapacity		Number of slots in the sampler heap, including the invalid slot and the global samplers.
		 */
		GpuBindlessHeaps(GpuDevice& device, IGpuBindlessHeap& backend, const IGpuCompletionTracker& completionTracker, u32 resourceCapacity, u32 samplerCapacity);

		/**
		 * Releases the global samplers and every slot and pin still pending release, and reports slots that were never released. The GPU
		 * must be idle, and nothing else may reference the global samplers.
		 */
		~GpuBindlessHeaps();

		/** Creates the global samplers and writes them into their permanent slots. Must be called once, before any other slot is allocated. */
		void CreateGlobalSamplers();

		/**
		 * Allocates a resource heap slot and writes @p view into it. Returns kGpuBindlessInvalidIndex and logs an error if the heap is full.
		 *
		 * @param	view	View to write into the slot. Its texture or buffer must be pinned with PinResource() while the slot is allocated.
		 * @param	owner	Object responsible for releasing the slot. Development builds check releases against it.
		 */
		u32 AllocateResourceSlot(const GpuBindlessResourceView& view, const void* owner);

		/**
		 * Allocates a sampler heap slot and writes @p sampler into it. If the heap fills past gpu.BindlessSamplerEvictionThreshold,
		 * evicts cached samplers nothing else references first. Returns kGpuBindlessInvalidIndex and logs an error if the heap is full.
		 *
		 * @param	sampler	Sampler to write into the slot.
		 * @param	owner	Object responsible for releasing the slot. Development builds check releases against it.
		 */
		u32 AllocateSamplerSlot(const SamplerState& sampler, const void* owner);

		/** Releases slot @p slotIndex of heap @p heap, allocated for @p owner. The slot is reused once the current frame completes on the GPU. */
		void ReleaseSlot(GpuBindlessHeapType heap, u32 slotIndex, const void* owner);

		/**
		 * Keeps @p resource alive until a matching UnpinResource() completes, even if it is destroyed. Heaps reference native resources
		 * without command buffers tracking them, so resources must be pinned while any slot refers to them.
		 */
		void PinResource(IGpuResource& resource);

		/** Releases a pin taken by PinResource(), once the current frame completes on the GPU. */
		void UnpinResource(IGpuResource& resource);

		/** Returns released slots and pins whose frames completed on the GPU to use. Called by the device once per frame. */
		void ProcessReleases();

		/** Returns released slots and pins to use regardless of GPU progress. The GPU must be idle. */
		void ProcessAllReleases();

		/** Returns usage counters of heap @p heap. */
		GpuBindlessHeapStatistics GetStatistics(GpuBindlessHeapType heap) const;

	private:
		/** State of one slot. Development builds only. */
		struct SlotRecord
		{
			SlotRecord() = default;

			const void* Owner = nullptr; /**< Object the slot is allocated for, or null if the slot isn't allocated. */
			String OwnerName; /**< Debug name of the viewed resource or sampler. */
			bool IsPendingRelease = false;
		};

		/** Slots of one heap. */
		struct Heap
		{
			Heap() = default;

			u32 Capacity = 0;
			u32 NextUnusedSlotIndex = 1; /**< First slot that was never allocated. Slot 0 is the invalid index. */
			TArray<u32> FreeSlotIndices; /**< Released slots ready for reuse, used in LIFO order. */
			GpuBindlessHeapStatistics Statistics;

#if B3D_BUILD_TYPE_DEVELOPMENT
			TArray<SlotRecord> SlotRecords; /**< Indexed by slot. */
#endif
		};

		/** Slot waiting for the GPU to finish the frame it was released in. */
		struct PendingSlotRelease
		{
			PendingSlotRelease() = default;
			PendingSlotRelease(u64 marker, GpuBindlessHeapType heap, u32 slotIndex)
				: Marker(marker), HeapType(heap), SlotIndex(slotIndex)
			{ }

			u64 Marker = 0; /**< Completion tracker marker of the release. */
			GpuBindlessHeapType HeapType = GpuBindlessHeapType::Resource;
			u32 SlotIndex = kGpuBindlessInvalidIndex;
		};

		/** Pin waiting for the GPU to finish the frame it was released in. */
		struct PendingUnpin
		{
			PendingUnpin() = default;
			PendingUnpin(u64 marker, IGpuResource* resource)
				: Marker(marker), Resource(resource)
			{ }

			u64 Marker = 0; /**< Completion tracker marker of the release. */
			IGpuResource* Resource = nullptr;
		};

		/** Takes a free slot of @p heap, or returns kGpuBindlessInvalidIndex if it is full. Must be called with mMutex held. */
		u32 TakeSlot(Heap& heap, const void* owner, const String& ownerName);

		/** Returns released entries that pass @p fnIsComplete to use. */
		template<class IsCompleteFunction>
		void ProcessReleases(IsCompleteFunction fnIsComplete);

		GpuDevice& mDevice;
		IGpuBindlessHeap& mBackend;
		const IGpuCompletionTracker& mCompletionTracker;
		Heap mHeaps[(u32)GpuBindlessHeapType::Count];
		TArray<PendingSlotRelease> mPendingSlotReleases; /**< In release order, so markers ascend. */
		TArray<PendingUnpin> mPendingUnpins; /**< In release order, so markers ascend. */
		TArray<TShared<SamplerState>> mGlobalSamplers; /**< Indexed by global sampler index minus one. */
		mutable Mutex mMutex;
	};

	/**
	 * Bindless slots of one texture or buffer: the slot of its default view, created on registration, and slots of other views,
	 * created on demand. Pins the resource's native object while registered.
	 *
	 * @note	Render thread only.
	 */
	class B3D_EXPORT GpuBindlessResourceSlots
	{
	public:
		GpuBindlessResourceSlots() = default;
		~GpuBindlessResourceSlots();

		/**
		 * Pins @p nativeResource and allocates the slot of @p defaultView. Logs an error and leaves the resource without slots if the device
		 * does not support bindless resources.
		 */
		void Register(GpuDevice& device, IGpuResource& nativeResource, const GpuBindlessResourceView& defaultView);

		/**
		 * Releases every slot and the pin of @p nativeResource, which must be the resource passed to Register(). Work recorded before the
		 * call can still use the slots.
		 */
		void Unregister(IGpuResource& nativeResource);

		/** Returns the index of the default view's slot, or kGpuBindlessInvalidIndex if not registered. */
		u32 GetDefaultIndex() const { return mDefaultIndex; }

		/** Returns the index of the slot of @p view, allocating it on first request. Returns kGpuBindlessInvalidIndex if not registered or the heap is full. */
		u32 GetOrCreateIndex(const GpuBindlessResourceView& view);

	private:
		/** Slot of a view other than the default view. */
		struct ViewSlot
		{
			ViewSlot() = default;
			ViewSlot(const GpuBindlessResourceView& view, u32 index)
				: View(view), Index(index)
			{ }

			GpuBindlessResourceView View;
			u32 Index = kGpuBindlessInvalidIndex;
		};

		GpuBindlessHeaps* mHeaps = nullptr; /**< Heaps the slots are allocated in, or null if not registered. */
		GpuBindlessResourceView mDefaultView;
		u32 mDefaultIndex = kGpuBindlessInvalidIndex;
		TArray<ViewSlot> mViewSlots;
	};

	/** @} */
} // namespace b3d
