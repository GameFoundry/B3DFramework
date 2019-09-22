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
	 * Base class containing common functionality for a reflectable pointer class field.
	 * 			
	 * @note	
	 * Reflectable fields are fields containing complex types deriving from IReflectable. They are serialized recursively
	 * and you may add/remove fields from them without breaking the serialized data.
	 * @note
	 * ReflectablePtr fields are different from Reflectable fields because other types may reference the same Reflectable
	 * object using a ReflectablePtr, while normal Reflectable fields are only referenced by a single field they're declared on.
	 */
	struct RTTIReflectablePtrFieldBase : public RTTIField
	{
		/**
		 * Retrieves the IReflectable value from the provided instance.
		 * 			
		 * @note	Field type must not be an array.
		 */
		virtual SPtr<IReflectable> getValue(RTTITypeBase* rtti, void* object) = 0;

		/**
		 * Retrieves the IReflectable value from an array on the provided instance and index.
		 * 			
		 * @note	Field type must be an array.
		 */
		virtual SPtr<IReflectable> getArrayValue(RTTITypeBase* rtti, void* object, UINT32 index) = 0;

		/**
		 * Sets the IReflectable value in the provided instance.
		 * 			
		 * @note	Field type must not be an array.
		 */
		virtual void setValue(RTTITypeBase* rtti, void* object, SPtr<IReflectable> value) = 0;

		/**
		 * Sets the IReflectable value in an array on the provided instance and index.
		 * 			
		 * @note	Field type must be an array.
		 */
		virtual void setArrayValue(RTTITypeBase* rtti, void* object, UINT32 index, SPtr<IReflectable> value) = 0;

		/** Creates a new object of the field type. */
		virtual SPtr<IReflectable> newObject() = 0;

		/** Returns the RTTI identifier of the class owning the field. */
		virtual UINT32 getRTTIId() = 0;

		/** Returns the name of the class owning the field. */
		virtual const String& getRTTIName() = 0;

		/** Retrieves the RTTI object for the type the field contains. */
		virtual RTTITypeBase* getType() = 0;
	};

	/** Reflectable field containing a pointer to a specific type with RTTI implemented.  */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIReflectablePtrField : public RTTIReflectablePtrFieldBase
	{
		typedef SPtr<DataType> (InterfaceType::*GetterType)(ObjectType*);
		typedef void (InterfaceType::*SetterType)(ObjectType*, SPtr<DataType>);

		typedef SPtr<DataType> (InterfaceType::*ArrayGetterType)(ObjectType*, UINT32);
		typedef void (InterfaceType::*ArraySetterType)(ObjectType*, UINT32, SPtr<DataType>);
		typedef UINT32(InterfaceType::*ArrayGetSizeType)(ObjectType*);
		typedef void(InterfaceType::*ArraySetSizeType)(ObjectType*, UINT32);

		/**
		 * Initializes a field pointing to a single data type implementing IReflectable interface.
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

			init(std::move(name), RTTIFieldSchema(uniqueId, false, true, 0, SerializableFT_ReflectablePtr,
				0, nullptr, info));
		}

		/**
		 * Initializes a field containing an array of pointers to data types implementing IReflectable interface.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a
		 *							small data type that can be used for efficiently serializing data to disk and similar.
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field.
		 * @param[in]	getSize 	Getter method that returns the size of an array.
		 * @param[in]	setter  	The setter method for the field.
		 * @param[in]	setSize 	Setter method that allows you to resize an array. Can be null.
		 * @param[in]	info		Various optional information about the field.
		 */
		void initArray(String name, UINT16 uniqueId, ArrayGetterType getter, ArrayGetSizeType getSize,
			ArraySetterType setter, ArraySetSizeType setSize, const RTTIFieldInfo& info)
		{
			arrayGetter = getter;
			arraySetter = setter;
			arrayGetSize = getSize;
			arraySetSize = setSize;

			init(std::move(name), RTTIFieldSchema(uniqueId, true, true, 0, SerializableFT_ReflectablePtr,
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

		/** @copydoc RTTIReflectablePtrFieldBase::getValue */
		SPtr<IReflectable> getValue(RTTITypeBase* rtti, void* object) override
		{
			checkIsArray(false);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);

			SPtr<IReflectable> castDataType = (rttiObject->*getter)(castObjType);
			return castDataType;
		}

		/** @copydoc RTTIReflectablePtrFieldBase::getArrayValue */
		SPtr<IReflectable> getArrayValue(RTTITypeBase* rtti, void* object, UINT32 index) override
		{
			checkIsArray(true);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);

			SPtr<IReflectable> castDataType = (rttiObject->*arrayGetter)(castObjType, index);
			return castDataType;
		}

		/** @copydoc RTTIReflectablePtrFieldBase::setValue */
		void setValue(RTTITypeBase* rtti, void* object, SPtr<IReflectable> value) override
		{
			checkIsArray(false);

			if(!setter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			SPtr<DataType> castDataObj = std::static_pointer_cast<DataType>(value);

			(rttiObject->*setter)(castObjType, castDataObj);
		}

		/** @copydoc RTTIReflectablePtrFieldBase::setArrayValue */
		void setArrayValue(RTTITypeBase* rtti, void* object, UINT32 index, SPtr<IReflectable> value) override
		{
			checkIsArray(true);

			if(!arraySetter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObjType = static_cast<ObjectType*>(object);
			SPtr<DataType> castDataObj = std::static_pointer_cast<DataType>(value);

			(rttiObject->*arraySetter)(castObjType, index, castDataObj);
		}

		/** @copydoc RTTIField::setArraySize */
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

		/** @copydoc RTTIReflectablePtrFieldBase::newObject */
		SPtr<IReflectable> newObject() override
		{
			return SPtr<IReflectable>(DataType::getRTTIStatic()->newRTTIObject());
		}

		/** @copydoc RTTIReflectablePtrFieldBase::getRTTIId */
		UINT32 getRTTIId() override
		{
			return DataType::getRTTIStatic()->getRTTIId();
		}

		/** @copydoc RTTIReflectablePtrFieldBase::getRTTIName */
		const String& getRTTIName() override
		{
			return DataType::getRTTIStatic()->getRTTIName();
		}


		/** @copydoc RTTIReflectablePtrFieldBase::getType */
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
