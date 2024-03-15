//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Base class containing common functionality for a reflectable class field.
	 *
	 * @note
	 * Reflectable fields are fields containing complex types deriving from IReflectable. They are serialized recursively
	 * and you may add/remove fields from them without breaking the serialized data.
	 */
	struct RTTIReflectableFieldBase : public RTTIField
	{
		/**
		 * Retrieves the IReflectable value from the provided instance.
		 *
		 * @note	Field type must not be an array.
		 */
		virtual IReflectable& GetValue(RTTITypeBase* rtti, void* object) = 0;

		/**
		 * Retrieves the IReflectable value from an array on the provided instance and index.
		 *
		 * @note	Field type must be an array.
		 */
		virtual IReflectable& GetArrayValue(RTTITypeBase* rtti, void* object, u32 index) = 0;

		/**
		 * Sets the IReflectable value in the provided instance.
		 *
		 * @note	Field type must not be an array.
		 */
		virtual void SetValue(RTTITypeBase* rtti, void* object, IReflectable& value) = 0;

		/**
		 * Sets the IReflectable value in an array on the provided instance and index.
		 *
		 * @note	Field type must be an array.
		 */
		virtual void SetArrayValue(RTTITypeBase* rtti, void* object, u32 index, IReflectable& value) = 0;

		/** Creates a new object of the field type. */
		virtual SPtr<IReflectable> NewObject() = 0;

		/** Retrieves the RTTI object for the type the field contains. */
		virtual RTTITypeBase* GetType() = 0;
	};

	/**	Reflectable field containing a specific type with RTTI implemented. */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIReflectableField : public RTTIReflectableFieldBase
	{
		typedef DataType& (InterfaceType::*GetterType)(ObjectType*);
		typedef void (InterfaceType::*SetterType)(ObjectType*, DataType&);

		typedef DataType& (InterfaceType::*ArrayGetterType)(ObjectType*, u32);
		typedef void (InterfaceType::*ArraySetterType)(ObjectType*, u32, DataType&);
		typedef u32 (InterfaceType::*ArrayGetSizeType)(ObjectType*);
		typedef void (InterfaceType::*ArraySetSizeType)(ObjectType*, u32);

		/**
		 * Initializes a field containing a single data type implementing IReflectable interface.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a
		 *							small data type that can be used for efficiently serializing data to disk and similar.
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field.
		 * @param[in]	setter  	The setter method for the field.
		 * @param[in]	info		Various optional information about the field.
		 */
		void InitSingle(String name, u16 uniqueId, GetterType getter, SetterType setter, const RTTIFieldInfo& info)
		{
			this->Getter = getter;
			this->Setter = setter;

			Init(std::move(name), RTTIFieldSchema(uniqueId, false, true, 0, SerializableFT_Reflectable, 0, nullptr, info));
		}

		/**
		 * Initializes a field containing an array of data types implementing IReflectable interface.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a
		 *							small data type that can be used for efficiently serializing data to disk and similar.
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field.
		 * @param[in]	getSize 	Getter method that returns the size of an array.
		 * @param[in]	setter  	The setter method for the field.
		 * @param[in]	setSize 	Setter method that allows you to resize an array.
		 * @param[in]	info		Various optional information about the field.
		 */
		void InitArray(const String& name, u16 uniqueId, ArrayGetterType getter, ArrayGetSizeType getSize, ArraySetterType setter, ArraySetSizeType setSize, const RTTIFieldInfo& info)
		{
			ArrayGetter = getter;
			ArraySetter = setter;
			ArrayGetSize = getSize;
			ArraySetSize = setSize;

			Init(std::move(name), RTTIFieldSchema(uniqueId, true, true, 0, SerializableFT_Reflectable, 0, nullptr, info));
		}

		void InitSchema() override
		{
			// This need to be initialized after the field itself, otherwise we get recursive static constructor
			// calls due to one type calling GetRttiStatic() on one another
			Schema.FieldTypeSchema = DataType::GetRttiStatic()->GetSchema();
			Schema.FieldTypeId = DataType::GetRttiStatic()->GetRttiId();

			// Add the new schema type
			RTTIFieldTypeSchema fieldTypeSchema;
			fieldTypeSchema.FieldTypeId = Schema.FieldTypeId;
			fieldTypeSchema.FieldTypeSchema = Schema.FieldTypeSchema;
			fieldTypeSchema.Type = Schema.Type;
			fieldTypeSchema.FixedSize = Schema.Size;
			fieldTypeSchema.HasDynamicSize = Schema.HasDynamicSize;

			Schema.FieldTypes.Add(fieldTypeSchema);
		}

		IReflectable& GetValue(RTTITypeBase* rtti, void* object) override
		{
			CheckIsArray(false);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			IReflectable& castDataType = (rttiObject->*Getter)(castObjType);

			return castDataType;
		}

		IReflectable& GetArrayValue(RTTITypeBase* rtti, void* object, u32 index) override
		{
			CheckIsArray(true);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);

			IReflectable& castDataType = (rttiObject->*ArrayGetter)(castObjType, index);
			return castDataType;
		}

		void SetValue(RTTITypeBase* rtti, void* object, IReflectable& value) override
		{
			CheckIsArray(false);

			if(!Setter)
			{
				B3D_EXCEPT(InternalErrorException, "Specified field (" + Name + ") has no setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			DataType& castDataObj = static_cast<DataType&>(value);

			(rttiObject->*Setter)(castObjType, castDataObj);
		}

		void SetArrayValue(RTTITypeBase* rtti, void* object, u32 index, IReflectable& value) override
		{
			CheckIsArray(true);

			if(!ArraySetter)
			{
				B3D_EXCEPT(InternalErrorException, "Specified field (" + Name + ") has no setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			DataType& castDataObj = static_cast<DataType&>(value);

			(rttiObject->*ArraySetter)(castObjType, index, castDataObj);
		}

		u32 GetArraySize(RTTITypeBase* rtti, void* object) override
		{
			CheckIsArray(true);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			return (rttiObject->*ArrayGetSize)(castObject);
		}

		void SetArraySize(RTTITypeBase* rtti, void* object, u32 size) override
		{
			CheckIsArray(true);

			if(!ArraySetSize)
			{
				B3D_EXCEPT(InternalErrorException, "Specified field (" + Name + ") has no array size setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			(rttiObject->*ArraySetSize)(castObject, size);
		}

		SPtr<IReflectable> NewObject() override
		{
			return DataType::GetRttiStatic()->NewRttiObject();
		}

		RTTITypeBase* GetType() override
		{
			return DataType::GetRttiStatic();
		}

	private:
		union
		{
			struct
			{
				GetterType Getter;
				SetterType Setter;
			};

			struct
			{
				ArrayGetterType ArrayGetter;
				ArraySetterType ArraySetter;

				ArrayGetSizeType ArrayGetSize;
				ArraySetSizeType ArraySetSize;
			};
		};
	};

	/** @} */
	/** @} */
} // namespace bs
