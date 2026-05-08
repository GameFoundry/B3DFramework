//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "B3DManagedResourceMetaData.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedResourceMetaDataRTTI : public TRTTIType<ManagedResourceMetaData, ResourceMetaData, ManagedResourceMetaDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(TypeNamespace, 0)
			B3D_RTTI_MEMBER(TypeName, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ManagedResourceMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedResourceMetaData;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
