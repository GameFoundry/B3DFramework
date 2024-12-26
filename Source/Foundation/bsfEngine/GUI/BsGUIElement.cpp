//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUISpace.h"
#include "GUI/BsGUIInteractable.h"
#include "Error/BsException.h"
#include "GUI/BsGUIWidget.h"
#include "BsGUIManager.h"
#include "StyleSheet/BsGUIStyleSheet.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIElementRTTI : public TRTTIType<GUIElement, IReflectable, GUIElementRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIElement";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIElement; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

GUIElement::GUIElement(const GUISizeConstraints& dimensions)
	: mSizeConstraints(dimensions)
{}

void GUIElement::SetPosition(i32 x, i32 y)
{
	mSizeConstraints.X = x;
	mSizeConstraints.Y = y;

	// Note: I could call _markMeshAsDirty with a little more work. If parent is layout then this call can be ignored
	// and if it's a panel, we can immediately change the position without a full layout rebuild.
	MarkLayoutAsDirty();
}

void GUIElement::SetSize(u32 width, u32 height)
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::FixedWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime | GUISizeConstraintFlag::FixedHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
	mSizeConstraints.MinWidth = mSizeConstraints.MaxWidth = width;
	mSizeConstraints.MinHeight = mSizeConstraints.MaxHeight = height;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetWidth(u32 width)
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::FixedWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime;
	mSizeConstraints.MinWidth = mSizeConstraints.MaxWidth = width;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetFlexibleWidth(u32 minWidth, u32 maxWidth)
{
	if(maxWidth < minWidth)
		std::swap(minWidth, maxWidth);

	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::WidthOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::FixedWidth);
	mSizeConstraints.MinWidth = minWidth;
	mSizeConstraints.MaxWidth = maxWidth;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetHeight(u32 height)
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::FixedHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
	mSizeConstraints.MinHeight = mSizeConstraints.MaxHeight = height;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::SetFlexibleHeight(u32 minHeight, u32 maxHeight)
{
	if(maxHeight < minHeight)
		std::swap(minHeight, maxHeight);

	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints.Flags |= GUISizeConstraintFlag::HeightOverridenAtRuntime;
	mSizeConstraints.Flags.Unset(GUISizeConstraintFlag::FixedHeight);
	mSizeConstraints.MinHeight = minHeight;
	mSizeConstraints.MaxHeight = maxHeight;

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

void GUIElement::ResetDimensions()
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints = GUISizeConstraints::Create();

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

Rect2I GUIElement::CalculateBoundsRelativeTo(GUIElement* relativeTo)
{
	if(relativeTo == nullptr)
		relativeTo = mPanelParent;

	Rect2I anchorBounds;
	if(relativeTo != nullptr)
		anchorBounds = relativeTo->GetBounds();

	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);

	Rect2I bounds = mLayoutData.Area;
	bounds.X -= anchorBounds.X;
	bounds.Y -= anchorBounds.Y;

	return bounds;
}

void GUIElement::SetBounds(const Rect2I& bounds)
{
	SetPosition(bounds.X, bounds.Y);
	SetWidth(bounds.Width);
	SetHeight(bounds.Height);
}

const Rect2I& GUIElement::GetBounds() const
{
	if(mLayoutUpdateParent != nullptr && mLayoutUpdateParent->IsDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayout(mLayoutUpdateParent);

	return mLayoutData.Area;
}

Rect2I GUIElement::GetScreenBounds() const
{
	Rect2I area = GetBounds();
	if(mParentWidget)
	{
		const Matrix4& widgetTfrm = mParentWidget->GetWorldTfrm();
		Vector2I localPos(area.X, area.Y);

		const Vector4 widgetPosFlt = widgetTfrm.MultiplyAffine(Vector4((float)localPos.X, (float)localPos.Y, 0.0f, 1.0f));
		const Vector2I widgetPos(Math::RoundToI32(widgetPosFlt.X), Math::RoundToI32(widgetPosFlt.Y));

		const RenderWindow* parentWindow = GUIManager::Instance().GetWidgetWindow(*mParentWidget);
		if(parentWindow)
		{
			const Vector2I windowPos = parentWindow->WindowToScreenPosition(widgetPos);
			area.X = windowPos.X;
			area.Y = windowPos.Y;
		}
		else
		{
			area.X = widgetPos.X;
			area.Y = widgetPos.Y;
		}
	}

	return area;
}

void GUIElement::MarkAsClean()
{
	mFlags &= ~GUIElem_Dirty;
}

void GUIElement::MarkLayoutAsDirty()
{
	if(!IsVisible())
		return;

	if(mLayoutUpdateParent != nullptr)
		mLayoutUpdateParent->mFlags |= GUIElem_Dirty;
	else
		mFlags |= GUIElem_Dirty;
}

void GUIElement::MarkContentAsDirty()
{
	if(!IsVisible())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkContentDirty(this);
}

void GUIElement::MarkMeshAsDirty()
{
	if(!IsVisible())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkMeshDirty(this);
}

void GUIElement::SetVisible(bool visible)
{
	// No visibility states matter if object is not active
	if(!IsActive())
		return;

	bool visibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
	if(visibleSelf != visible)
	{
		// If making an element visible make sure to mark layout as dirty, as we didn't track any dirty flags while the element was inactive
		if(!visible)
		{
			mFlags |= GUIElem_HiddenSelf;
			SetVisibleRecursive(false);
		}
		else
		{
			mFlags &= ~GUIElem_HiddenSelf;

			if(mParent == nullptr || mParent->IsVisible())
				SetVisibleRecursive(true);
		}
	}
}

void GUIElement::SetVisibleRecursive(bool visible)
{
	bool isVisible = (mFlags & GUIElem_Hidden) == 0;
	if(isVisible == visible)
		return;

	if(!visible)
	{
		if(mParentWidget)
			mParentWidget->NotifyElementVisibilityChanged(this, false);

		mFlags |= GUIElem_Hidden;
		MarkMeshAsDirty();

		for(auto& child : mChildren)
			child->SetVisibleRecursive(false);
	}
	else
	{
		bool childVisibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
		if(childVisibleSelf)
		{
			mFlags &= ~GUIElem_Hidden;

			if(mParentWidget)
				mParentWidget->NotifyElementVisibilityChanged(this, true);

			MarkLayoutAsDirty();

			for(auto& child : mChildren)
				child->SetVisibleRecursive(true);
		}
	}
}

void GUIElement::SetActive(bool active)
{
	static const u8 kActiveFlags = GUIElem_InactiveSelf | GUIElem_HiddenSelf;

	bool activeSelf = (mFlags & GUIElem_InactiveSelf) == 0;
	if(activeSelf != active)
	{
		if(!active)
		{
			mFlags |= kActiveFlags;

			SetActiveRecursive(false);
			SetVisibleRecursive(false);
		}
		else
		{
			mFlags &= ~kActiveFlags;

			if(mParent != nullptr)
			{
				if(mParent->IsActive())
				{
					SetActiveRecursive(true);

					if(mParent->IsVisible())
						SetVisibleRecursive(true);
				}
			}
			else
			{
				SetActiveRecursive(true);
				SetVisibleRecursive(true);
			}
		}
	}
}

void GUIElement::SetActiveRecursive(bool active)
{
	bool isActive = (mFlags & GUIElem_Inactive) == 0;
	if(isActive == active)
		return;

	if(!active)
	{
		MarkLayoutAsDirty();

		mFlags |= GUIElem_Inactive;

		for(auto& child : mChildren)
			child->SetActiveRecursive(false);
	}
	else
	{
		bool childActiveSelf = (mFlags & GUIElem_InactiveSelf) == 0;
		if(childActiveSelf)
		{
			mFlags &= ~GUIElem_Inactive;
			MarkLayoutAsDirty();

			for(auto& child : mChildren)
				child->SetActiveRecursive(true);
		}
	}
}

void GUIElement::SetDisabled(bool disabled)
{
	bool disabledSelf = (mFlags & GUIElem_DisabledSelf) != 0;
	if(disabledSelf != disabled)
	{
		if(!disabled)
			mFlags &= ~GUIElem_DisabledSelf;
		else
			mFlags |= GUIElem_DisabledSelf;

		SetDisabledRecursive(disabled);
	}
}

void GUIElement::SetDisabledRecursive(bool disabled)
{
	bool isDisabled = (mFlags & GUIElem_Disabled) != 0;
	if(isDisabled == disabled)
		return;

	if(!disabled)
	{
		bool disabledSelf = (mFlags & GUIElem_DisabledSelf) != 0;
		if(!disabledSelf)
		{
			mFlags &= ~GUIElem_Disabled;

			for(auto& child : mChildren)
				child->SetDisabledRecursive(false);
		}
	}
	else
	{
		mFlags |= GUIElem_Disabled;

		for(auto& child : mChildren)
			child->SetDisabledRecursive(true);
	}

	if(IsVisible())
		MarkContentAsDirty();
}

void GUIElement::UpdateLayout(const GUILayoutData& data)
{
	UpdateOptimalLayoutSizes(); // We calculate optimal sizes of all layouts as a pre-processing step, as they are requested often during update
	UpdateLayoutRecursive(data);
}

void GUIElement::UpdateOptimalLayoutSizes()
{
	for(auto& child : mChildren)
	{
		child->UpdateOptimalLayoutSizes();
	}
}

void GUIElement::UpdateLayoutRecursive(const GUILayoutData& data)
{
	for(auto& child : mChildren)
	{
		child->UpdateLayoutRecursive(data);
	}
}

GUIConstrainedSize GUIElement::CalculateConstrainedSize() const
{
	const GUISizeConstraints& sizeConstraints = GetSizeConstraints();
	return sizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize());
}

GUIConstrainedSize GUIElement::GetConstrainedSize() const
{
	return CalculateConstrainedSize();
}

const RectOffset& GUIElement::GetMargins() const
{
	static RectOffset margins;
	return margins;
}

const RectOffset& GUIElement::GetPadding() const
{
	static RectOffset padding;
	return padding;
}

void GUIElement::GetChildLayoutAreas(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const
{
	B3D_ASSERT(mChildren.size() == 0);
}

void GUIElement::SetParent(GUIElement* parent)
{
	if(mParent != parent)
	{
		mParent = parent;
		UpdatePanelAndLayoutUpdateParents();

		if(parent != nullptr)
		{
			if(GetParentWidget() != parent->GetParentWidget())
				ChangeParentWidget(parent->GetParentWidget());
		}
		else
			ChangeParentWidget(nullptr);
	}
}

void GUIElement::RegisterChildElement(GUIElement* element)
{
	B3D_ASSERT(!element->IsPendingDestroy());

	GUIElement* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Add(element);

	element->SetActiveRecursive(IsActive());
	element->SetVisibleRecursive(IsVisible());
	element->SetDisabledRecursive(IsDisabled());

	// No need to mark ourselves as dirty. If we're part of the element's update chain, this will do it for us.
	element->MarkLayoutAsDirty();
}

void GUIElement::UnregisterChildElement(GUIElement* element)
{
	bool foundElem = false;
	for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
	{
		GUIElement* child = *iter;

		if(child == element)
		{
			element->MarkLayoutAsDirty();

			mChildren.erase(iter);
			element->SetParent(nullptr);
			foundElem = true;

			break;
		}
	}

	if(!foundElem)
		B3D_EXCEPT(InvalidParametersException, "Provided element is not a part of this element.");
}

void GUIElement::Destroy()
{
	if(mIsPendingDestroy)
		return;

	if(mParent != nullptr)
		mParent->UnregisterChildElement(this);

	DestroyChildElements();

	mIsPendingDestroy = true;

	GUIManager::Instance().QueueForDestroy(this);
}

void GUIElement::DestroyChildElements()
{
	TInlineArray<GUIElement*, 4> childCopy = mChildren;
	for(auto& child : childCopy)
		child->Destroy();

	B3D_ASSERT(mChildren.Empty());
}

void GUIElement::ChangeParentWidget(GUIWidget* widget)
{
	B3D_ASSERT(!IsPendingDestroy());

	if(mParentWidget != widget)
	{
		if(mParentWidget != nullptr)
			mParentWidget->UnregisterElement(this);

		if(widget != nullptr)
			widget->RegisterElement(this);
	}

	mParentWidget = widget;

	for(auto& child : mChildren)
	{
		child->ChangeParentWidget(widget);
	}

	MarkLayoutAsDirty();
}

void GUIElement::UpdatePanelAndLayoutUpdateParents()
{
	GUIElement* layoutUpdateParent = nullptr;
	if(mParent != nullptr)
	{
		layoutUpdateParent = mParent->FindLayoutUpdateParent();

		// If parent is a panel then we can do an optimization and only update
		// one child instead of all of them, so change parent to that child.
		if(layoutUpdateParent != nullptr && B3DRTTIIsOfType<GUIPanel>(layoutUpdateParent))
		{
			GUIElement* optimizedUpdateParent = this;
			while(optimizedUpdateParent->GetParent() != layoutUpdateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParent();

			layoutUpdateParent = optimizedUpdateParent;
		}
	}

	GUIPanel* panelParent = nullptr;
	GUIElement* currentParent = mParent;
	while(currentParent != nullptr)
	{
		if(B3DRTTIIsOfType<GUIPanel>(currentParent))
		{
			panelParent = static_cast<GUIPanel*>(currentParent);
			break;
		}

		currentParent = currentParent->mParent;
	}

	SetPanelParent(panelParent);
	SetLayoutUpdateParent(layoutUpdateParent);
}

GUIElement* GUIElement::FindLayoutUpdateParent()
{
	GUIElement* currentElement = this;
	while(currentElement != nullptr)
	{
		const GUISizeConstraints& parentDimensions = currentElement->GetSizeConstraints();
		bool boundsDependOnChildren = !parentDimensions.IsHeightFixed() || !parentDimensions.IsWidthFixed();

		if(!boundsDependOnChildren)
			return currentElement;

		currentElement = currentElement->mParent;
	}

	return nullptr;
}

void GUIElement::RefreshLayoutUpdateParentsForChildren()
{
	GUIElement* updateParent = FindLayoutUpdateParent();

	for(auto& child : mChildren)
	{
		GUIElement* childUpdateParent = updateParent;

		// If parent is a panel then we can do an optimization and only update
		// one child instead of all of them, so change parent to that child.
		if(childUpdateParent != nullptr && B3DRTTIIsOfType<GUIPanel>(childUpdateParent))
		{
			GUIElement* optimizedUpdateParent = child;
			while(optimizedUpdateParent->GetParent() != childUpdateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParent();

			childUpdateParent = optimizedUpdateParent;
		}

		child->SetLayoutUpdateParent(childUpdateParent);
	}
}

void GUIElement::SetPanelParent(GUIPanel* panelParent)
{
	mPanelParent = panelParent;

	if(B3DRTTIIsOfType<GUIPanel>(this))
		return;

	for(auto& child : mChildren)
		child->SetPanelParent(panelParent);
}

void GUIElement::SetLayoutUpdateParent(GUIElement* layoutUpdateParent)
{
	mLayoutUpdateParent = layoutUpdateParent;

	const GUISizeConstraints& dimensions = GetSizeConstraints();
	bool boundsDependOnChildren = !dimensions.IsHeightFixed() || !dimensions.IsWidthFixed();

	if(!boundsDependOnChildren)
		return;

	for(auto& child : mChildren)
		child->SetLayoutUpdateParent(layoutUpdateParent);
}

RTTIType* GUIElement::GetRttiStatic()
{
	return GUIElementRTTI::Instance();
}

RTTIType* GUIElement::GetRtti() const
{
	return GUIElement::GetRttiStatic();
}
