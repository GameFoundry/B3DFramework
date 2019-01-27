//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Resources/BsResourceMetaData.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**	Resource meta-data for user-defined managed resources. */
	class BS_SCR_BE_EXPORT ManagedResourceMetaData : public ResourceMetaData
	{
	public:
		String typeNamespace;
		String typeName;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ManagedResourceMetaDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}