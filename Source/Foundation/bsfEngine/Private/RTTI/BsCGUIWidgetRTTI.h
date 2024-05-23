//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "GUI/BsCGUIWidget.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT CGUIWidgetRTTI : public RTTIType<CGUIWidget, Component, CGUIWidgetRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "CGUIWidget";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CGUIWidget;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<CGUIWidget>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
