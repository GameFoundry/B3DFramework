//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIIteratorField.h"
#include "Reflection/B3DRTTIECSIterator.h"
#include "ECS/B3DIECSEntityOwner.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * RTTI field type that reads/writes ECS data directly from/to an ecs::Registry.
	 *
	 * Auto-detects whether DataType is a TagGroup or a data component at compile time
	 * and selects the appropriate iterator. For tag groups, the serialized element is the
	 * StorageType bitfield. For fragments, it is the component type itself.
	 *
	 * The owner type must implement ecs::IECSEntityOwner to provide registry and entity access.
	 *
	 * @tparam DataType     ECS data type -- either a component struct (e.g. ecs::WorldTransform)
	 *                      or a TagGroup<Storage, Tags...> specialization (e.g. ecs::MobilityTags).
	 * @tparam OwnerType    Type of the object that owns the ECS entity.
	 */
	template<typename DataType, typename OwnerType>
	struct TRTTIECSField : public RTTIIteratorField
	{
		static_assert(std::is_base_of_v<ecs::IECSEntityOwner, OwnerType>, "OwnerType must implement ecs::IECSEntityOwner interface");

		using IteratorType = std::conditional_t<ecs::IsTagGroup<DataType>::value,
			TRTTIECSTagGroupIterator<DataType>,
			TRTTIECSFragmentIterator<DataType>>;
		using ElementType = typename IteratorType::ElementType;

		TRTTIECSField(String name, u16 uniqueId, const RTTIFieldInfo& info)
		{
			this->Name = std::move(name);
			this->Schema = RTTIFieldSchema(uniqueId, false, RTTIFieldType::Iterable, info);
		}

		void InitSchema() override
		{
			this->Schema.FieldDataTypes.Add(detail::CreateFieldTypeSchema<std::remove_cv_t<ElementType>>(this->Schema.Info));
		}

		TShared<IRTTIIterator> GetIterator(b3d::RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator) const override
		{
			OwnerType* owner = static_cast<OwnerType*>(object);
			auto* it = frameAllocator.Construct<IteratorType>(owner->GetECSRegistry(), owner->GetECSEntity());
			return B3DMakeSharedFromExisting<IteratorType>(it, TRTTIECSIteratorDeleter<IteratorType>(&frameAllocator));
		}

		bool IteratorSupportsSeekToIndex() const override { return false; }
		bool IteratorSupportsSeekToKey() const override { return false; }

		const void* GetIteratorValue(b3d::RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator) const override
		{
			return iterator.GetValue();
		}

		void* GetIteratorValueCopy(b3d::RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator) const override
		{
			return frameAllocator.Construct<ElementType>(*static_cast<const ElementType*>(GetIteratorValue(rttiTypeInstance, object, frameAllocator, iterator)));
		}

		void SetIteratorValue(b3d::RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator, void* value) override
		{
			static_cast<IteratorType&>(iterator).WriteValue(*static_cast<const ElementType*>(value));
		}

		void* CreateEmptyFieldValue(FrameAllocator& frameAllocator) override
		{
			return frameAllocator.Construct<ElementType>();
		}

		void FreeFieldValue(void* fieldValue, FrameAllocator& frameAllocator) override
		{
			if(fieldValue != nullptr)
				frameAllocator.Destruct(static_cast<ElementType*>(fieldValue));
		}

		void ReadPlainTypeTupleFromStream(void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			using MutableType = std::remove_cv_t<ElementType>;
			detail::ReadPlainType(const_cast<MutableType&>(*static_cast<MutableType*>(fieldValue)), stream, Schema.Info, useCompression);
		}

		void WritePlainTypeTupleToStream(const void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			detail::WritePlainType(*static_cast<const ElementType*>(fieldValue), stream, Schema.Info, useCompression);
		}

		BitLength GetPlainTypeSize(const void* fieldValue, u32 tupleElementIndex, bool useCompression) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			return detail::GetPlainTypeSize(*static_cast<const ElementType*>(fieldValue), Schema.Info, useCompression);
		}

		void SetReflectable(void* fieldValue, u32 tupleElementIndex, const IReflectable& reflectable) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			using MutableType = std::remove_cv_t<ElementType>;
			detail::SetReflectableValue(const_cast<MutableType&>(*static_cast<MutableType*>(fieldValue)), reflectable);
		}

		const IReflectable& GetReflectable(const void* fieldValue, u32 tupleElementIndex) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			return detail::GetReflectableValue(*static_cast<const ElementType*>(fieldValue));
		}

		void SetReflectablePointer(void* fieldValue, u32 tupleElementIndex, const TShared<IReflectable>& reflectable) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			using MutableType = std::remove_cv_t<ElementType>;
			detail::SetReflectablePointerValue(const_cast<MutableType&>(*static_cast<MutableType*>(fieldValue)), reflectable);
		}

		TShared<IReflectable> GetReflectablePointer(const void* fieldValue, u32 tupleElementIndex) override
		{
			B3D_ASSERT(tupleElementIndex == 0);
			return detail::GetReflectablePointerValue(*static_cast<const ElementType*>(fieldValue));
		}
	};

	/** @} */
	/** @} */
} // namespace b3d
