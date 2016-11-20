//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsMonoPrerequisites.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**
	 * Encapsulates information about a single Mono (managed) property belonging to some managed class. This object
	 * also allows you to set or retrieve values to/from specific instances containing the property.
	 */
	class BS_MONO_EXPORT MonoProperty
	{
	public:
		/**	Returns the name of the property. */
		const String& getName() const { return mName; }

		/**
		 * Returns a boxed value contained in the property in the specified instance.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @return					A boxed value of the property.
		 */
		MonoObject* get(MonoObject* instance) const;

		/**
		 * Sets a value of the property in the specified instance. 
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		void set(MonoObject* instance, void* value) const;

		/**
		 * Returns a boxed value contained in the property in the specified instance. Used for properties with indexers.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	index		Index of the value to retrieve.
		 * @return					A boxed value of the property.
		 */
		MonoObject* getIndexed(MonoObject* instance, UINT32 index) const;

		/**
		 * Sets a value of the property in the specified instance. Used for properties with indexers.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	index		Index of the value to set.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		void setIndexed(MonoObject* instance, UINT32 index, void* value) const;

		/** Checks does the property contains indexed data, or just a single value. */
		bool isIndexed() const;

		/**	Returns the data type the property holds. */
		MonoClass* getReturnType() const;

		/**	Checks if property has an attribute of the specified type. */
		bool hasAttribute(MonoClass* monoClass);

		/**
		 * Returns an instance of an attribute of the specified type. Returns null if the property doesn't have such an
		 * attribute.
		 */
		MonoObject* getAttribute(MonoClass* monoClass);

		/**	
		 * Returns property visibility in the class. If getter/setter methods have different visibility, the more 
		 * restrictive one is returned.
		 */
		MonoMemberVisibility getVisibility();
	private:
		friend class MonoClass;

		MonoProperty(::MonoProperty* monoProp);

		/** 
		 * Some property data is not initialized by default on creation (with the assumption it will never be accessed). 
		 * This method will initialize that data.
		 */
		void initializeDeferred() const;

		String mName;
		::MonoProperty* mProperty;
		::MonoMethod* mGetMethod;
		::MonoMethod* mSetMethod;

		mutable MonoClass* mReturnType;
		mutable bool mIsIndexed;
		mutable bool mIsFullyInitialized;
	};

	/** @} */
}