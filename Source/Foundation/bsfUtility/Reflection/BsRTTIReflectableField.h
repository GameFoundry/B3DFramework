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
		virtual IReflectable& getValue(RTTITypeBase* rtti, void* object) = 0;

		/**
		 * Retrieves the IReflectable value from an array on the provided instance and index.
		 * 			
		 * @note	Field type must be an array.
		 */
		virtual IReflectable& getArrayValue(RTTITypeBase* rtti, void* object, UINT32 index) = 0;

		/**
		 * Sets the IReflectable value in the provided instance.
		 * 			
		 * @note	Field type must not be an array.
		 */
		virtual void setValue(RTTITypeBase* rtti, void* object, IReflectable& value) = 0;

		/**
		 * Sets the IReflectable value in an array on the provided instance and index.
		 * 			
		 * @note	Field type must be an array.
		 */
		virtual void setArrayValue(RTTITypeBase* rtti, void* object, UINT32 index, IReflectable& value) = 0;

		/** Creates a new object of the field type. */
		virtual SPtr<IReflectable> newObject() = 0;

		/** Retrieves the RTTI object for the type the field contains. */
		virtual RTTITypeBase* getType() = 0;
	};

	/**	Reflectable field containing a specific type with RTTI implemented. */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIReflectableField : public RTTIReflectableFieldBase
	{
		typedef DataType& (InterfaceType::*GetterType)(ObjectType*);
		typedef void (InterfaceType::*SetterType)(ObjectType*, DataType&);

		typedef DataType& (InterfaceType::*ArrayGetterType)(ObjectType*, UINT32);
		typedef void (InterfaceType::*ArraySetterType)(ObjectType*, UINT32, DataType&);
		typedef UINT32(InterfaceType::*ArrayGetSizeType)(ObjectType*);
		typedef void(InterfaceType::*ArraySetSizeType)(ObjectType*, UINT32);

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
		void initSingle(String name, UINT16 uniqueId, GetterType getter, SetterType setter, const RTTIFieldInfo& info)
		{
			this->getter = getter;
			this->setter = setter;

			init(std::move(name), RTTIFieldSchema(uniqueId, false, true, 0, SerializableFT_Reflectable,
				0, nullptr, info));
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
		void initArray(const String& name, UINT16 uniqueId, ArrayGetterType getter, ArrayGetSizeType getSize,
			ArraySetterType setter, ArraySetSizeType setSize, const RTTIFieldInfo& info)
		{
			arrayGetter = getter;
			arraySetter = setter;
			arrayGetSize = getSize;
			arraySetSize = setSize;

			const SPtr<RTTISchema>& fieldTypeSchema = DataType::getRTTIStatic()->getSchema();;
			UINT32 typeId = DataType::getRTTIStatic()->getRTTIId();
			init(std::move(name), RTTIFieldSchema(uniqueId, true, true, 0, SerializableFT_Reflectable,
				0, nullptr, info));
		}

		/** @copydoc RTTIField::initSchema */
		void initSchema() override
		{
			// This need to be initialized after the field itself, otherwise we get recursive static constructor
			// calls due to one type calling getRTTIStatic() on one another
			schema.fieldTypeSchema = DataType::getRTTIStatic()->getSchema();;
			schema.fieldTypeId = DataType::getRTTIStatic()->getRTTIId();
		}

		/** @copydoc RTTIReflectableFieldBase::getValue */
		IReflectable& getValue(RTTITypeBase* rtti, void* object) override
		{
			checkIsArray(false);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			IReflectable& castDataType = (rttiObject->*getter)(castObjType);

			return castDataType;
		}

		/** @copydoc RTTIReflectableFieldBase::getArrayValue */
		IReflectable& getArrayValue(RTTITypeBase* rtti, void* object, UINT32 index) override
		{
			checkIsArray(true);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);

			IReflectable& castDataType = (rttiObject->*arrayGetter)(castObjType, index);
			return castDataType;
		}

		/** @copydoc RTTIReflectableFieldBase::setValue */
		void setValue(RTTITypeBase* rtti, void* object, IReflectable& value) override
		{
			checkIsArray(false);

			if(!setter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			DataType& castDataObj = static_cast<DataType&>(value);

			(rttiObject->*setter)(castObjType, castDataObj);
		}

		/** @copydoc RTTIReflectableFieldBase::setArrayValue */
		void setArrayValue(RTTITypeBase* rtti, void* object, UINT32 index, IReflectable& value) override
		{
			checkIsArray(true);

			if(!arraySetter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			DataType& castDataObj = static_cast<DataType&>(value);

			(rttiObject->*arraySetter)(castObjType, index, castDataObj);
		}

		/** @copydoc RTTIField::getArraySize */
		UINT32 getArraySize(RTTITypeBase* rtti, void* object) override
		{
			checkIsArray(true);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			return (rttiObject->*arrayGetSize)(castObject);
		}

		/** @copydoc RTTIField::setArraySize */
		void setArraySize(RTTITypeBase* rtti, void* object, UINT32 size) override
		{
			checkIsArray(true);

			if(!arraySetSize)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no array size setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			(rttiObject->*arraySetSize)(castObject, size);
		}

		/** @copydoc RTTIReflectableFieldBase::newObject */
		SPtr<IReflectable> newObject() override
		{
			return DataType::getRTTIStatic()->newRTTIObject();
		}

		/** @copydoc RTTIReflectableFieldBase::getType */
		RTTITypeBase* getType() override
		{
			return DataType::getRTTIStatic();
		}

	private:
		union
		{
			struct
			{
				GetterType getter;
				SetterType setter;
			};

			struct
			{
				ArrayGetterType arrayGetter;
				ArraySetterType arraySetter;

				ArrayGetSizeType arrayGetSize;
				ArraySetSizeType arraySetSize;
			};
		};
	};

	/** @} */
	/** @} */
}
