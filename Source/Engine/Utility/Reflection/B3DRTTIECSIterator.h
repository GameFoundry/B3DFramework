//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "ECS/B3DRegistry.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DECSTagGroup.h"
#include "Reflection/B3DRTTIIterator.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Deleter for ECS RTTI iterators allocated via FrameAllocator. */
	template<typename ConcreteIterator>
	struct TRTTIECSIteratorDeleter
	{
		TRTTIECSIteratorDeleter(FrameAllocator* allocator = nullptr)
			: mAllocator(allocator)
		{ }

		void operator()(ConcreteIterator* iterator)
		{
			if(B3D_ENSURE(mAllocator != nullptr))
				mAllocator->Destruct(iterator);
		}

	private:
		FrameAllocator* mAllocator;
	};

	/**
	 * Base class for ECS RTTI iterators. Provides the faux single-element iterator
	 * behavior (always 1 element) used by both fragment and tag group iterators.
	 */
	class RTTIECSIterator : public IRTTIIterator
	{
	public:
		RTTIECSIterator(ecs::Registry* registry, ecs::Entity entity)
			: mRegistry(registry), mEntity(entity)
		{ }

		bool IsValid() const override { return mIsValid; }
		u64 GetElementCount() const override { return 1; }
		void SeekToBeginning() override { mIsValid = true; }
		void SeekToEnd() override { mIsValid = false; }
		void Increment() override { mIsValid = false; }
		bool SeekToIndex(u64 index) override { mIsValid = (index == 0); return mIsValid; }
		bool SeekToKey(const void*) override { return false; }
		void Erase() override { }
		void Clear() override { }

	protected:
		ecs::Registry* mRegistry = nullptr;
		ecs::Entity mEntity = ecs::kNullEntity;
		bool mIsValid = true;
	};

	/**
	 * RTTI iterator for a single ECS fragment (data component). Reads directly from ECS storage
	 * and writes via AddOrReplaceComponent.
	 */
	template<typename ComponentType>
	class TRTTIECSFragmentIterator : public RTTIECSIterator
	{
	public:
		using ElementType = ComponentType;

		TRTTIECSFragmentIterator(ecs::Registry* registry, ecs::Entity entity)
			: RTTIECSIterator(registry, entity)
		{ }

		const void* GetValue() const override
		{
			if(mRegistry != nullptr && mEntity != ecs::kNullEntity && mRegistry->HasAllOf<ComponentType>(mEntity))
				return &mRegistry->GetComponents<ComponentType>(mEntity);

			B3D_ASSERT(false);
			return nullptr;
		}

		SPtr<IRTTIIterator> Clone(FrameAllocator& allocator) const override
		{
			auto* clone = allocator.Construct<TRTTIECSFragmentIterator>(mRegistry, mEntity);
			clone->mIsValid = mIsValid;
			return B3DMakeSharedFromExisting<TRTTIECSFragmentIterator>(clone, TRTTIECSIteratorDeleter<TRTTIECSFragmentIterator>(&allocator));
		}

		/** Writes a component value to the ECS registry via AddOrReplaceComponent. */
		void WriteValue(const ElementType& value)
		{
			if(mRegistry != nullptr && mEntity != ecs::kNullEntity)
			{
				ComponentType copy = value;
				mRegistry->AddOrReplaceComponent<ComponentType>(mEntity, std::move(copy));
			}
		}
	};

	/**
	 * RTTI iterator for an ECS tag group. Synthesizes a bitfield on read (bit N set if
	 * Nth tag is present) and applies tag presence/absence on write.
	 */
	template<typename TagGroupType>
	class TRTTIECSTagGroupIterator;

	template<typename Storage, typename... TagTypes>
	class TRTTIECSTagGroupIterator<ecs::TagGroup<Storage, TagTypes...>> : public RTTIECSIterator
	{
		using TagGroupType = ecs::TagGroup<Storage, TagTypes...>;

	public:
		using ElementType = Storage;

		TRTTIECSTagGroupIterator(ecs::Registry* registry, ecs::Entity entity)
			: RTTIECSIterator(registry, entity)
		{
			SynthesizeBitfield();
		}

		const void* GetValue() const override
		{
			return &mValue;
		}

		SPtr<IRTTIIterator> Clone(FrameAllocator& allocator) const override
		{
			auto* clone = allocator.Construct<TRTTIECSTagGroupIterator>(mRegistry, mEntity);
			clone->mIsValid = mIsValid;
			clone->mValue = mValue;
			return B3DMakeSharedFromExisting<TRTTIECSTagGroupIterator>(clone, TRTTIECSIteratorDeleter<TRTTIECSTagGroupIterator>(&allocator));
		}

		/** Applies tag presence/absence to the entity based on the bitfield value. */
		void WriteValue(const ElementType& bitfield)
		{
			if(mRegistry == nullptr || mEntity == ecs::kNullEntity)
				return;

			WriteTagsForIndices(bitfield, std::index_sequence_for<TagTypes...>{});
		}

	private:
		/** Reads current tag state from registry into mValue. */
		void SynthesizeBitfield()
		{
			mValue = ElementType{0};
			if(mRegistry == nullptr || mEntity == ecs::kNullEntity)
				return;

			SynthesizeBitfieldFromIndices(std::index_sequence_for<TagTypes...>{});
		}

		template<std::size_t... Indices>
		void SynthesizeBitfieldFromIndices(std::index_sequence<Indices...>)
		{
			((mRegistry->HasAllOf<TagTypes>(mEntity) ? (mValue |= (ElementType{1} << Indices)) : ElementType{0}), ...);
		}

		template<std::size_t... Indices>
		void WriteTagsForIndices(ElementType bitfield, std::index_sequence<Indices...>)
		{
			(WriteTag<TagTypes>(bitfield, Indices), ...);
		}

		template<typename TagType>
		void WriteTag(ElementType bitfield, std::size_t bitIndex)
		{
			const bool shouldBePresent = (bitfield & (ElementType{1} << bitIndex)) != 0;
			const bool isPresent = mRegistry->HasAllOf<TagType>(mEntity);

			if(shouldBePresent && !isPresent)
				mRegistry->AddTag<TagType>(mEntity);
			else if(!shouldBePresent && isPresent)
				mRegistry->RemoveTag<TagType>(mEntity);
		}

		mutable ElementType mValue = 0;
	};

	/** @} */
	/** @} */
} // namespace b3d
