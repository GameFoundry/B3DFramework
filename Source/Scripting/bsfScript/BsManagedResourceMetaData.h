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
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedResourceMetaData : public ResourceMetaData
	{
	public:
		B3D_SCRIPT_EXPORT()
		String TypeNamespace;

		B3D_SCRIPT_EXPORT()
		String TypeName;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ManagedResourceMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace bs
