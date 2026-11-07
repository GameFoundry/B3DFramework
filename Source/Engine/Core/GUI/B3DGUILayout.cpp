//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUILayout.h"
#include "GUI/B3DGUIUtility.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUICommandEvent.h"
#include "GUI/StyleSheet/B3DGUIStyleSheet.h"
#include "Reflection/B3DRTTIType.h"
#include "Utility/B3DTime.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
{
	class B3D_EXPORT GUILayoutRTTI : public TRTTIType<GUILayout, GUIInteractable, GUILayoutRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "GUILayout";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUILayout; }

		TShared<IReflectable> NewRttiObject() override { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

GUILayout::GUILayout()
	: GUIInteractable(StringUtility::kBlank, GUISizeConstraints::Create(), GUIElementOption::ClickThrough)
{}

GUILayout::GUILayout(const String& styleClass, const GUISizeConstraints& dimensions)
	: GUIInteractable(styleClass, dimensions, GUIElementOption::ClickThrough)
{
	mBackgroundSprite.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());
}

void GUILayout::SetInteractive(bool interactive)
{
	if(mIsInteractive == interactive)
		return;

	const bool wasLayoutOnly = IsLayoutOnly();

	mIsInteractive = interactive;

	// An interactive layout handles clicks like any other clickable element, so it stops letting them (and focus) through to the elements behind it
	if(interactive)
		mOptionFlags.Unset(GUIElementOption::ClickThrough);
	else
	{
		mOptionFlags.Set(GUIElementOption::ClickThrough);
		RemoveStateFlags(kGUIInteractionStates);
	}

	if(wasLayoutOnly != IsLayoutOnly())
		RefreshStyleAndRegistration(wasLayoutOnly);
}

void GUILayout::SetChecked(bool value)
{
	if(value)
		AddStateFlags(GUIElementState::Checked);
	else
		RemoveStateFlags(GUIElementState::Checked);
}

void GUILayout::NotifyStyleChanged()
{
	mBackgroundSprite.SetAnimationStartTime(GetTime().GetRealTimeInSeconds());
}

void GUILayout::UpdateRenderElements()
{
	mRenderElements.Clear();

	if(!IsLayoutOnly())
		GUISpriteHelper::BuildSpriteRenderElements(*this, mBackgroundSprite);

	Super::UpdateRenderElements();
}

bool GUILayout::DoOnMouseEvent(const GUIMouseEvent& event)
{
	if(!mIsInteractive || mOptionFlags.IsSet(GUIElementOption::IgnorePointerEvents))
		return false;

	// The layout sits behind every element inside it, so routed events only reach it if none of those elements handled
	// them. Its hover and active states come from the notifications instead, which it receives while the pointer is over
	// any element inside it.
	switch(event.GetType())
	{
	case GUIMouseEventType::MouseEnter:
		if(!IsDisabled())
			AddStateFlags(GUIElementState::Hover);

		return false;
	case GUIMouseEventType::MouseLeave:
		RemoveStateFlags(GUIElementState::Hover);
		return false;
	case GUIMouseEventType::MousePressBegin:
		AddStateFlags(GUIElementState::Active);
		return false;
	case GUIMouseEventType::MousePressEnd:
		RemoveStateFlags(GUIElementState::Active);
		return false;
	case GUIMouseEventType::MouseDown:
		return !IsDisabled() && !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	case GUIMouseEventType::MouseUp:
		if(IsDisabled())
			return false;

		if(event.GetButton() == GUIMouseButton::Right)
			OnContextMenu();
		else
			OnClick();

		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	case GUIMouseEventType::MouseDoubleClick:
		if(IsDisabled() || event.GetButton() == GUIMouseButton::Right)
			return false;

		OnDoubleClick();
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	default:
		return false;
	}
}

bool GUILayout::DoOnCommandEvent(const GUICommandEvent& event)
{
	const bool baseReturnValue = Super::DoOnCommandEvent(event);

	if(!mIsInteractive)
		return baseReturnValue;

	if(event.GetType() == GUICommandEventType::FocusGained)
	{
		if(!IsDisabled())
			AddStateFlags(GUIElementState::Focus);
	}
	else if(event.GetType() == GUICommandEventType::FocusLost)
		RemoveStateFlags(GUIElementState::Focus);

	return baseReturnValue;
}

GUILogicalSize GUILayout::AddStylePaddingAndBorder(const GUILogicalSize& childrenSize) const
{
	if(IsLayoutOnly() || mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return childrenSize;

	return GUIUtility::CalculateSizeWithPaddingAndBorder(childrenSize, mStyleSheetRuleInformation.CurrentStateRuleset->Rules);
}

GUILayoutData GUILayout::GetChildLayoutData() const
{
	GUILayoutData childData = mLayoutData;

	// Children inherit the layout's depth, and a styled layout draws its background at that depth, so without this the
	// background would land at exactly the depth of a nested layout's background
	if(IsLayoutOnly())
		return childData;

	const bool hasInfiniteRange = childData.DepthRangeMin == (u16)-1;
	if(!hasInfiniteRange && childData.DepthRangeMin == 0)
		return childData; // No room left in the parent panel's depth range

	childData.SetPanelDepth((i16)Math::Max((i32)childData.GetPanelDepth() - 1, (i32)std::numeric_limits<i16>::min()));

	if(!hasInfiniteRange)
		childData.DepthRangeMin--;

	return childData;
}

GUILogicalArea GUILayout::GetChildLayoutArea() const
{
	if(IsLayoutOnly() || mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return GUILogicalArea(GUILogicalPoint(0, 0), mLayoutData.Size);

	return GetContentBounds();
}

void GUILayout::AddElement(GUIElement* element)
{
	if(element->IsPendingDestroy())
		return;

	RegisterChildElement(element);
}

void GUILayout::InsertElement(u32 index, GUIElement* element)
{
	if(!B3D_ENSURE_LOG(index <= (u32)mChildren.size(), "Index out of range: {0}. Valid range: 0 .. {1}", index, (u32)mChildren.size()))
		return;

	if(element->IsPendingDestroy())
		return;

	GUIElement* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Insert(mChildren.begin() + index, element);

	element->SetActiveRecursive(IsActive());
	element->SetHiddenRecursive(IsHidden());
	element->SetDisabledRecursive(IsDisabled());

	if(mCulling != nullptr)
		mCulling->RegisterElement(element);

	MarkLayoutAsDirty();
}

void GUILayout::RemoveElement(GUIElement* element)
{
	UnregisterChildElement(element);
}

void GUILayout::RemoveElementAt(u32 index)
{
	if(!B3D_ENSURE_LOG(index < (u32)mChildren.size(), "Index out of range: {0}. Valid range: 0 .. {1}", index, (u32)mChildren.size()))
		return;

	GUIElement* child = mChildren[index];
	mChildren.erase(mChildren.begin() + index);

	child->SetParent(nullptr);

	if(mCulling != nullptr)
		mCulling->UnregisterElement(child);

	MarkLayoutAsDirty();
}

void GUILayout::Clear()
{
	if(mCulling != nullptr)
	{
		mCulling->ClearElements();
		mCulling = B3DMakeUnique<GUICulling>(mChildren);
	}

	DestroyChildElements();
}

void GUILayout::SetEnableCulling(bool enable)
{
	const bool isCullingEnabled = mCulling != nullptr;
	if(isCullingEnabled == enable)
		return;
	
	if(enable)
		mCulling = B3DMakeUnique<GUICulling>(mChildren);
	else
	{
		mCulling->ClearElements();
		mCulling = nullptr;
	}
}

void GUILayout::RegisterChildElement(GUIElement* element)
{
	Super::RegisterChildElement(element);

	if(mCulling != nullptr)
		mCulling->RegisterElement(element);
}

void GUILayout::UnregisterChildElement(GUIElement* element)
{
	if(mCulling != nullptr)
		mCulling->UnregisterElement(element);

	Super::UnregisterChildElement(element);
}

void GUILayout::UpdateAbsoluteCoordinates(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea)
{
	// Account for the fact that the content size may be larger than the calculated layout size (e.g. a scroll area with more elements than it can display)
	const GUILogicalSize optimalContentSize = CalculateConstrainedOptimalSize();
	const GUILogicalSize contentSize(Math::Max(optimalContentSize.Width, mLayoutData.Size.Width), Math::Max(optimalContentSize.Height, mLayoutData.Size.Height));

	UpdateAbsoluteCoordinatesWithExplicitContentSize(parentOrigin, parentScale, parentVisibleArea, contentSize);
}

void GUILayout::UpdateAbsoluteCoordinatesForChildren()
{
	if(mCulling != nullptr)
	{
		const Area2 relativeClippedArea = WidgetToElementSpace(mAbsoluteClippedArea).To<float>();
		mCulling->UpdateVisibleElements(relativeClippedArea);

		for(auto& visibleElement : mCulling->GetVisibleElements())
		{
			visibleElement->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
			visibleElement->UpdateAbsoluteCoordinatesForChildren();
		}
	}
	else
	{
		for(auto& childElement : mChildren)
		{
			childElement->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
			childElement->UpdateAbsoluteCoordinatesForChildren();
		}
	}
}

RTTIType* GUILayout::GetRttiStatic()
{
	return GUILayoutRTTI::Instance();
}

RTTIType* GUILayout::GetRtti() const
{
	return GetRttiStatic();
}
