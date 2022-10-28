//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	struct SerializationContext;

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Serialization-Internal
	 *  @{
	 */

	/**
	 * Represents an interface RTTI objects need to implement if they want to provide custom method of comparing
	 * their objects for equality.
	 */
	class BS_UTILITY_EXPORT ICompare
	{
	public:
		virtual ~ICompare() = default;

		/** Checks if two IReflectable objects are equal. */
		virtual bool Run(IReflectable& a, IReflectable& b) = 0;
	};

	/** Compares native IReflectable objects for equality. */
	class BS_UTILITY_EXPORT BinaryCompare : public ICompare
	{
	public:
		BinaryCompare();
		virtual ~BinaryCompare() = default;

		bool Run(IReflectable& a, IReflectable& b) override;

	protected:
		/**
		 * Checks if two IReflectable objects are equal. Inserts the results into an object map so multiple references
		 * to the same object don't need to be checked twice.
		 */
		bool Compare(IReflectable& a, IReflectable& b);

		UnorderedSet<IReflectable*> mObjectMap;
		SerializationContext* mContext = nullptr;
		FrameAlloc* mAlloc = nullptr;
	};

	/** @} */
	/** @} */
} // namespace bs
