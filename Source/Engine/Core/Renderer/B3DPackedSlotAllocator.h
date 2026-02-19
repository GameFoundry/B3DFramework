//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "ECS/B3DRegistry.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

#define B3D_64BIT_SLOT_IDS 0

#if B3D_64BIT_SLOT_IDS
	using SlotId = u64;
#else
	using SlotId = u32;
#endif

	/** Invalid slot ID sentinel value (all bits set). */
	constexpr SlotId kInvalidSlotId = ~SlotId(0);

	/** Type of operation recorded by a packed slot allocator. */
	enum class SlotCommandType : u8
	{
		Allocate,
		Deallocate
	};

	/** Records a single allocation or deallocation in packed arrays managed by a TPackedSlotAllocator. */
	struct SlotCommand
	{
		SlotCommandType Type;
		SlotId TargetSlotId;
		SlotId LastSlotId; /**< Only meaningful for deallocation — the slot that was swapped into SlotId. */
	};

	/**
	 * Allocates packed slot IDs on the main thread and records allocate/deallocate commands so they can
	 * later be replayed on the render thread.
	 *
	 * All slots are recorded as ECS fragments on the provided entity. Maintains a mapping between slot ID's and ECS entities.
	 * The slots are not stable and may change with each Allocate/Deallocate call, even for entities that are not directly touched.
	 *
	 * @tparam IdFragment	ECS fragment type that stores the packed ID (must have a `SlotId Id` field).
	 */
	template<typename IdFragment>
	class TPackedSlotAllocator
	{
	public:
		/**
		 * Allocates a new slot for the given entity. Caller must ensure the entity does not have a slot allocated.
		 *
		 * @return The packed slot index assigned to the entity.
		 */
		SlotId Allocate(ecs::Entity entity)
		{
			const SlotId slotId = (SlotId)mIdToEntity.size();

			mIdToEntity.Add(entity);
			mEntityToId[entity.IdentifierAndVersion] = slotId;

			SlotCommand command;
			command.Type = SlotCommandType::Allocate;
			command.TargetSlotId = slotId;
			command.LastSlotId = 0;

			mCommandBuffers[mActiveBuffer].Add(command);

			return slotId;
		}

		/**
		 * Deallocates the slot belonging to the given entity. Uses swap-and-pop internally and updates the
		 * slot ID of the entity that was swapped into the deallocated entity's slot.
		 */
		void Deallocate(ecs::Entity entity, ecs::Registry& registry)
		{
			auto found = mEntityToId.find(entity.IdentifierAndVersion);
			B3D_ASSERT(found != mEntityToId.end());

			const SlotId slotId = found->second;
			const SlotId lastSlotId = (SlotId)mIdToEntity.size() - 1;

			if(slotId != lastSlotId)
			{
				const ecs::Entity swappedEntity = mIdToEntity[lastSlotId];

				mIdToEntity[slotId] = swappedEntity;
				mEntityToId[swappedEntity.IdentifierAndVersion] = slotId;

				registry.GetComponents<IdFragment>(swappedEntity).Id = slotId;
			}

			mIdToEntity.Pop();
			mEntityToId.erase(found);

			SlotCommand command;
			command.Type = SlotCommandType::Deallocate;
			command.TargetSlotId = slotId;
			command.LastSlotId = lastSlotId;

			mCommandBuffers[mActiveBuffer].Add(command);
		}

		/**
		 * Returns the currently recorded slot commands, and swaps the buffers internally so new allocate/deallocate
		 * calls are recorded into a new commands array.
		 */
		const TArray<SlotCommand>& ConsumeCommands()
		{
			const u32 consumedBufferIndex = mActiveBuffer;
			mActiveBuffer = 1 - mActiveBuffer;
			mCommandBuffers[mActiveBuffer].clear();

			return mCommandBuffers[consumedBufferIndex];
		}

	private:
		TArray<ecs::Entity> mIdToEntity;
		UnorderedMap<ecs::Entity::StorageType, SlotId> mEntityToId;
		TArray<SlotCommand> mCommandBuffers[2];
		u32 mActiveBuffer = 0;
	};

	/**
	 * Helper that replays recorded slot commands against a set of packed arrays on the render thread.
	 *
	 * @tparam ClearFn     Callable(SlotId) — Called on the slot that was deallocated, before swapping. Only relevant for deallocate commands.
	 * @tparam SwappedFn   Callable(SlotId) — Called on the slot that was deallocated, after swapping (slot arrays now holds new data at this position). Only relevant for deallocate commands. Only called if swap occurred.
	 * @tparam Arrays      Vector-like containers that support push_back, pop_back, operator[], std::swap.
	 */
	template<typename ClearFn, typename SwappedFn, typename... Arrays>
	void ReplaySlotCommands(const SlotCommand* commands, u32 commandCount, ClearFn&& fnClear, SwappedFn&& fnSwapped, Arrays&... arrays)
	{
		for(u32 commandIndex = 0; commandIndex < commandCount; ++commandIndex)
		{
			const SlotCommand& command = commands[commandIndex];

			if(command.Type == SlotCommandType::Allocate)
				(arrays.push_back(typename std::remove_reference_t<decltype(arrays)>::value_type{}), ...);
			else
			{
				fnClear(command.TargetSlotId);

				if(command.TargetSlotId != command.LastSlotId)
				{
					(std::swap(arrays[command.TargetSlotId], arrays[command.LastSlotId]), ...);
					fnSwapped(command.TargetSlotId);
				}

				(arrays.pop_back(), ...);
			}
		}
	}

	/** @} */
} // namespace b3d
