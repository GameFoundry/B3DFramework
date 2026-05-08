//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "GUI/B3DGUIWidget.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT GUIWidgetRTTI : public TRTTIType<GUIWidget, Component, GUIWidgetRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "GUIWidget";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GUIWidget;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<GUIWidget>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
