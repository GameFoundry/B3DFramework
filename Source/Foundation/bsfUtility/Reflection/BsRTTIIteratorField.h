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

		virtual UPtr<IRTTIIterator> GetIterator() const = 0;
		virtual void DecodeFromStream(RTTITypeBase* objectType, void* object, IRTTIIterator& iterator, Bitstream& stream, bool compress) = 0;
		virtual void EncodeToStream(RTTITypeBase* objectType, void* object, IRTTIIterator& iterator, Bitstream& stream, bool compress) = 0;
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
	struct TRTTIIteratorField : public RTTIIteratorField
	{
		using ElementType = typename ContainerType::value_type;

		typedef UPtr<TRTTIIterator<ContainerType>> (RTTIType::*GetIteratorDelegate)(ObjectType*);
		typedef const ElementType& (RTTIType::*GetValueDelegate)(ObjectType*, TRTTIIterator<ContainerType>&);
		typedef void (RTTIType::*SetValueDelegate)(ObjectType*, TRTTIIterator<ContainerType>&, const ElementType&);

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

		/** Creates a schema for a type stored in the field. */
		template<typename FieldType>
		RTTIFieldTypeSchema CreateFieldTypeSchema(const RTTIFieldInfo& fieldInfo)
		{
			if constexpr(B3DIsSharedPointer<FieldType>())
			{
				using UnderlyingType = B3DDecaySharedPointer<FieldType>;
				static_assert(std::is_base_of_v<IReflectable, UnderlyingType>, "RTTI fields holding shared pointers must ensure the pointed-to data types implement the IReflectable interface.");

				return RTTIFieldTypeSchema(true, 0, SerializableFT_ReflectablePtr, UnderlyingType::GetRttiStatic()->GetRttiId(), UnderlyingType::GetRttiStatic()->GetSchema());
			}
			else if constexpr(std::is_base_of_v<IReflectable, FieldType>)
			{
				return RTTIFieldTypeSchema(true, 0, SerializableFT_Reflectable, FieldType::GetRttiStatic()->GetRttiId(), FieldType::GetRttiStatic()->GetSchema());
			}
			else // Plain type
			{
				static_assert(B3DIsComplete<RTTIPlainType<FieldType>>::value, "RTTI field type that doesn't derive from IReflectable must implement the RTTIPlainType specialization.");
				static_assert(sizeof(RTTIPlainType<FieldType>::id) > 0, "Plain type has no valid ID.");

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
		}

		UPtr<IRTTIIterator> GetIterator() const override
		{
			return mGetIteratorCallback();
		}

		// TODO - Also supply tuple index
		void DecodeFromStream(RTTITypeBase* objectType, void* object, IRTTIIterator& iterator, Bitstream& stream, bool compress) override
		{
			// TODO - Extend IRTTIIterator with SetTupleElement(index, element), then AssignTuple to actually assign it to the container
			// - Internally this just creates std::pair
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
						for(tupleIndex in field.tupleCount)
						{
							switch(field.tupleType[tupleIndex])
							{
								case plain:
								void* tupleValue = iterator.getPlainTupleValue(tupleIndex);
								field.EncodeToStream(tupleIndex, tupleValue, stream)

								case reflectable:
								IReflectable& tupleValue = iterator.getReflectableTupleValue(tupleIndex);
								EncodeComplex(tupleValue);

								case reflectableptr:
								SPtr<IReflectable> tupleValue = iterator.getReflectablePtrTupleValue(tupleIndex);
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
						void* tuple = iterator.AllocateEmptyObject();
						
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

						field.SetValue(iterator, tuple); // Internally this std::moves the data into the field
					}
		 *		}
		 *		else
				{
				 // Old handling
				}
		 * }
		 *
		 */
		
		GetIteratorDelegate mGetIteratorCallback;
		GetValueDelegate mGetValueCallback;
		SetValueDelegate mSetValueCallback;
	};

	/** @} */
	/** @} */
} // namespace bs
