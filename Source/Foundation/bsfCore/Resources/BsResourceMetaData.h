//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Class containing meta-information describing a resource. */
	class BS_CORE_EXPORT ResourceMetaData : public IReflectable
	{
	public:
		String displayName;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ResourceMetaDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}