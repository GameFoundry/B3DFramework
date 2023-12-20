//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIManager.h"
#include "BsGUINavGroup.h"
#include "Resources/BsBuiltinResources.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIInteractableRTTI : public RTTIType<GUIInteractable, GUIRenderable, GUIInteractableRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIInteractable";
			return name;
		}

		u32 GetRttiId() { return TID_GUIInteractable; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

void GUIInteractable::NotifyStateFlagsChanged()
{
	if(mStyleSheetRuleInformation.StateRulesets != nullptr)
		mStyleSheetRuleInformation.CurrentStateRuleset = mStyleSheetRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags);
	else
		mStyleSheetRuleInformation.CurrentStateRuleset = GUIStyleSheetRuleset::kDefault;

	const GUIStyleSheetRules* inheritedRules = mStyleSheetRuleInformation.CurrentStateRuleset != nullptr ? &mStyleSheetRuleInformation.CurrentStateRuleset->Rules : nullptr;
	for(auto& psudoElementRuleInformation : mPseudoElementStyleSheetRules)
	{
		if(psudoElementRuleInformation.StateRulesets != nullptr)
			psudoElementRuleInformation.CurrentStateRuleset = psudoElementRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags, inheritedRules);
		else
			psudoElementRuleInformation.CurrentStateRuleset = GUIStyleSheetRuleset::kDefault;
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
		OnFocusChanged(true);
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(event.GetType() == GUICommandEventType::FocusLost)
	{
		OnFocusChanged(false);
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
	if(IsDestroyed())
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

void GUIInteractable::SetNavigationGroup(const SPtr<GUINavGroup>& navGroup)
{
	SPtr<GUINavGroup> currentNavGroup = GetNavigationGroup();
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
	SPtr<GUINavGroup> navGroup = GetNavigationGroup();
	if(navGroup != nullptr)
		navGroup->SetIndex(this, index);
}

SPtr<GUINavGroup> GUIInteractable::GetNavigationGroup() const
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

bool GUIInteractable::IsInBounds(const Vector2I& position) const
{
	return GetCachedClippedBounds().Contains(position);
}

SPtr<GUIContextMenu> GUIInteractable::GetContextMenu() const
{
	if(!IsDisabled())
		return mContextMenu;

	return nullptr;
}

void GUIInteractable::Destroy(GUIInteractable* element)
{
	if(element->mIsDestroyed)
		return;

	SPtr<GUINavGroup> currentNavGroup = element->GetNavigationGroup();
	if(currentNavGroup)
		currentNavGroup->UnregisterElement(element);

	if(element->mParent != nullptr)
		element->mParent->UnregisterChildElement(element);

	element->mIsDestroyed = true;

	GUIManager::Instance().QueueForDestroy(element);
}


RTTITypeBase* GUIInteractable::GetRttiStatic()
{
	return GUIInteractableRTTI::Instance();
}

RTTITypeBase* GUIInteractable::GetRtti() const
{
	return GetRttiStatic();
}
