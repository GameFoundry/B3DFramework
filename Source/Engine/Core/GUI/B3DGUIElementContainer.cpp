//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIElementContainer.h"
#include "GUI/B3DGUIManager.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
{
	class B3D_EXPORT GUIElementContainerRTTI : public TRTTIType<GUIElementContainer, GUIInteractable, GUIElementContainerRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIElementContainer";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIElementContainer; }

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace b3d

GUIElementContainer::GUIElementContainer(const GUISizeConstraints& dimensions, const char* style, GUIElementOptions options)
	: GUIInteractable(style, dimensions, options)
{
	mOptionFlags.Set(GUIElementOption::ClickThrough);
}

GUIElementContainer::GUIElementContainer(const GUISizeConstraints& dimensions, const String& style, GUIElementOptions options)
	: GUIInteractable(style, dimensions, options)
{
	mOptionFlags.Set(GUIElementOption::ClickThrough);
}

GUILogicalSize GUIElementContainer::CalculateUnconstrainedOptimalSize() const
{
	return GUILogicalSize(kZeroTag);
}

void GUIElementContainer::SetFocus(bool enabled, bool clear)
{
	if(mFocusElement)
		mFocusElement->SetFocus(enabled, clear);
	else
		GUIInteractable::SetFocus(enabled, clear);
}

bool GUIElementContainer::DoOnCommandEvent(const GUICommandEvent& ev)
{
	// Make sure to pass through focus events to elements below
	if(ev.GetType() == GUICommandEventType::FocusGained)
		return false;
	else if(ev.GetType() == GUICommandEventType::FocusLost)
		return false;

	return GUIInteractable::DoOnCommandEvent(ev);
}

RTTIType* GUIElementContainer::GetRttiStatic()
{
	return GUIElementContainerRTTI::Instance();
}

RTTIType* GUIElementContainer::GetRtti() const
{
	return GetRttiStatic();
}
