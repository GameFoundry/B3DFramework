//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"

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
		SavedResourceData();
		SavedResourceData(const Vector<String>& dependencies, bool allowAsync, UINT32 compressionMethod);

		/**	Returns a list of all resource dependencies. */
		const Vector<String>& getDependencies() const { return mDependencies; }

		/**	Returns true if this resource is allow to be asynchronously loaded. */
		bool allowAsyncLoading() const { return mAllowAsync; }

		/** Returns the method used for compressing the resource. 0 if none. */
		UINT32 getCompressionMethod() const { return mCompressionMethod; }

	private:
		Vector<String> mDependencies;
		bool mAllowAsync;
		UINT32 mCompressionMethod;

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