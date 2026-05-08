//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIField.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Shared helper functions used by TRTTIIteratorField and TRTTIECSField. */
	namespace detail
	{
		/** Checks is the provided type a value type deriving from IReflectable. */
		template<class T>
		constexpr bool IsReflectable()
		{
			return std::is_base_of_v<IReflectable, std::remove_reference_t<std::remove_cv_t<T>>>;
		}

		/** Checks is the provided type a shared pointer referencing a type deriving from IReflectable. */
		template<class T>
		constexpr bool IsReflectableShared()
		{
			return B3DIsSharedPointer<std::remove_reference_t<std::remove_cv_t<T>>>::value && IsReflectable<typename B3DDecaySharedPointer<std::remove_reference_t<std::remove_cv_t<T>>>::value>();
		}

		/** Checks is the provided type a plain type (implements the RTTIPlainType<T> specialization). */
		template<class T>
		constexpr bool IsPlain()
		{
			return B3DHasRTTIPlainTypeSpecialization<std::remove_reference_t<std::remove_cv_t<T>>>::value;
		}

		/** Creates a schema for a type stored in a field. */
		template<typename FieldType>
		RTTIFieldDataTypeSchema CreateFieldTypeSchema(const RTTIFieldInfo& fieldInfo)
		{
			if constexpr(IsReflectableShared<FieldType>())
			{
				using UnderlyingType = typename B3DDecaySharedPointer<FieldType>::value;
				static_assert(std::is_base_of_v<IReflectable, UnderlyingType>, "RTTI fields holding shared pointers must ensure the pointed-to data types implement the IReflectable interface.");

				return RTTIFieldDataTypeSchema(true, 0, RTTIFieldDataType::ReflectablePointer, UnderlyingType::GetRttiStatic()->GetRttiId(), UnderlyingType::GetRttiStatic()->GetSchema());
			}
			else if constexpr(IsReflectable<FieldType>())
			{
				return RTTIFieldDataTypeSchema(true, 0, RTTIFieldDataType::Reflectable, FieldType::GetRttiStatic()->GetRttiId(), FieldType::GetRttiStatic()->GetSchema());
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
						B3D_LOG(Error, LogRTTI, "Trying to create a plain RTTI field with size larger than 255. In order to use larger sizes for plain types please specialize RTTIPlainType, set hasDynamicSize to true.");
					}
				}

				return RTTIFieldDataTypeSchema(RTTIPlainType<FieldType>::hasDynamicSize, fixedSize, RTTIFieldDataType::Plain, RTTIPlainType<FieldType>::id, nullptr);
			}
			else
			{
				static_assert(false, "Cannot initialize RTTIField. Unsupported type provided. Make sure your type either derives from IReflectable or implements the RTTIPlainType<T> specialization.");
				return RTTIFieldDataTypeSchema();
			}
		}

		/** Reads the provided plain object from the stream. */
		template<typename T>
		void ReadPlainType(T& value, Bitstream& stream, const RTTIFieldInfo& info, bool useCompression)
		{
			if constexpr(IsPlain<T>())
				RTTIPlainType<T>::FromMemory(value, stream, info, useCompression);
		}

		/** Writes the provided plain object to the stream. */
		template<typename T>
		void WritePlainType(const T& value, Bitstream& stream, const RTTIFieldInfo& info, bool useCompression)
		{
			if constexpr(IsPlain<T>())
				RTTIPlainType<T>::ToMemory(value, stream, info, useCompression);
		}

		/** Returns the size of the provided plain object. */
		template<typename T>
		BitLength GetPlainTypeSize(const T& value, const RTTIFieldInfo& info, bool useCompression)
		{
			if constexpr(IsPlain<T>())
				return RTTIPlainType<T>::GetSize(value, info, useCompression);

			return 0;
		}

		/** Assigns the reflectable value to the provided field value. */
		template<typename T>
		void SetReflectableValue(T& value, const IReflectable& reflectable)
		{
			if constexpr(IsReflectable<T>())
				value = static_cast<const T&>(reflectable);
		}

		/** Reads the reflectable value from the provided field value. */
		template<typename T>
		const IReflectable& GetReflectableValue(const T& value)
		{
			if constexpr(IsReflectable<T>())
				return value;

			B3D_ASSERT(false);
			static IReflectable* kNull = nullptr;
			return *kNull;
		}

		/** Assigns the reflectable pointer to the provided field value. */
		template<typename T>
		void SetReflectablePointerValue(T& value, const TShared<IReflectable>& reflectable)
		{
			if constexpr(IsReflectableShared<T>())
				value = std::static_pointer_cast<typename B3DDecaySharedPointer<T>::value>(reflectable);
		}

		/** Reads the reflectable pointer from the provided field value. */
		template<typename T>
		TShared<IReflectable> GetReflectablePointerValue(const T& value)
		{
			if constexpr(IsReflectableShared<T>())
				return value;
			return nullptr;
		}
	} // namespace detail

	/** Provides interface to be implemented by specializations of TRTTIIteratorField. */
	struct RTTIIteratorField : public RTTIField
	{
		virtual ~RTTIIteratorField() = default;

		/** Returns the iterator that can be used for iterating all entries in the field. */
		virtual TShared<IRTTIIterator> GetIterator(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator) const = 0;

		/** Returns true if the iterator is allowed to seek to index. True for iterators over arrays. */
		virtual bool IteratorSupportsSeekToIndex() const = 0;

		/** Returns true if the iterator is allowed to seek to key. True for iterators over sets and maps. */
		virtual bool IteratorSupportsSeekToKey() const = 0;

		/** Returns the current value of the iterator. */
		virtual const void* GetIteratorValue(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator) const = 0;

		/** Returns a modifiable copy of the current value of the iterator. Returned value must be freed via FreeFieldValue() when done using it. */
		virtual void* GetIteratorValueCopy(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator) const = 0;

		/**
		 * Inserts a new value before the iterator location. @p value must have been created by a call to CreateEmptyFieldValue() or
		 * GetIteratorValueCopy(), using the same @p frameAllocator. 
		 */
		virtual void SetIteratorValue(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator, void* value) = 0;

		/**
		 * Creates a new empty field value. This should be populated by calls to WritePlainTypeTupleToStream(), SetReflectablePointer() or SetReflectable(), then
		 * passed to SetIteratorValue(), and finally freed via FreeFieldValue().
		 */
		virtual void* CreateEmptyFieldValue(FrameAllocator& frameAllocator) = 0;

		/**
		 * Frees the field value created by CreateEmptyFieldValue() or GetIteratorValueCopy(). Same allocator must be provided as used for creating the field
		 * value.
		 */
		virtual void FreeFieldValue(void* fieldValue, FrameAllocator& frameAllocator) = 0;
		
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
		virtual void SetReflectablePointer(void* fieldValue, u32 tupleElementIndex, const TShared<IReflectable>& reflectable) = 0;

		/**
		 * Reads the reflectable pointer from the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this reads a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p fieldValue is simply cast to a reflectable pointer type.
		 *
		 * @param fieldValue		Field value containing the reflectable pointer, as returned by GetIteratorValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue which contains the reflectable pointer. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @return					Reflectable pointer.
		 */
		virtual TShared<IReflectable> GetReflectablePointer(const void* fieldValue, u32 tupleElementIndex) = 0;

		/**
		 * Assigns the reflectable to the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this assigns a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p reflectable is simply assigned to @p fieldValue.
		 *
		 * @param fieldValue		Field value in which to store the reflectable, as created by CreateEmptyFieldValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue in which to store the reflectable. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @param reflectable		Reflectable to assign to the field value.
		 */
		virtual void SetReflectable(void* fieldValue, u32 tupleElementIndex, const IReflectable& reflectable) = 0;

		/**
		 * Reads the reflectable value from the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this reads a single element of the tuple, as specified by @p tupleElementIndex. If the field value is not a tuple, @p fieldValue is simply cast to a reflectable type.
		 *
		 * @param fieldValue		Field value containing the reflectable, as returned by GetIteratorValue().
		 * @param tupleElementIndex Index of the tuple element in @p fieldValue which contains the reflectable. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @return					Reflectable.
		 */
		virtual const IReflectable& GetReflectable(const void* fieldValue, u32 tupleElementIndex) = 0;

		/**
		 * Returns the size of the plain type in the provided field value. If the field value represents a tuple (i.e. contains multiple sub-types, such as std::pair<K, V>),
		 * this returns the size of just a single tuple element, as specified by @p tupleElementIndex.
		 *
		 * @param	fieldValue			Field value to retrieve size from, as returned by GetIteratorValue().
		 * @param	tupleElementIndex	Index of the tuple element in @p fieldValue to retrieve size for. If @p fieldValue doesn't represent a tuple, this should be 0.
		 * @param	useCompression		If true, size will be compressed if possible.
		 */
		virtual BitLength GetPlainTypeSize(const void* fieldValue, u32 tupleElementIndex, bool useCompression) = 0;
	};

	/**
	 * RTTI field type that supports iteration over arbitrary containers, including maps. Unlike older field types, this field type internally handles plain, reflectable and reflectable pointer types,
	 * rather than requiring a separate field type for each. Also handles access to non-container elements by mimicing a single-element container.
	 *
	 * @tparam ObjectRTTIType		RTTIType of the object that contains the field we're accessing.
	 * @tparam DataType				Type of the container being iterated over. 
	 * @tparam IsDataTypeContainer	Set to true DataType is a container (e.g. vector, map) and you wish to iterate over it as such. If false, the iterator will act as faux iterator over a single element.
	 * @tparam ObjectType			Type of the object that the field is a member of.
	 */
	template <class ObjectRTTIType, class DataType, bool IsDataTypeContainer, class ObjectType>
	struct TRTTIIteratorField : public RTTIIteratorField
	{
		using IteratorType = TRTTIIterator<DataType, IsDataTypeContainer>;
		using ElementType = typename IteratorType::ElementType;

		typedef UPtr<IteratorType, DefaultAllocatorTag, TRTTIIteratorDeleter<DataType, IsDataTypeContainer>> (ObjectRTTIType::*GetIteratorDelegate)(ObjectType&, FrameAllocator&);
		typedef const ElementType& (ObjectRTTIType::*GetValueDelegate)(ObjectType&, FrameAllocator&, IteratorType&);
		typedef void (ObjectRTTIType::*SetValueDelegate)(ObjectType&, FrameAllocator&, IteratorType&, const ElementType&);

		TRTTIIteratorField(String name, u16 uniqueId, GetIteratorDelegate getIteratorCallback, GetValueDelegate getValueCallback, SetValueDelegate setValueCallback, const RTTIFieldInfo& fieldInfo)
		{
			B3D_ASSERT(getIteratorCallback != nullptr);
			B3D_ASSERT(getValueCallback != nullptr);
			B3D_ASSERT(setValueCallback != nullptr);

			this->mGetIteratorCallback = getIteratorCallback;
			this->mGetValueCallback = getValueCallback;
			this->mSetValueCallback = setValueCallback;

			this->Name = std::move(name);
			this->Schema = RTTIFieldSchema(uniqueId, IsDataTypeContainer, RTTIFieldType::Iterable, fieldInfo);
		}

		void InitSchema() override
		{
			// Special case for pairs so we natively support reflectable types in maps (This could technically be extended to support std::tuple as well if needed)
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				this->Schema.FieldDataTypes.Add(detail::CreateFieldTypeSchema<std::remove_cv_t<typename ElementType::first_type>>(this->Schema.Info));
				this->Schema.FieldDataTypes.Add(detail::CreateFieldTypeSchema<std::remove_cv_t<typename ElementType::second_type>>(this->Schema.Info));
			}
			else
			{
				this->Schema.FieldDataTypes.Add(detail::CreateFieldTypeSchema<std::remove_cv_t<ElementType>>(this->Schema.Info));
			}
		}

		TShared<IRTTIIterator> GetIterator(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator) const override
		{
			ObjectRTTIType* const exactRttiTypeInstane = static_cast<ObjectRTTIType*>(rttiTypeInstance);
			ObjectType* const exactObject = static_cast<ObjectType*>(object);

			return std::move((exactRttiTypeInstane->*mGetIteratorCallback)(*exactObject, frameAllocator));
		}

		bool IteratorSupportsSeekToIndex() const override
		{
			return IteratorType::SupportsSeekToIndex();
		}

		bool IteratorSupportsSeekToKey() const override
		{
			return IteratorType::SupportsSeekToKey();
		}

		const void* GetIteratorValue(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator) const override
		{
			ObjectRTTIType* const exactRttiTypeInstance = static_cast<ObjectRTTIType*>(rttiTypeInstance);
			ObjectType* const exactObject = static_cast<ObjectType*>(object);
			IteratorType& exactIterator = static_cast<IteratorType&>(iterator);

			return &(exactRttiTypeInstance->*mGetValueCallback)(*exactObject, frameAllocator, exactIterator);
		}

		void SetIteratorValue(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator, void* value) override
		{
			ObjectRTTIType* const exactRttiTypeInstance = static_cast<ObjectRTTIType*>(rttiTypeInstance);
			ObjectType* const exactObject = static_cast<ObjectType*>(object);
			IteratorType& exactIterator = static_cast<IteratorType&>(iterator);

			ElementType& exactValue = *static_cast<ElementType*>(value);
			(exactRttiTypeInstance->*mSetValueCallback)(*exactObject, frameAllocator, exactIterator, exactValue);
		}

		void* CreateEmptyFieldValue(FrameAllocator& frameAllocator) override
		{
			return frameAllocator.Construct<ElementType>();
		}

		void* GetIteratorValueCopy(RTTIType* rttiTypeInstance, void* object, FrameAllocator& frameAllocator, IRTTIIterator& iterator) const override
		{
			 return frameAllocator.Construct<ElementType>(*static_cast<const ElementType*>(GetIteratorValue(rttiTypeInstance, object, frameAllocator, iterator)));
		}

		void FreeFieldValue(void* fieldValue, FrameAllocator& frameAllocator) override
		{
			if(fieldValue != nullptr)
			{
				ElementType& exactValue = *static_cast<ElementType*>(fieldValue);
				frameAllocator.Destruct(&exactValue);
			}
		}

		void ReadPlainTypeTupleFromStream(void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) override
		{
			ElementType& value = *static_cast<ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value) // Note: Currently supporting just std::pair, but can support other types eventually
			{
				if(!B3D_ENSURE(tupleElementIndex <= 1))
					return;

				if(tupleElementIndex == 0)
				{
					// Make sure to remove constness, as map values usually have a const key (Proper fix would be to deduce a non-const version of std::pair<const K, V>/etc.)
					using MutableType = std::remove_cv_t<typename ElementType::first_type>;
					detail::ReadPlainType(const_cast<MutableType&>(value.first), stream, Schema.Info, useCompression);
				}
				else
				{
					// Same as above
					using MutableType = std::remove_cv_t<typename ElementType::second_type>;
					detail::ReadPlainType(const_cast<MutableType&>(value.second), stream, Schema.Info, useCompression);
				}
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);

				// Same as above
				using MutableType = std::remove_cv_t<ElementType>;
				detail::ReadPlainType(const_cast<MutableType&>(value), stream, Schema.Info, useCompression);
			}
		}

		void WritePlainTypeTupleToStream(const void* fieldValue, u32 tupleElementIndex, Bitstream& stream, bool useCompression) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					detail::WritePlainType(value.first, stream, Schema.Info, useCompression);
				else
					detail::WritePlainType(value.second, stream, Schema.Info, useCompression);
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				detail::WritePlainType(value, stream, Schema.Info, useCompression);
			}
		}

		void SetReflectablePointer(void* fieldValue, u32 tupleElementIndex, const TShared<IReflectable>& reflectable) override
		{
			ElementType& value = *static_cast<ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
				{
					// Make sure to remove constness, as map values usually have a const key (Proper fix would be to deduce a non-const version of std::pair<const K, V>/etc.)
					using MutableType = std::remove_cv_t<typename ElementType::first_type>;
					detail::SetReflectablePointerValue(const_cast<MutableType&>(value.first), reflectable);
				}
				else
				{
					// Same as above
					using MutableType = std::remove_cv_t<typename ElementType::second_type>;
					detail::SetReflectablePointerValue(const_cast<MutableType&>(value.second), reflectable);
				}
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);

				// Same as above
				using MutableType = std::remove_cv_t<ElementType>;
				detail::SetReflectablePointerValue(const_cast<MutableType&>(value), reflectable);
			}
		}

		TShared<IReflectable> GetReflectablePointer(const void* fieldValue, u32 tupleElementIndex) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					return detail::GetReflectablePointerValue(value.first);

				return detail::GetReflectablePointerValue(value.second);
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				return detail::GetReflectablePointerValue(value);
			}
		}

		void SetReflectable(void* fieldValue, u32 tupleElementIndex, const IReflectable& reflectable) override
		{
			ElementType& value = *static_cast<ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
				{
					// Make sure to remove constness, as map values usually have a const key (Proper fix would be to deduce a non-const version of std::pair<const K, V>/etc.)
					using MutableType = std::remove_cv_t<typename ElementType::first_type>;
					detail::SetReflectableValue(const_cast<MutableType&>(value.first), reflectable);
				}
				else
				{
					// Same as above
					using MutableType = std::remove_cv_t<typename ElementType::second_type>;
					detail::SetReflectableValue(const_cast<MutableType&>(value.second), reflectable);
				}
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);

				// Same as above
				using MutableType = std::remove_cv_t<ElementType>;
				detail::SetReflectableValue(const_cast<MutableType&>(value), reflectable);
			}
		}

		const IReflectable& GetReflectable(const void* fieldValue, u32 tupleElementIndex) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					return detail::GetReflectableValue(value.first);

				return detail::GetReflectableValue(value.second);
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				return detail::GetReflectableValue(value);
			}
		}

		BitLength GetPlainTypeSize(const void* fieldValue, u32 tupleElementIndex, bool useCompression) override
		{
			const ElementType& value = *static_cast<const ElementType*>(fieldValue);
			if constexpr(B3DIsStdPair<ElementType>::value)
			{
				B3D_ENSURE(tupleElementIndex <= 1);

				if(tupleElementIndex == 0)
					return detail::GetPlainTypeSize(value.first, Schema.Info, useCompression);
				else
					return detail::GetPlainTypeSize(value.second, Schema.Info, useCompression);
			}
			else
			{
				B3D_ENSURE(tupleElementIndex == 0);
				return detail::GetPlainTypeSize(value, Schema.Info, useCompression);
			}
		}

	private:
		GetIteratorDelegate mGetIteratorCallback;
		GetValueDelegate mGetValueCallback;
		SetValueDelegate mSetValueCallback;
	};

	/** @} */
	/** @} */
} // namespace b3d
