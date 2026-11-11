//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Threading/B3DThreading.h"
#include "Utility/B3DBitfield.h"

namespace b3d
{
	class GpuDevice;
	class IGpuResource;
	class IGpuCompletionTracker;
	class SamplerState;
	class GpuBindlessHeap;
	class GpuBindlessHeapPool;

	namespace render
	{
		class Texture;
		class GpuBuffer;
		class GpuResourceTracker;
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
		 * View of a buffer, interpreting its contents as @p viewType. Typed views of simple storage buffers use @p format, or their own
		 * format if BF_UNKNOWN.
		 */
		GpuBindlessResourceView(render::GpuBuffer& buffer, GpuBufferViewType viewType, GpuBufferFormat format);

		bool operator==(const GpuBindlessResourceView& other) const;

		render::Texture* Texture = nullptr; /**< Viewed texture, or null for a buffer view. */
		TextureSurface Surface; /**< Viewed surface of the texture. */

		render::GpuBuffer* Buffer = nullptr; /**< Viewed buffer, or null for a texture view. */
		GpuBufferViewType ViewType = GpuBufferViewType::Typed; /**< How the buffer view interprets the contents of the buffer. */
		GpuBufferFormat Format = BF_UNKNOWN; /**< Element format of a typed buffer view. BF_UNKNOWN for the buffer's own format. */
	};

	/** New descriptor of a bindless resource heap slot. */
	struct GpuBindlessSlotUpdate
	{
		GpuBindlessSlotUpdate() = default;
		GpuBindlessSlotUpdate(u32 slotIndex, const GpuBindlessResourceView& view)
			: SlotIndex(slotIndex), View(view)
		{ }

		u32 SlotIndex = kGpuBindlessInvalidIndex;
		GpuBindlessResourceView View;
	};


	/** Usage counters of a bindless heap. */
	struct GpuBindlessHeapStatistics
	{
		GpuBindlessHeapStatistics() = default;

		u32 Capacity = 0; /**< Number of slots in the heap, including the invalid slot and permanent slots. */
		u32 MaximumCapacity = 0; /**< Number of slots the heap may grow to. */
		u32 AllocatedCount = 0; /**< Number of slots allocated and not released. */
		u32 HighWaterMark = 0; /**< Highest AllocatedCount reached. */
		u32 PendingReleaseCount = 0; /**< Number of released slots waiting for the GPU before they can be reused. */
		u32 EvictionCount = 0; /**< Number of unused cached samplers evicted to free sampler slots. */
		u32 GrowthCount = 0; /**< Number of times the heap grew. */
		u32 UpdateCount = 0; /**< Number of slot updates applied, see GpuBindlessHeaps::UpdateResourceSlot(). */
		u32 UpdateBatchCount = 0; /**< Number of times pending slot updates were applied, each a heap switch or a GPU patch. */
	};

	/** How the bindless heaps change the descriptors of resource slots the GPU may be reading. */
	enum class GpuBindlessUpdateMethod : u8
	{
		/** Switch to a native heap holding the new descriptors. Work recorded before the switch keeps reading the previous native heap. */
		SwitchHeap,
		/** Patch the current native heap in GPU order, after all earlier work on every queue. */
		Patch
	};

	/** Information used for initializing GpuBindlessHeaps. */
	struct GpuBindlessHeapsCreateInformation
	{
		u32 ResourceCapacity = 0; /**< Initial number of slots in the resource heap, including the invalid slot. */
		u32 ResourceMaximumCapacity = 0; /**< Number of slots the resource heap may grow to. At least ResourceCapacity. */
		u32 SamplerCapacity = 0; /**< Number of slots in the sampler heap, including the invalid slot and the global samplers. */
		u32 MaximumResourceHeapCount = 1; /**< Maximum number of native resource heaps kept alive for reuse, including the current one. 1 disables reuse. */
		GpuBindlessUpdateMethod UpdateMethod = GpuBindlessUpdateMethod::SwitchHeap; /**< How slot updates reach the GPU. */
	};

	/**
	 * Allocates slots in the device-global bindless heaps and writes descriptors into them. Owned by GpuDevice, on devices that support
	 * bindless resources.
	 *
	 * A released slot, and the native resource pinned with it, stay alive until the frame of the release completes on the GPU, so work
	 * recorded before the release can still read it.
	 *
	 * The resource heap grows at the end of a frame once fewer than gpu.BindlessResourceGrowthHeadroom of its slots are free, up to
	 * its maximum capacity.
	 *
	 * A command buffer that accesses the heaps must finish recording in the frame it started recording in. The end of a frame may
	 * switch the native resource heap that work recorded afterwards uses, and slots allocated or written afterwards are only valid in it.
	 *
	 * Each backend that supports bindless resources implements the native heaps in a subclass.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuBindlessHeaps
	{
	public:
		/** Destroy() must have run, from the subclass destructor. */
		virtual ~GpuBindlessHeaps();

		/**
		 * Allocates a resource heap slot and writes @p view into it. Returns kGpuBindlessInvalidIndex and logs an error if the heap is full.
		 *
		 * @param	view	View to write into the slot. Its texture or buffer must be pinned with GpuDevice::PinResource() while the slot is allocated.
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

		/**
		 * Replaces the descriptor of allocated resource heap slot @p slotIndex with @p view, at the end of the current frame. Work recorded
		 * before then keeps reading the old descriptor. A later update of the same slot before then replaces this one.
		 *
		 * @param	slotIndex			Allocated resource heap slot.
		 * @param	view				New view of the slot. Its texture or buffer must be pinned with GpuDevice::PinResource() while the slot refers to it.
		 * @param	owner				Object the slot is allocated for.
		 * @param	previousTarget		Resource the old descriptor refers to, if it should be unpinned once no work reads the old descriptor. May be null.
		 */
		void UpdateResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view, const void* owner, IGpuResource* previousTarget);

		/** Releases slot @p slotIndex of heap @p heap, allocated for @p owner. The slot is reused once the current frame completes on the GPU. */
		void ReleaseSlot(GpuBindlessHeapType heap, u32 slotIndex, const void* owner);

		/** Returns released slots whose frames completed on the GPU to use. Called by the device once per frame. */
		void ProcessReleases();

		/** Returns released slots to use regardless of GPU progress. The GPU must be idle. */
		void ProcessAllReleases();

		/** Returns the device the heaps belong to. */
		GpuDevice& GetDevice() const { return mDevice; }

		/**
		 * Applies pending slot updates, and grows the resource heap if it is running out of free slots. Called by the device at the end
		 * of each frame, before the frame boundary, from the render thread.
		 */
		void ApplyPendingChanges();

		/** Returns usage counters of heap @p heap. */
		GpuBindlessHeapStatistics GetStatistics(GpuBindlessHeapType heap) const;

		/**
		 * Returns the current native resource heap, and tracks it with @p resourceTracker, which keeps it alive and unchanged while the
		 * command buffer owning the tracker is recording or executing.
		 */
		GpuBindlessHeap* GetAndTrackResourceHeap(render::GpuResourceTracker& resourceTracker);

	protected:
		friend class GpuDevice;

		/**
		 * @param	device				Device that owns the heaps.
		 * @param	completionTracker	Tracker whose current marker is stamped onto released slots. Slots return to use once it reports the marker complete.
		 * @param	createInformation	Capacities and update method of the heaps.
		 */
		GpuBindlessHeaps(GpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation);

		/**
		 * Creates the first native resource heap. Called once by GpuDevice::CreateBindlessHeaps(), before it creates the global samplers.
		 * Subclasses that need more setup override it, and call it once that setup succeeded. Returns false on failure.
		 */
		virtual bool Initialize();

		/**
		 * Releases the global samplers and every slot and pin still pending release, reports slots that were never released, and destroys
		 * the native resource heaps. Must be called from the subclass destructor, since it writes slots. The GPU must be idle, and nothing
		 * else may reference the global samplers.
		 */
		void Destroy();

		/** Returns the native heaps behind the resource heap. */
		GpuBindlessHeapPool& GetResourceHeapPool() { return *mResourceHeapPool; }
		const GpuBindlessHeapPool& GetResourceHeapPool() const { return *mResourceHeapPool; }

		/**
		 * Makes a native resource heap of @p capacity slots current, holding the current contents of the resource heap and @p updates.
		 * Takes a pooled native heap if @p capacity is the current capacity, and creates one otherwise. Used for growth by every backend,
		 * and for updates by backends that switch native heaps. Returns false and leaves the current native heap unchanged on failure.
		 *
		 * @param	capacity	Number of slots of the native heap. At least the current capacity.
		 * @param	updates		New descriptors of slots in the current capacity, at most one per slot.
		 * @param	liveSlots	Slots whose current descriptors are valid, and may be copied.
		 */
		virtual bool SwitchResourceHeap(u32 capacity, TArrayView<const GpuBindlessSlotUpdate> updates, const TBitfield<>& liveSlots);

		/**
		 * Creates a native resource heap of @p capacity slots, growing backend copies of the resource heap as needed. Its slot contents are
		 * undefined. Returns null on failure.
		 */
		virtual GpuBindlessHeap* CreateResourceHeap(u32 capacity) = 0;

		/** Copies the latest descriptors of the resource slots in @p ranges into @p destination, which is not current. */
		virtual void CopyResourceSlots(GpuBindlessHeap& destination, TArrayView<const TIndexRange<u32>> ranges) = 0;

		/** Writes the descriptors of @p updates into @p destination, which is not current, and into any backend copy of the resource heap. */
		virtual void WriteResourceSlots(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates) = 0;

		/**
		 * Writes the descriptor of @p view into resource slot @p slotIndex. Never called for slots that GPU work may still read, so only
		 * the current native heap needs it. Backends that switch native heaps then call GpuBindlessHeapPool::MarkSlotWritten().
		 */
		virtual void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) = 0;

		/**
		 * Writes a null descriptor into resource slot @p slotIndex, like WriteResourceSlot().
		 *
		 * @param	slotIndex	Index of the slot in the resource heap.
		 * @param	viewType	Type of the view last written into the slot, which stale indices are read as. The null descriptor must
		 *						match it.
		 */
		virtual void ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType) = 0;

		/** Writes the descriptor of @p sampler into sampler slot @p slotIndex. Never called for slots that GPU work may still read. */
		virtual void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) = 0;

		/**
		 * Changes the descriptors of resource slots in the current native heap, in GPU order after all earlier work on every queue.
		 * Called at the end of a frame, before the frame boundary, whose fence makes the next frame's work on every queue wait for the
		 * patch. Only called with GpuBindlessUpdateMethod::Patch. Returns false if the updates could not be applied. They are dropped.
		 */
		virtual bool PatchResourceSlots(TArrayView<const GpuBindlessSlotUpdate> updates);

	private:
		/** State of one slot. Development builds only. */
		struct SlotRecord
		{
			SlotRecord() = default;

			const void* Owner = nullptr; /**< Object the slot is allocated for, or null if the slot isn't allocated. */
			String OwnerName; /**< Debug name of the viewed resource or sampler. */
			GpuParameterObjectType ViewType = GPOT_UNKNOWN; /**< Type shaders read the view in a resource heap slot as. */
			bool IsPendingRelease = false;
		};

		/** Slots of one heap. */
		struct Heap
		{
			Heap() = default;

			u32 Capacity = 0;
			u32 MaximumCapacity = 0;
			u32 NextUnusedSlotIndex = 1; /**< First slot that was never allocated. Slot 0 is the invalid index. */
			TArray<u32> FreeSlotIndices; /**< Released slots ready for reuse, used in LIFO order. */

			/**
			 * Slots whose descriptors are valid, so copying them into another native heap object is safe: allocated slots, and in
			 * development builds also released slots, which hold a null descriptor or a view of a resource still pinned.
			 */
			TBitfield<> LiveSlots;
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

		/** Slot update waiting for the end of the current frame. */
		struct PendingSlotUpdate
		{
			PendingSlotUpdate() = default;
			PendingSlotUpdate(const GpuBindlessSlotUpdate& update, IGpuResource* previousTarget)
				: Update(update), PreviousTarget(previousTarget)
			{ }

			GpuBindlessSlotUpdate Update;
			IGpuResource* PreviousTarget = nullptr; /**< Unpinned once the update is applied and no work reads the old descriptor. May be null. */
		};

		/**
		 * Creates the global samplers and writes them into their permanent slots. Called once by GpuDevice::CreateBindlessHeaps(), after
		 * Initialize() and before any other sampler slot is allocated.
		 */
		void CreateGlobalSamplers();

		/** Takes a free slot of @p heap, or returns kGpuBindlessInvalidIndex if it is full. Must be called with mMutex held. */
		u32 TakeSlot(Heap& heap, const void* owner, const String& ownerName);

		/** Grows the resource heap if fewer slots than the growth headroom are free. Must be called with mMutex held. */
		void GrowIfNeeded();

		/** Returns released entries that pass @p fnIsComplete to use. */
		template<class IsCompleteFunction>
		void ProcessReleases(IsCompleteFunction fnIsComplete);

		GpuDevice& mDevice;
		const IGpuCompletionTracker& mCompletionTracker;
		GpuBindlessUpdateMethod mUpdateMethod;
		u32 mMaximumResourceHeapCount;
		TUnique<GpuBindlessHeapPool> mResourceHeapPool; /**< Native heaps behind the resource heap. Null before Initialize() and after Destroy(). */
		Heap mHeaps[(u32)GpuBindlessHeapType::Count];
		TArray<PendingSlotRelease> mPendingSlotReleases; /**< In release order, so markers ascend. */
		UnorderedMap<u32, PendingSlotUpdate> mPendingSlotUpdates; /**< Keyed by slot index. */
		TArray<TShared<SamplerState>> mGlobalSamplers; /**< Indexed by global sampler index minus one. */
		mutable Mutex mMutex;
	};

	/**
	 * Bindless slots of one texture or buffer: the slot of its default view, created on registration, and slots of other views,
	 * created on demand. Pins the resource's current native object while registered.
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

		/**
		 * Points every slot at @p nativeResource, the native resource that replaced @p previousNativeResource when the texture or buffer moved
		 * to other memory, and moves the pin to it. The slots keep their indices. Work that reads the slots before the update is applied
		 * keeps reading @p previousNativeResource, which stays pinned until then. See GpuBindlessHeaps::UpdateResourceSlot().
		 */
		void Retarget(IGpuResource& previousNativeResource, IGpuResource& nativeResource);

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
