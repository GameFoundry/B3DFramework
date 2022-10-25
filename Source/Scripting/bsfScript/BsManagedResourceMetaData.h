//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Resources/BsResourceMetaData.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Resource meta-data for user-defined managed resources. */
	class BS_SCR_BE_EXPORT ManagedResourceMetaData : public ResourceMetaData
	{
	public:
		String TypeNamespace;
		String TypeName;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ManagedResourceMetaDataRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
