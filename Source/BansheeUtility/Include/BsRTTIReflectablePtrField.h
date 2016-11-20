//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsRTTIField.h"
#include "BsIReflectable.h"

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
		virtual SPtr<IReflectable> getValue(void* object) = 0;

		/**
		 * Retrieves the IReflectable value from an array on the provided instance and index.
		 * 			
		 * @note	Field type must be an array.
		 */
		virtual SPtr<IReflectable> getArrayValue(void* object, UINT32 index) = 0;

		/**
		 * Sets the IReflectable value in the provided instance.
		 * 			
		 * @note	Field type must not be an array.
		 */
		virtual void setValue(void* object, SPtr<IReflectable> value) = 0;

		/**
		 * Sets the IReflectable value in an array on the provided instance and index.
		 * 			
		 * @note	Field type must be an array.
		 */
		virtual void setArrayValue(void* object, UINT32 index, SPtr<IReflectable> value) = 0;

		/** Creates a new object of the field type. */
		virtual SPtr<IReflectable> newObject() = 0;

		/** Returns the RTTI identifier of the class owning the field. */
		virtual UINT32 getRTTIId() = 0;

		/** Returns the name of the class owning the field. */
		virtual const String& getRTTIName() = 0;

		/** @copydoc RTTIField::hasDynamicSize */
		bool hasDynamicSize() override { return true; }

		/** Retrieves the RTTI object for the type the field contains. */
		virtual RTTITypeBase* getType() = 0;
	};

	/** Reflectable field containing a pointer to a specific type with RTTI implemented.  */
	template <class DataType, class ObjectType>
	struct RTTIReflectablePtrField : public RTTIReflectablePtrFieldBase
	{
		/**
		 * Initializes a field pointing to a single data type implementing IReflectable interface.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a 
		 *							small data type that can be used for efficiently serializing data to disk and similar. 
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field. Must be a specific signature: DataType*(ObjectType*)
		 * @param[in]	setter  	The setter method for the field. Must be a specific signature: void(ObjectType*, DataType*)
		 * @param[in]	flags		Various flags you can use to specialize how systems handle this field. See "RTTIFieldFlag".
		 */
		void initSingle(const String& name, UINT16 uniqueId, Any getter, Any setter, UINT64 flags)
		{
			initAll(getter, setter, nullptr, nullptr, name, uniqueId, false, SerializableFT_ReflectablePtr, flags);
		}

		/**
		 * Initializes a field containing an array of pointers to data types implementing IReflectable interface.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a 
		 *							small data type that can be used for efficiently serializing data to disk and similar. 
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field. Must be a specific signature: DataType*(ObjectType*, UINT32)
		 * @param[in]	getSize 	Getter method that returns the size of an array. Must be a specific signature: UINT32(ObjectType*)
		 * @param[in]	setter  	The setter method for the field. Must be a specific signature: void(ObjectType*, UINT32, DataType*)
		 * @param[in]	setSize 	Setter method that allows you to resize an array. Can be null. Must be a specific signature: void(ObjectType*, UINT32)
		 * @param[in]	flags		Various flags you can use to specialize how systems handle this field. See "RTTIFieldFlag".
		 */
		void initArray(const String& name, UINT16 uniqueId, Any getter,
			Any getSize, Any setter, Any setSize, UINT64 flags)
		{
			initAll(getter, setter, getSize, setSize, name, uniqueId, true, SerializableFT_ReflectablePtr, flags);
		}

		/** @copydoc RTTIField::getTypeSize */
		UINT32 getTypeSize() override
		{
			return 0; // Complex types don't store size the conventional way
		}

		/** @copydoc RTTIReflectablePtrFieldBase::getValue */
		SPtr<IReflectable> getValue(void* object) override
		{
			checkIsArray(false);

			ObjectType* castObjType = static_cast<ObjectType*>(object);
			std::function<SPtr<DataType>(ObjectType*)> f = any_cast<std::function<SPtr<DataType>(ObjectType*)>>(valueGetter);
			SPtr<IReflectable> castDataType = f(castObjType);

			return castDataType;
		}

		/** @copydoc RTTIReflectablePtrFieldBase::getArrayValue */
		SPtr<IReflectable> getArrayValue(void* object, UINT32 index) override
		{
			checkIsArray(true);

			ObjectType* castObjType = static_cast<ObjectType*>(object);
			std::function<SPtr<DataType>(ObjectType*, UINT32)> f = any_cast<std::function<SPtr<DataType>(ObjectType*, UINT32)>>(valueGetter);

			SPtr<IReflectable> castDataType = f(castObjType, index);
			return castDataType;
		}

		/** @copydoc RTTIReflectablePtrFieldBase::setValue */
		void setValue(void* object, SPtr<IReflectable> value) override
		{
			checkIsArray(false);

			if(valueSetter.empty())
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + mName + ") has no setter.");
			}

			ObjectType* castObjType = static_cast<ObjectType*>(object);
			SPtr<DataType> castDataObj = std::static_pointer_cast<DataType>(value);
			std::function<void(ObjectType*, SPtr<DataType>)> f = any_cast<std::function<void(ObjectType*, SPtr<DataType>)>>(valueSetter);
			f(castObjType, castDataObj);
		}

		/** @copydoc RTTIReflectablePtrFieldBase::setArrayValue */
		void setArrayValue(void* object, UINT32 index, SPtr<IReflectable> value) override
		{
			checkIsArray(true);

			if(valueSetter.empty())
			{
				BS_EXCEPT(InternalErrorException, 
					"Specified field (" + mName + ") has no setter.");
			}

			ObjectType* castObjType = static_cast<ObjectType*>(object);
			SPtr<DataType> castDataObj = std::static_pointer_cast<DataType>(value);
			std::function<void(ObjectType*, UINT32, SPtr<DataType>)> f = any_cast<std::function<void(ObjectType*, UINT32, SPtr<DataType>)>>(valueSetter);
			f(castObjType, index, castDataObj);
		}

		/** @copydoc RTTIField::setArraySize */
		UINT32 getArraySize(void* object) override
		{
			checkIsArray(true);

			std::function<UINT32(ObjectType*)> f = any_cast<std::function<UINT32(ObjectType*)>>(arraySizeGetter);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			return f(castObject);
		}

		/** @copydoc RTTIField::setArraySize */
		void setArraySize(void* object, UINT32 size) override
		{
			checkIsArray(true);

			if(arraySizeSetter.empty())
			{
				BS_EXCEPT(InternalErrorException, 
					"Specified field (" + mName + ") has no array size setter.");
			}

			std::function<void(ObjectType*, UINT32)> f = any_cast<std::function<void(ObjectType*, UINT32)>>(arraySizeSetter);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			f(castObject, size);
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
	};

	/** @} */
	/** @} */
}