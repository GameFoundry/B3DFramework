//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"

namespace bs
{
	/** @addtogroup Renderer
	 *  @{
	 */
	
	/** Base class whose implementations contain settings that control post-process operations during rendering. */
	struct BS_CORE_EXPORT PostProcessSettings : public IReflectable
	{
		PostProcessSettings() { }
		virtual ~PostProcessSettings() { }

		/** @name Internal
		 *  @{
		 */

		/** 
		 * Populates the provided buffer with data that can be used for syncing between two versions of this object.
		 * Apply the retrieved buffer via _setSyncData().
		 *
		 * @param[in]		buffer		Pre-allocated buffer to store the sync data in. Set to null to calculate the size
		 *								of the required buffer.
		 * @param[in, out]	size		Size of the provided allocated buffer. Or if the buffer is null, this parameter will
		 *								contain the required buffer size when the method executes.
		 */
		virtual void _getSyncData(UINT8* buffer, UINT32& size) = 0;

		/** 
		 * Updates the stored data from the provided buffer, allowing changes to be transfered between two versions of this
		 * object. Buffer must be retrieved from _getSyncData(). 
		 *
		 * @param[in]		buffer		Buffer containing the dirty data.
		 * @param[in, out]	size		Size of the provided buffer.
		 */
		virtual void _setSyncData(UINT8* buffer, UINT32 size) = 0;

		/** @} */
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class PostProcessSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
