//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

#if B3D_WITH_EDITOR
	class B3D_EXPORT IEditorSceneInstanceRTTI : public TRTTIType<IEditorSceneInstance, IReflectable, IEditorSceneInstanceRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "IEditorSceneInstance";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_IEditorSceneInstance;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return nullptr;
		}
	};
#endif

	/** @} */
	/** @endcond */
} // namespace b3d
