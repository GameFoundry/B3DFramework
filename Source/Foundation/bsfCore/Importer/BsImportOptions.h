//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** 
	 * Base class for creating import options from. Import options are specific for each importer and control how is data 
	 * imported.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf) ImportOptions : public IReflectable
	{
	public:
		virtual ~ImportOptions() = default;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}