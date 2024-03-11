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
	 * Any class implementing this interface must implement the GetRtti() method, as well as a static GetRttiStatic()
	 * method, returning the same value as GetRtti(). Object returned by those methods is used for retrieving actual RTTI
	 * data about the class.
	 */
	class B3D_UTILITY_EXPORT IReflectable
	{
	public:
		virtual ~IReflectable() = default;

		/**
		 * Returns an interface you can use to access class' Run Time Type Information.
		 *
		 * @note
		 * You must derive your own version of RTTITypeBase, in which you may encapsulate all reflection specific operations.
		 */
		virtual RTTITypeBase* GetRtti() const = 0;

		/** Returns true if current RTTI class is derived from @p base (Or if it is the same type as base). */
		bool IsDerivedFrom(const RTTITypeBase* base) const;

		/** Returns an unique type identifier of the class. */
		u32 GetTypeId() const;

		/**
		 * Returns the type name of the class.
		 *
		 * @note	Name is not necessarily unique.
		 */
		const String& GetTypeName() const;

		/** Creates an empty instance of a class from a type identifier. */
		static SPtr<IReflectable> CreateInstanceFromTypeId(u32 rttiTypeId);

		/** Returns all available RTTI types. */
		static UnorderedMap<u32, RTTITypeBase*>& GetAllRttiTypes()
		{
			static UnorderedMap<u32, RTTITypeBase*> mAllRTTITypes;
			return mAllRTTITypes;
		}

		/** Returns class' RTTI type from type id. */
		static RTTITypeBase* GetRTTITypeFromTypeId(u32 rttiTypeId);

		/** @name Internal
		 *  @{
		 */

		/** Called by each type implementing RTTITypeBase, on program load. */
		static void RegisterRTTITypeInternal(RTTITypeBase* rttiType);

		/** Checks if the provided type id is unique. */
		static bool IsTypeIdDuplicateInternal(u32 typeId);

		/** Returns an interface you can use to access class' Run Time Type Information. */
		static RTTITypeBase* GetRttiStatic();

		/** @} */
	};

	/** @} */
} // namespace bs
