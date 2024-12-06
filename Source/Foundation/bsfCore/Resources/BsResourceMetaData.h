//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreObject/BsCoreObject.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Class containing meta-information describing a resource. */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT() ResourceMetaData : public IReflectable, public IScriptExportable
	{
		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ResourceMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace bs
