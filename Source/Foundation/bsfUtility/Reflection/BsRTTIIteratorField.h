//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Provides interface to be implemented by specializations of TRTTIIteratorField. */
	struct RTTIIteratorField : public RTTIField
	{
		virtual ~RTTIIteratorField() = default;

		/** Returns the iterator that can be used for iterating all entries in the field. */
		virtual UPtr<IRTTIIterator> GetIterator(RTTITypeBase* rttiType, void* object, FrameAllocator& frameAllocator) const = 0;

		/** Returns the current value of the iterator. */
		virtual const void* GetIteratorValue(RTTITypeBase* rttiType, void* object, FrameAllocator& frameAllocator, const IRTTIIterator& iterator) const = 0;

		/**
		 * Appends a new value at the end of the iterator, and increments the iterator. @p value must have been created by a call to CreateEmptyFieldValue(),
		 * using the same @p frameAllocator.
		 */
		virtual void SetIteratorValue(RTTITypeBase* rttiType, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator, void* value) = 0;

		/**
		 * Creates a new empty field value. This should be populated by calls to WritePlainTypeTupleToStream, SetReflectablePointer or SetReflectable, and then
		 * passed to SetIteratorValue().
		 */
		virtual void* CreateEmptyFieldValue(FrameAllocator& frameAllocator);
		
		/**
		 * Reads into the provided field value from the stream. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this reads just a single tuple element, as specified by @p tupleElementIndex.
		 *
		 * @param	fieldValue			Field value to read into, as created by CreateEmptyFieldValue().
		 * @param	tupleElementIndex	Index of the tuple element in @p fieldValue to read into. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @param	stream				Stream from which to read from.
		 * @param	useCompression		If true, values will read as compressed where relevant. Must be set of the value was written with compression enabled.
		 */
		virtual void ReadPlainTypeTupleFromStream(void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) = 0;

		/**
		 * Writes the provided field value to the stream. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this encodes just a single tuple element, as specified by @p tupleElementIndex.
		 *
		 * @param	fieldValue			Field value to write, as returned by GetIteratorValue().
		 * @param	tupleElementIndex	Index of the tuple element in @p fieldValue to write. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @param	stream				Stream in which to write to.
		 * @param	useCompression		If true, values will attempt to be compressed where relevant. Must be decoded with this flag in the same state.
		 */
		virtual void WritePlainTypeTupleToStream(const void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) = 0;

		/**
		 * Assigns the reflectable pointer to the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this assigns a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p reflectable is simply assigned to @p fieldValue.
		 *
		 * @param fieldValue		Field value in which to store the reflectable pointer, as created by CreateEmptyFieldValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue in which to store the reflectable pointer. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @param reflectable		Reflectable pointer to assign to the field value.
		 */
		virtual void SetReflectablePointer(const void* fieldValue, u32 tupleElementIndex, const SPtr<IReflectable>& reflectable) = 0;

		/**
		 * Reads the reflectable pointer from the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this reads a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p fieldValue is simply cast to a reflectable pointer type.
		 *
		 * @param fieldValue		Field value containing the reflectable pointer, as returned by GetIteratorValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue which contains the reflectable pointer. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @return					Reflectable pointer.
		 */
		virtual SPtr<IReflectable> GetReflectablePointer(const void* fieldValue, u32 tupleElementIndex) = 0;

		/**
		 * Assigns the reflectable to the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this assigns a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p reflectable is simply assigned to @p fieldValue.
		 *
		 * @param fieldValue		Field value in which to store the reflectable, as created by CreateEmptyFieldValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue in which to store the reflectable. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @param reflectable		Reflectable to assign to the field value.
		 */
		virtual void SetReflectable(const void* fieldValue, u32 tupleElementIndex, const IReflectable& reflectable) = 0;

		/**
		 * Reads the reflectable value from the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this reads a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p fieldValue is simply cast to a reflectable type.
		 *
		 * @param fieldValue		Field value containing the reflectable, as returned by GetIteratorValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue which contains the reflectable. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @return					Reflectable.
		 */
		virtual const IReflectable& GetReflectable(const void* fieldValue, u32 tupleElementIndex) = 0;
	};

	template<typename T>
	struct B3DIsStdPair : std::false_type { };

	template<typename KeyType, typename ValueType>
	struct B3DIsStdPair<std::pair<KeyType, ValueType>> : std::true_type { };

	/**
	 * RTTI field type that supports iteration over arbitrary containers, including maps. Unlike older field types, this field type internally handles plain, reflectable and reflectable pointer types,
	 * rather than requiring a separate field type for each.
	 *
	 * @tparam RTTIType			RTTIType of the object that contains the field we're accessing.
	 * @tparam ContainerType	Type of the container being iterated over.
	 * @tparam ObjectType		Type of the object that the field is a member of.
	 */
	template <class RTTIType, class ContainerType, class ObjectType>
	struct TRTTIIteratorField : public RTTIIteratorField // TODO - This type should be re-used for non-container types as well (treat the iterator as a single entry array)
	{
		using ElementType = typename ContainerType::value_type;

		typedef UPtr<TRTTIIterator<ContainerType>> (RTTIType::*GetIteratorDelegate)(ObjectType*, FrameAllocator&);
		typedef const ElementType& (RTTIType::*GetValueDelegate)(ObjectType*, FrameAllocator&, TRTTIIterator<ContainerType>&);
		typedef void (RTTIType::*SetValueDelegate)(ObjectType*, FrameAllocator&, TRTTIIterator<ContainerType>&, const ElementType&);

		TRTTIIteratorField(String name, u16 uniqueId, GetIteratorDelegate getIteratorCallback, GetValueDelegate getValueCallback, SetValueDelegate setValueCallback, const RTTIFieldInfo& fieldInfo)
		{
			B3D_ASSERT(getIteratorCallback != nullptr);
			B3D_ASSERT(getValueCallback != nullptr);
			B3D_ASSERT(setValueCallback != nullptr);

			this->mGetIteratorCallback = getIteratorCallback;
			this->mGetValueCallback = getValueCallback;
			this->mSetValueCallback = setValueCallback;

			this->Name = std::move(name);
			this->Schema = RTTIFieldSchema(uniqueId, false, false, 0, SerializableFT_Plain, 0, nullptr, fieldInfo);
		}

		void InitSchema() override
		{
			// Special case for pairs so we natively support reflectable types in maps (This could technically be extended to support std::tuple as well if needed)
			if constexpr(B3DIsStdPair<ElementType>::value_type)
			{
				this->Schema.FieldTypes.Add(CreateFieldTypeSchema<typename ElementType::first_type>(this->Schema.Info));
				this->Schema.FieldTypes.Add(CreateFieldTypeSchema<typename ElementType::second_type>(this->Schema.Info));
			}
			else
			{
				this->Schema.FieldTypes.Add(CreateFieldTypeSchema<ElementType>(this->Schema.Info));
			}
		}

		UPtr<IRTTIIterator> GetIterator(RTTITypeBase* rttiType, void* object, FrameAllocator& frameAllocator) const override
		{
			RTTIType* const exactRttiType = static_cast<RTTIType*>(rttiType);
			ObjectType* const exactObject = static_cast<ObjectType*>(object);

			return (exactRttiType->*mGetIteratorCallback)(exactObject, frameAllocator);
		}

		const void* GetIteratorValue(RTTITypeBase* rttiType, void* object, FrameAllocator& frameAllocator, const IRTTIIterator& iterator) const override
		{
			RTTIType* const exactRttiType = static_cast<RTTIType*>(rttiType);
			ObjectType* const exactObject = static_cast<ObjectType*>(object);

			return &(exactRttiType->*mGetValueCallback)(exactObject, frameAllocator, iterator);
		}

		void SetIteratorValue(RTTITypeBase* rttiType, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator, void* value) override
		{
			RTTIType* const exactRttiType = static_cast<RTTIType*>(rttiType);
			ObjectType* const exactObject = static_cast<ObjectType*>(object);

			ElementType& exactValue = *static_cast<ElementType*>(value);
			(exactRttiType->*mSetValueCallback)(exactObject, frameAllocator, iterator, exactValue);

			frameAllocator.Destruct(&exactValue);
		}

		void* CreateEmptyFieldValue(FrameAllocator& frameAllocator) override
		{
			return frameAllocator.Construct<ElementType>();
		}

		void ReadPlainTypeTupleFromStream(void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) override
		{
			ElementType& value = *static_cast<ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value_type) // Note: Currently supporting just std::pair, but can support other types eventually
			{
				if(!B3D_ENSURE(tupleElementIndex <= 1))
					return;

				if(tupleElementIndex == 0)
					ReadPlainTypeFromStream(value.first, stream, useCompression);
				else
					ReadPlainTypeFromStream(value.second, stream, useCompression);
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				ReadPlainTypeFromStream(value, stream, useCompression);
			}
		}

		void WritePlainTypeTupleToStream(const void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value_type)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					WritePlainTypeToStream(value.first, stream, useCompression);
				else
					WritePlainTypeToStream(value.second, stream, useCompression);
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				WritePlainTypeToStream(value, stream, useCompression);
			}
		}

		void SetReflectablePointer(const void* fieldValue, u32 tupleElementIndex, const SPtr<IReflectable>& reflectable) override
		{
			ElementType& value = *static_cast<ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value_type)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					value.first = reflectable;
				else
					value.second = reflectable;
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				value = reflectable;
			}
		}

		SPtr<IReflectable> GetReflectablePointer(const void* fieldValue, u32 tupleElementIndex) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value_type)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					return value.first;

				return value.second;
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				return value;
			}
		}

		void SetReflectable(const void* fieldValue, u32 tupleElementIndex, const IReflectable& reflectable) override
		{
			ElementType& value = *static_cast<ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value_type)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					value.first = reflectable;
				else
					value.second = reflectable;
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				value = reflectable;
			}
		}

		const IReflectable& GetReflectable(const void* fieldValue, u32 tupleElementIndex) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value_type)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					return value.first;

				return value.second;
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				return value;
			}
		}

	private:
		/** Creates a schema for a type stored in the field. */
		template<typename FieldType>
		RTTIFieldTypeSchema CreateFieldTypeSchema(const RTTIFieldInfo& fieldInfo)
		{
			if constexpr(IsReflectableShared<FieldType>())
			{
				using UnderlyingType = B3DDecaySharedPointer<FieldType>;
				static_assert(std::is_base_of_v<IReflectable, UnderlyingType>, "RTTI fields holding shared pointers must ensure the pointed-to data types implement the IReflectable interface.");

				return RTTIFieldTypeSchema(true, 0, SerializableFT_ReflectablePtr, UnderlyingType::GetRttiStatic()->GetRttiId(), UnderlyingType::GetRttiStatic()->GetSchema());
			}
			else if constexpr(IsReflectable<FieldType>())
			{
				return RTTIFieldTypeSchema(true, 0, SerializableFT_Reflectable, FieldType::GetRttiStatic()->GetRttiId(), FieldType::GetRttiStatic()->GetSchema());
			}
			else if constexpr(IsPlain<FieldType>())
			{
				static_assert(sizeof(RTTIPlainType<FieldType>::id) > 0, "Plain type has no valid ID."); // TODO - sizeof here is probably a bug, but it would break too many things to fix right now (also breaks memcpy serialization)

				BitLength fixedSize = 0; 
				const bool hasDynamicSize = RTTIPlainType<FieldType>::hasDynamicSize != 0;
				if(!hasDynamicSize)
				{
					// Note: Would be nice to avoid the construction of FieldType
					fixedSize = RTTIPlainType<FieldType>::GetSize(FieldType(), fieldInfo, false);
					
					if(fixedSize.Bytes > 255)
					{
						B3D_ASSERT(false);
						B3D_LOG(Error, RTTI, "Trying to create a plain RTTI field with size larger than 255. In order to use larger sizes for plain types please specialize RTTIPlainType, set hasDynamicSize to true.");
					}
				}

				return RTTIFieldTypeSchema(RTTIPlainType<FieldType>::hasDynamicSize, fixedSize, SerializableFT_Plain, RTTIPlainType<FieldType>::id, nullptr);
			}
			else
			{
				static_assert(false, "Cannot initialize RTTIField. Unsupported type provided. Make sure your type either derives from IReflectable or implements the RTTIPlainType<T> specialization.");
				return RTTIFieldTypeSchema();
			}
		}

		/** Writes the provided plain object to the stream. */
		template<typename T>
		void WritePlainTypeToStream(const T& value, Bitstream& stream, bool useCompression)
		{
			if(!B3D_ENSURE(IsPlain<T>))
				return;

			RTTIPlainType<T>::ToMemory(value, stream, Schema.Info, useCompression);
		}

		/** Reads the provided plain object from the stream. */
		template<typename T>
		void ReadPlainTypeFromStream(T& value, Bitstream& stream, bool useCompression)
		{
			if(!B3D_ENSURE(IsPlain<T>))
				return;

			RTTIPlainType<T>::FromMemory(value, stream, Schema.Info, useCompression);
		}

		/*
		 * API as seen from the binary serializer:
		 *
		 * foreach(field in fields)
		 * {
		 *		if(field.Iterator)
		 *		{
					auto iterator = field.GetIterator();
					for(; iterator.isValid(); iterator.advance())
					{
						const void* tuple = field.GetValue(iterator);

						for(tupleIndex in field.tupleCount)
						{
							switch(field.tupleType[tupleIndex])
							{
								case plain:
								field.EncodeToStream(tuple, tupleIndex, tupleValue, stream)

								case reflectable:
								IReflectable& tupleValue = field.GetReflectableTupleValue(tuple, tupleIndex);
								EncodeComplex(tupleValue);

								case reflectableptr:
								SPtr<IReflectable> tupleValue = field.GetReflectablePtrTupleValue(tuple, tupleIndex);
								EncodeEntry(tupleValue);
								write object id to stream
							} 
						}

					}
		 *		}
		 *		else
				{
				 // Old handling
				}
		 * }
		 *
		 */

		/*
		 * API as seen from the binary deserializer:
		 *
		 * foreach(field in fields)
		 * {
		 *		if(field.Iterator)
		 *		{
					auto iterator = field.GetIterator();
					for(; iterator.isValid(); iterator.advance())
					{
						void* tuple = field.AllocateEmptyObject(frameAllocator); // Allocate dynamically using the provided frame allocator
						
						for(tupleIndex in field.tupleCount)
						{
							switch(field.tupleType[tupleIndex])
							{
								case plain:
								field.DecodeFromStream(tupleIndex, tuple, stream)

								case reflectable:
								SPtr<Reflectable> value = DecodeEntry();
								field.SetTupleValue(tupleIndex, tuple, *value);

								case reflectableptr:
								read object id
								SPtr<Reflectable> value = FindOrDecodeEntry(object id);
								field.SetTupleValue(tupleIndex, tuple, value);
							} 
						}

						field.SetValue(iterator, tuple); // Internally this COPIES the data into the field (original also must be explicitly destructed, as it's frame allocated, and could be a SPtr)
					}
		 *		}
		 *		else
				{
				 // Old handling
				}
		 * }
		 *
		 */

		/** Checks is the provided type a value type deriving from IReflectable. */
		template<class T>
		constexpr bool IsReflectable() const
		{
			return std::is_base_of_v<IReflectable, B3DDecaySharedPointer<T>>;
		}

		/** Checks is the provided type a shared pointer referencing a type deriving from IReflectable. */
		template<class T>
		constexpr bool IsReflectableShared() const
		{
			return B3DIsSharedPointer<T>() && IsReflectable<B3DDecaySharedPointer<T>>;
		}

		/** Checks is the provided type a plain type (implements the RTTIPlainType<T> specialization). */
		template<class T>
		constexpr bool IsPlain() const
		{
			return B3DIsComplete<RTTIPlainType<T>>::value;
		}
		
		GetIteratorDelegate mGetIteratorCallback;
		GetValueDelegate mGetValueCallback;
		SetValueDelegate mSetValueCallback;
	};

	/** @} */
	/** @} */
} // namespace bs
