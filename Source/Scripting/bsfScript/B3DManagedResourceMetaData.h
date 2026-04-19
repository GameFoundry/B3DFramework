//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Resources/B3DResourceMetaData.h"

namespace b3d
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
} // namespace b3d
