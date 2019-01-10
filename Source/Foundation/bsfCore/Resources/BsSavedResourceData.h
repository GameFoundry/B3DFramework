//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Contains information about a resource saved to the disk.
	 *
	 * @note Purpose of this class is primarily to be a wrapper around a list of objects to make serialization easier.
	 */
	class BS_CORE_EXPORT SavedResourceData : public IReflectable
	{
	public:
		SavedResourceData() = default;
		SavedResourceData(const Vector<UUID>& dependencies, bool allowAsync, UINT32 compressionMethod);

		/**	Returns a list of all resource dependencies. */
		const Vector<UUID>& getDependencies() const { return mDependencies; }

		/**	Returns true if this resource is allow to be asynchronously loaded. */
		bool allowAsyncLoading() const { return mAllowAsync; }

		/** Returns the method used for compressing the resource. 0 if none. */
		UINT32 getCompressionMethod() const { return mCompressionMethod; }

	private:
		Vector<UUID> mDependencies;
		bool mAllowAsync = true;
		UINT32 mCompressionMethod = 0;

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	public:
		friend class SavedResourceDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
