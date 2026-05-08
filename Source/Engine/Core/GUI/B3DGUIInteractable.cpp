//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUIManager.h"
#include "B3DGUINavGroup.h"
#include "Resources/B3DBuiltinResources.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
{
	class B3D_EXPORT GUIInteractableRTTI : public TRTTIType<GUIInteractable, GUIRenderable, GUIInteractableRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIInteractable";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIInteractable; }

		TShared<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

void GUIInteractable::NotifyStateFlagsChanged()
{
	if(mStyleSheetRuleInformation.StateRulesets != nullptr)
		mStyleSheetRuleInformation.CurrentStateRuleset = mStyleSheetRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags);
	else
		mStyleSheetRuleInformation.CurrentStateRuleset = GUIStyleSheetRuleset::kDefault;

	const GUIStyleSheetRules* inheritedRules = mStyleSheetRuleInformation.CurrentStateRuleset != nullptr ? &mStyleSheetRuleInformation.CurrentStateRuleset->Rules : nullptr;
	for(auto& pseudoElementRuleInformation : mPseudoElementStyleSheetRules)
	{
		if(pseudoElementRuleInformation.StateRulesets != nullptr)
			pseudoElementRuleInformation.CurrentStateRuleset = pseudoElementRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags, inheritedRules);
		else
			pseudoElementRuleInformation.CurrentStateRuleset = GUIStyleSheetRuleset::kDefault;
	}
}

GUIInteractable::GUIInteractable(String styleClass, const GUISizeConstraints& dimensions, GUIElementOptions options)
	: GUIRenderable(std::move(styleClass), dimensions), mOptionFlags(options)
{
}

GUIInteractable::GUIInteractable(const char* styleClass, const GUISizeConstraints& dimensions, GUIElementOptions options)
	: GUIRenderable(styleClass, dimensions), mOptionFlags(options)
{
}

bool GUIInteractable::IsInInteractionBounds(const GUIPhysicalPoint& position) const
{
	return GetAbsoluteClippedArea().Contains(position);
}

bool GUIInteractable::DoOnMouseEvent(const GUIMouseEvent& event)
{
	return false;
}

bool GUIInteractable::DoOnTextInputEvent(const GUITextInputEvent& event)
{
	return false;
}

bool GUIInteractable::DoOnCommandEvent(const GUICommandEvent& event)
{
	if(event.GetType() == GUICommandEventType::FocusGained)
	{
		OnFocusGained();
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(event.GetType() == GUICommandEventType::FocusLost)
	{
		OnFocusLost();
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

bool GUIInteractable::DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& event)
{
	return false;
}

void GUIInteractable::ChangeParentWidget(GUIWidget* widget)
{
	if(IsPendingDestroy())
		return;

	bool widgetChanged = false;
	if(mParentWidget != widget)
	{
		// Unregister from current widget's nav-group
		if(!mNavigationGroup && mParentWidget)
			mParentWidget->GetDefaultNavGroupInternal()->UnregisterElement(this);

		widgetChanged = true;
	}

	GUIRenderable::ChangeParentWidget(widget);

	if(widgetChanged)
	{
		// Register with the new widget's nav-group
		if(!mNavigationGroup && mParentWidget)
			mParentWidget->GetDefaultNavGroupInternal()->RegisterElement(this);
	}
}

void GUIInteractable::SetNavigationGroup(const TShared<GUINavGroup>& navGroup)
{
	TShared<GUINavGroup> currentNavGroup = GetNavigationGroup();
	if(currentNavGroup == navGroup)
		return;

	if(currentNavGroup)
		currentNavGroup->UnregisterElement(this);

	if(navGroup)
		navGroup->RegisterElement(this);

	mNavigationGroup = navGroup;
}

void GUIInteractable::SetNavigationGroupIndex(i32 index)
{
	TShared<GUINavGroup> navGroup = GetNavigationGroup();
	if(navGroup != nullptr)
		navGroup->SetIndex(this, index);
}

TShared<GUINavGroup> GUIInteractable::GetNavigationGroup() const
{
	if(mNavigationGroup)
		return mNavigationGroup;

	if(mParentWidget)
		return mParentWidget->GetDefaultNavGroupInternal();

	return nullptr;
}

void GUIInteractable::SetFocus(bool enabled, bool clear)
{
	GUIManager::Instance().SetFocus(this, enabled, clear);
}

void GUIInteractable::AddStateFlags(GUIElementStateFlags flags)
{
	if(mStateFlags.IsSetAll(flags))
		return;

	mStateFlags |= flags;

	NotifyStateFlagsChanged();
	MarkContentAsDirty();
}

void GUIInteractable::RemoveStateFlags(GUIElementStateFlags flags)
{
	if(!mStateFlags.IsSetAny(flags))
		return;

	mStateFlags &= ~flags;

	NotifyStateFlagsChanged();
	MarkContentAsDirty();
}

TShared<GUIContextMenu> GUIInteractable::GetContextMenu() const
{
	if(!IsDisabled())
		return mContextMenu;

	return nullptr;
}

void GUIInteractable::Destroy()
{
	if(mIsPendingDestroy)
		return;

	const TShared<GUINavGroup> navigationGroup = GetNavigationGroup();
	if(navigationGroup != nullptr)
		navigationGroup->UnregisterElement(this);

	GUIRenderable::Destroy();
}

RTTIType* GUIInteractable::GetRttiStatic()
{
	return GUIInteractableRTTI::Instance();
}

RTTIType* GUIInteractable::GetRtti() const
{
	return GetRttiStatic();
}
