//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "BsManagedResourceMetaData.h"

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

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
