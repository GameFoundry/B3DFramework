//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Renderer/B3DRendererId.h"
#include "Renderer/B3DRendererSyncManager.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Determines whether a renderer ID was allocated or deallocated. Used for re-playing the operation on the render thread.  */
	enum class RendererIdCommandType : u8
	{
		Allocate,
		Deallocate
	};

	/** Records a single allocation or deallocation of a persistent renderer object ID. */
	struct RendererIdCommand
	{
		RendererIdCommandType Type;
		RendererId ObjectId;
	};


	/**
	 * Replays renderer object ID allocation/deallocation commands on the render thread. This ensures the render thread can maintain packed arrays
	 * of renderer objects, while the main thread can freely allocate/deallocate RendererId values without needing to know about slots or packed array management.
	 *
	 * @tparam ClearFn		Callable(PackedRendererId) — called on the entry being deallocated, before swapping.
	 * @tparam SwappedFn	Callable(PackedRendererId) — called on the entry after swapping (arrays now hold new data at this position). Only called if swap occurred.
	 * @tparam Arrays		One or multiple packed array that hold the data associated with the renderer objects. This data will be moved to ensure they arrays remain packed.
	 *						Must be vector-like containers that support push_back, pop_back, operator[], std::swap.
	 */
	template<typename ClearFn, typename SwappedFn, typename... Arrays>
	void ReplayRendererIdCommands(const RendererIdCommand* commands, u32 commandCount, Vector<RendererId>& sparseSlots, Vector<RendererId>& denseToSparse, ClearFn&& fnClear, SwappedFn&& fnSwapped, Arrays&... arrays)
	{
		for(u32 commandIndex = 0; commandIndex < commandCount; ++commandIndex)
		{
			const RendererIdCommand& command = commands[commandIndex];
			const u32 identifier = command.ObjectId.GetIdentifier();

			if(command.Type == RendererIdCommandType::Allocate)
			{
				const PackedRendererId newSlot = (PackedRendererId)denseToSparse.size();

				// Grow sparse array if needed
				if(identifier >= (u32)sparseSlots.size())
					sparseSlots.resize(identifier + 1);

				sparseSlots[identifier] = RendererId((RendererId::IdentifierType)newSlot, (RendererId::VersionType)command.ObjectId.GetVersion());

				denseToSparse.push_back(command.ObjectId);
				(arrays.push_back(typename std::remove_reference_t<decltype(arrays)>::value_type{}), ...);
			}
			else
			{
				B3D_ASSERT(identifier < (u32)sparseSlots.size());
				B3D_ASSERT(sparseSlots[identifier].GetVersion() == command.ObjectId.GetVersion());

				const PackedRendererId slotToRemove = (PackedRendererId)sparseSlots[identifier].GetIdentifier();

				fnClear(slotToRemove);

				const PackedRendererId lastSlot = (PackedRendererId)denseToSparse.size() - 1;
				if(slotToRemove != lastSlot)
				{
					// Swap-and-pop: move last element into the removed slot
					(std::swap(arrays[slotToRemove], arrays[lastSlot]), ...);
					std::swap(denseToSparse[slotToRemove], denseToSparse[lastSlot]);

					// Update the swapped object's sparse entry to point to its new slot
					const u32 swappedIdentifier = denseToSparse[slotToRemove].GetIdentifier();
					sparseSlots[swappedIdentifier].Identifier = slotToRemove;

					fnSwapped(slotToRemove);
				}

				denseToSparse.pop_back();
				(arrays.pop_back(), ...);

				sparseSlots[identifier] = kInvalidRendererId;
			}
		}
	}

	/**
	 * Processes provided Deallocate commands. Swaps the provided arrays so that all removed entries are moved to the tail of the arrays,
	 * and returns the index past the last non-deallocated entry (logical size). This allows the caller to batch-notify and resize arrays
	 * after processing all deallocations.
	 *
	 * @param commands			Array of deallocation commands to process.
	 * @param sparseSlots		Sparse array mapping RendererId identifiers to PackedRendererId slots. This will be updated to point to new slots after swapping, and invalidated for removed entries.
	 * @param denseToSparse		Dense array mapping PackedRendererId slots to RendererId values. This will be updated to point to new slots after swapping, and have removed entries swapped to the tail.
	 * @param fnSwapped			Callable(PackedRendererId) — Called on each entry that was swapped into a new slot.
	 * @param arrays			One or multiple packed arrays to swap. Must support operator[] and std::swap. These will be swapped according to the new slot positions.
	 * @return					Index past the last non-deallocated entry (logical size). All entries at and past this index are deallocated and can be popped from @p sparseSlots and @p arrays.
	 */
	template<typename SwappedFn, typename... Arrays>
	PackedRendererId ProcessRendererObjectDeallocations(TArrayView<const RendererIdCommand> commands, Vector<RendererId>& sparseSlots, Vector<RendererId>& denseToSparse, SwappedFn&& fnSwapped, Arrays&... arrays)
	{
		PackedRendererId logicalSize = (PackedRendererId)denseToSparse.size();

		for(u32 commandIndex = 0; commandIndex < commands.Size(); ++commandIndex)
		{
			const RendererIdCommand& command = commands[commandIndex];
			const u32 identifier = command.ObjectId.GetIdentifier();

			B3D_ASSERT(identifier < (u32)sparseSlots.size());
			B3D_ASSERT(sparseSlots[identifier].GetVersion() == command.ObjectId.GetVersion());

			const PackedRendererId slotToRemove = (PackedRendererId)sparseSlots[identifier].GetIdentifier();

			--logicalSize;
			if(slotToRemove != logicalSize)
			{
				// Swap into the "to-be-removed" zone at the tail
				(std::swap(arrays[slotToRemove], arrays[logicalSize]), ...);
				std::swap(denseToSparse[slotToRemove], denseToSparse[logicalSize]);

				// Update the swapped element's sparse entry to point to its new slot
				const u32 swappedIdentifier = denseToSparse[slotToRemove].GetIdentifier();
				sparseSlots[swappedIdentifier].Identifier = slotToRemove;

				fnSwapped(slotToRemove);
			}

			// Invalidate the removed element's sparse entry
			sparseSlots[identifier] = kInvalidRendererId;
		}

		return logicalSize;
	}

	/**
	 * Processes provided Allocate commands, growing sparse/dense arrays and pushing default-constructed entries to the provided packed arrays.
	 *
	 * @param commands			Array of allocation commands to process.
	 * @param sparseSlots		Sparse array mapping RendererId identifiers to PackedRendererId slots. This will be updated to point to new slots for allocated entries.
	 * @param denseToSparse		Dense array mapping PackedRendererId slots to RendererId values. This will be appended with new entries for allocated objects.
	 * @param arrays			One or multiple packed arrays to grow. Must support push_back with default-constructed value_type. These will be appended with default-constructed entries for allocated objects.
	 */
	template<typename... Arrays>
	void ProcessRendererObjectAllocations(TArrayView<const RendererIdCommand> commands, Vector<RendererId>& sparseSlots, Vector<RendererId>& denseToSparse, Arrays&... arrays)
	{
		for(u32 commandIndex = 0; commandIndex < commands.Size(); ++commandIndex)
		{
			const RendererIdCommand& command = commands[commandIndex];
			const u32 identifier = command.ObjectId.GetIdentifier();

			const PackedRendererId newSlot = (PackedRendererId)denseToSparse.size();

			// Grow sparse array if needed
			if(identifier >= (u32)sparseSlots.size())
				sparseSlots.resize(identifier + 1);

			sparseSlots[identifier] = RendererId((RendererId::IdentifierType)newSlot, (RendererId::VersionType)command.ObjectId.GetVersion());

			denseToSparse.push_back(command.ObjectId);
			(arrays.push_back(typename std::remove_reference_t<decltype(arrays)>::value_type{}), ...);
		}
	}

	/**
	 * Provides render thread storage for renderer objects, and synchronization of object data from the main thread into that storage.
	 *
	 * On the main thread renderer objects are represented as ECS entities, with ECS fragments for storing data. On the render thread the objects are stored
	 * in packed arrays referenced by PackedRendererIds. Persistent RendererId values are allocated from the main thread and stored on the ECS entity, and then
	 * resolved on the render thread to packed PackedRendererId values for array access.
	 *
	 * # Allocating renderer object IDs
	 * Renderer object IDs are allocated / deallocated from the main thread using AllocateRendererId / DeallocateRendererId.
	 *
	 * Since RendererId values are allocated/deallocated from the main thread, the render thread needs to be informed about which IDs were allocated/deallocated
	 * so it can update its packed arrays to match. This is done via FlushCommands/ApplyCommands(), which replays the allocation/deallocation
	 * commands recorded by the main thread on the render thread.
	 *
	 * It is expected for the main thread to call FlushCommands during SyncRead(), and render thread to call ApplyCommands during SyncWrite().
	 * This should be called as the first step during SyncWrite().
	 *
	 * # Syncing data
	 * Data should be synced from the main thread to the render thread using SyncRead() / SyncWrite().
	 *  - SyncRead is called on the main thread, and should gather dirty data from ECS fragments associated with the entity into a batch buffer allocated by the
	 *    provided allocator, and return a pointer to that buffer. It must call FlushCommands() to ensure SyncWrite() can replay the commands.
	 *  - SyncWrite is called on the render thread with the batch buffer generated by SyncRead. It must call ApplyCommands() first to
	 *    ensure packed arrays are up to date, then resolve RendererId values to PackedRendererId via GetPackedRendererId() for array access.
	 */
	class B3D_EXPORT RendererObjectStorage : public IRendererObjectSyncHandler
	{
	public:
		virtual ~RendererObjectStorage() = default;

		/** Result of FlushCommands — views into FrameAllocator-backed deallocation and allocation command arrays. */
		struct FlushedCommands
		{
			TArrayView<const RendererIdCommand> Deallocations;
			TArrayView<const RendererIdCommand> Allocations;
		};

		/**
		 * Allocates a persistent renderer object ID.
		 *
		 * @note Main thread only.
		 */
		RendererId AllocateRendererId();

		/**
		 * Deallocates a persistent renderer object ID.
		 *
		 * @note Main thread only.
		 */
		void DeallocateRendererId(RendererId objectId);

		/**
		 * Resolves a RendererId to the current packed PackedRendererId on the render thread.
		 * Returns kInvalidPackedRendererId if the ID is stale or out of range.
		 *
		 * @note Render thread only.
		 */
		PackedRendererId GetPackedRendererId(RendererId objectId) const
		{
			u32 identifier = objectId.GetIdentifier();
			if(identifier >= (u32)mSparseSlots.size())
				return kInvalidPackedRendererId;

			const RendererId& entry = mSparseSlots[identifier];
			if(entry.GetVersion() != objectId.GetVersion())
				return kInvalidPackedRendererId;

			return (PackedRendererId)entry.GetIdentifier();
		}

	protected:
		/**
		 * Applies flushed renderer object commands on the render thread, updating packed arrays and freeing the command buffers.
		 * Handles the full cycle: deallocations are swapped to the tail, destroyed via @p fnDestroy, arrays are resized,
		 * then allocations are appended. Finally, the command buffers are freed from the allocator.
		 *
		 * @param commands		Flushed allocation/deallocation commands to apply.
		 * @param allocator		FrameAllocator used to free the command buffers.
		 * @param fnSwapped		Callable(PackedRendererId) — called on each entry that was swapped into a new slot during deallocation.
		 * @param fnDestroy		Callable(TArrayView<const PackedRendererId>) — called with the slot IDs of all deallocated entries (at the tail) before they are resized away.
		 * @param arrays		Packed arrays to manage. Must support operator[], resize, push_back, and std::swap.
		 */
		template<typename SwappedFn, typename DestroyFn, typename... Arrays>
		void ApplyCommands(const FlushedCommands& commands, FrameAllocator& allocator, SwappedFn&& fnSwapped, DestroyFn&& fnDestroy, Arrays&... arrays)
		{
			if(commands.Deallocations.Size() > 0)
			{
				const PackedRendererId originalSize = (PackedRendererId)mDenseToSparse.size();

				PackedRendererId logicalSize = ProcessRendererObjectDeallocations(commands.Deallocations, mSparseSlots, mDenseToSparse,
					std::forward<SwappedFn>(fnSwapped), arrays...);

				const u32 removedCount = originalSize - logicalSize;

				FrameAllocatorScope frameAllocatorScope;
				FrameVector<PackedRendererId> deallocSlotIds(removedCount);
				for(u32 slotIndex = 0; slotIndex < removedCount; ++slotIndex)
					deallocSlotIds[slotIndex] = logicalSize + slotIndex;

				fnDestroy(TArrayView<const PackedRendererId>(deallocSlotIds.data(), removedCount));

				(arrays.resize(logicalSize), ...);
				mDenseToSparse.resize(logicalSize);
			}

			if(commands.Allocations.Size() > 0)
				ProcessRendererObjectAllocations(commands.Allocations, mSparseSlots, mDenseToSparse, arrays...);

			if(commands.Deallocations.Data())
				allocator.Free(reinterpret_cast<u8*>(const_cast<RendererIdCommand*>(commands.Deallocations.Data())));

			if(commands.Allocations.Data())
				allocator.Free(reinterpret_cast<u8*>(const_cast<RendererIdCommand*>(commands.Allocations.Data())));
		}

		/** Consumes pending allocations and deallocations, returning views into FrameAllocator-backed arrays. */
		FlushedCommands FlushCommands(FrameAllocator& allocator);

		// Main thread
		RendererIdAllocator mObjectIdAllocator;

		TUnorderedSet<RendererId> mPendingAllocations; /**< Allocations this frame, not yet flushed. */
		TArray<RendererId> mDeallocations; /**< Deallocations this frame, not yet flushed. */

		// Render thread
		Vector<RendererId> mSparseSlots; /**< RendererId -> PackedRendererId. */
		Vector<RendererId> mDenseToSparse; /**< PackedRendererId -> RendererId. */
	};

	/** @} */
} // namespace b3d
