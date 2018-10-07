//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsAny.h"

namespace bs
{
	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Interface implemented by classes that provide run time type information.
	 * 			
	 * @note	
	 * Any class implementing this interface must implement the getRTTI() method, as well as a static getRTTIStatic() 
	 * method, returning the same value as getRTTI(). Object returned by those methods is used for retrieving actual RTTI 
	 * data about the class.
	 */
	class BS_UTILITY_EXPORT IReflectable
	{
	public:
		virtual ~IReflectable() = default;

		/**
		 * Returns an interface you can use to access class' Run Time Type Information.
		 *
		 * @note	
		 * You must derive your own version of RTTITypeBase, in which you may encapsulate all reflection specific operations. 
		 */
		virtual RTTITypeBase* getRTTI() const = 0;

		/** Returns all available RTTI types. */
		static UnorderedMap<UINT32, RTTITypeBase*>& getAllRTTITypes()
		{
			static UnorderedMap<UINT32, RTTITypeBase*> mAllRTTITypes;
			return mAllRTTITypes;
		}

		/** Returns true if current RTTI class is derived from @p base (Or if it is the same type as base). */
		bool isDerivedFrom(RTTITypeBase* base);

		/** Returns an unique type identifier of the class. */
		UINT32 getTypeId() const;

		/**
		 * Returns the type name of the class.
		 *
		 * @note	Name is not necessarily unique.
		 */
		const String& getTypeName() const;

		/** Creates an empty instance of a class from a type identifier. */
		static SPtr<IReflectable> createInstanceFromTypeId(UINT32 rttiTypeId);

		/** @name Internal
		 *  @{
		 */

		/** Called by each type implementing RTTITypeBase, on program load. */
		static void _registerRTTIType(RTTITypeBase* rttiType);

		/** Returns class' RTTI type from type id. */
		static RTTITypeBase* _getRTTIfromTypeId(UINT32 rttiTypeId);

		/** Checks if the provided type id is unique. */
		static bool _isTypeIdDuplicate(UINT32 typeId);

		/**
		 * Iterates over all RTTI types and reports any circular references (for example one type having a field referencing
		 * another type, and that type having a field referencing the first type). Circular references are problematic
		 * because when serializing the system cannot determine in which order they should be resolved. In that case user
		 * should use RTTI_Flag_WeakRef to mark one of the references as weak. This flags tells the system that the reference
		 * may be resolved in an undefined order, but also no longer guarantees that object assigned to that field during
		 * deserialization will be fully deserialized itself, as that might be delayed to a later time.
		 */
		static void _checkForCircularReferences();

		/** Returns an interface you can use to access class' Run Time Type Information. */
		static RTTITypeBase* getRTTIStatic();

		/** @} */
	};

	/** @} */
}
