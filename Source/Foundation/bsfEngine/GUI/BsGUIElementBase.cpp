//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElementBase.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUISpace.h"
#include "GUI/BsGUIElement.h"
#include "Error/BsException.h"
#include "GUI/BsGUIWidget.h"
#include "BsGUIManager.h"

using namespace bs;

GUIElementBase::GUIElementBase(const GUIDimensions& dimensions)
	: mDimensions(dimensions)
{}

GUIElementBase::~GUIElementBase()
{
	DestroyChildElements();
}

void GUIElementBase::SetPosition(i32 x, i32 y)
{
	mDimensions.X = x;
	mDimensions.Y = y;

	// Note: I could call _markMeshAsDirty with a little more work. If parent is layout then this call can be ignored
	// and if it's a panel, we can immediately change the position without a full layout rebuild.
	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::SetSize(u32 width, u32 height)
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_FixedWidth | GUIDF_OverWidth | GUIDF_FixedHeight | GUIDF_OverHeight;
	mDimensions.MinWidth = mDimensions.MaxWidth = width;
	mDimensions.MinHeight = mDimensions.MaxHeight = height;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::SetWidth(u32 width)
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_FixedWidth | GUIDF_OverWidth;
	mDimensions.MinWidth = mDimensions.MaxWidth = width;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::SetFlexibleWidth(u32 minWidth, u32 maxWidth)
{
	if(maxWidth < minWidth)
		std::swap(minWidth, maxWidth);

	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_OverWidth;
	mDimensions.Flags &= ~GUIDF_FixedWidth;
	mDimensions.MinWidth = minWidth;
	mDimensions.MaxWidth = maxWidth;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::SetHeight(u32 height)
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_FixedHeight | GUIDF_OverHeight;
	mDimensions.MinHeight = mDimensions.MaxHeight = height;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::SetFlexibleHeight(u32 minHeight, u32 maxHeight)
{
	if(maxHeight < minHeight)
		std::swap(minHeight, maxHeight);

	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_OverHeight;
	mDimensions.Flags &= ~GUIDF_FixedHeight;
	mDimensions.MinHeight = minHeight;
	mDimensions.MaxHeight = maxHeight;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::ResetDimensions()
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions = GUIDimensions::Create();

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirtyInternal();
}

Rect2I GUIElementBase::GetBounds(GUIPanel* relativeTo)
{
	if(relativeTo == nullptr)
		relativeTo = mAnchorParent;

	Rect2I anchorBounds;
	if(relativeTo != nullptr)
		anchorBounds = relativeTo->GetGlobalBounds();

	if(mUpdateParent != nullptr && mUpdateParent->IsDirtyInternal() && mParentWidget != nullptr)
		mParentWidget->UpdateLayoutInternal(mUpdateParent);

	Rect2I bounds = mLayoutData.Area;
	bounds.X -= anchorBounds.X;
	bounds.Y -= anchorBounds.Y;

	return bounds;
}

void GUIElementBase::SetBounds(const Rect2I& bounds)
{
	SetPosition(bounds.X, bounds.Y);
	SetWidth(bounds.Width);
	SetHeight(bounds.Height);
}

Rect2I GUIElementBase::GetGlobalBounds()
{
	if(mUpdateParent != nullptr && mUpdateParent->IsDirtyInternal() && mParentWidget != nullptr)
		mParentWidget->UpdateLayoutInternal(mUpdateParent);

	return mLayoutData.Area;
}

Rect2I GUIElementBase::GetScreenBounds() const
{
	if(mUpdateParent != nullptr && mUpdateParent->IsDirtyInternal() && mParentWidget != nullptr)
		mParentWidget->UpdateLayoutInternal(mUpdateParent);

	Rect2I area = mLayoutData.Area;
	if(mParentWidget)
	{
		const Matrix4& widgetTfrm = mParentWidget->GetWorldTfrm();
		Vector2I localPos(area.X, area.Y);

		const Vector4 widgetPosFlt = widgetTfrm.MultiplyAffine(Vector4((float)localPos.X, (float)localPos.Y, 0.0f, 1.0f));
		const Vector2I widgetPos(Math::RoundToI32(widgetPosFlt.X), Math::RoundToI32(widgetPosFlt.Y));

		const RenderWindow* parentWindow = GUIManager::Instance().GetWidgetWindow(*mParentWidget);
		if(parentWindow)
		{
			const Vector2I windowPos = parentWindow->WindowToScreenPos(widgetPos);
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

Rect2I GUIElementBase::GetVisibleBounds()
{
	return GetBounds();
}

void GUIElementBase::MarkAsCleanInternal()
{
	mFlags &= ~GUIElem_Dirty;
}

void GUIElementBase::MarkLayoutAsDirtyInternal()
{
	if(!IsVisibleInternal())
		return;

	if(mUpdateParent != nullptr)
		mUpdateParent->mFlags |= GUIElem_Dirty;
	else
		mFlags |= GUIElem_Dirty;
}

void GUIElementBase::MarkContentAsDirtyInternal()
{
	if(!IsVisibleInternal())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkContentDirtyInternal(this);
}

void GUIElementBase::MarkMeshAsDirtyInternal()
{
	if(!IsVisibleInternal())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkMeshDirtyInternal(this);
}

void GUIElementBase::SetVisible(bool visible)
{
	// No visibility states matter if object is not active
	if(!IsActiveInternal())
		return;

	bool visibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
	if(visibleSelf != visible)
	{
		// If making an element visible make sure to mark layout as dirty, as we didn't track any dirty flags while the element was inactive
		if(!visible)
		{
			mFlags |= GUIElem_HiddenSelf;
			SetVisibleInternal(false);
		}
		else
		{
			mFlags &= ~GUIElem_HiddenSelf;

			if(mParentElement == nullptr || mParentElement->IsVisibleInternal())
				SetVisibleInternal(true);
		}
	}
}

void GUIElementBase::SetVisibleInternal(bool visible)
{
	bool isVisible = (mFlags & GUIElem_Hidden) == 0;
	if(isVisible == visible)
		return;

	if(!visible)
	{
		MarkMeshAsDirtyInternal();

		mFlags |= GUIElem_Hidden;

		for(auto& child : mChildren)
			child->SetVisibleInternal(false);
	}
	else
	{
		bool childVisibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
		if(childVisibleSelf)
		{
			mFlags &= ~GUIElem_Hidden;
			MarkLayoutAsDirtyInternal();

			for(auto& child : mChildren)
				child->SetVisibleInternal(true);
		}
	}
}

void GUIElementBase::SetActive(bool active)
{
	static const u8 kActiveFlags = GUIElem_InactiveSelf | GUIElem_HiddenSelf;

	bool activeSelf = (mFlags & GUIElem_InactiveSelf) == 0;
	if(activeSelf != active)
	{
		if(!active)
		{
			mFlags |= kActiveFlags;

			SetActiveInternal(false);
			SetVisibleInternal(false);
		}
		else
		{
			mFlags &= ~kActiveFlags;

			if(mParentElement != nullptr)
			{
				if(mParentElement->IsActiveInternal())
				{
					SetActiveInternal(true);

					if(mParentElement->IsVisibleInternal())
						SetVisibleInternal(true);
				}
			}
			else
			{
				SetActiveInternal(true);
				SetVisibleInternal(true);
			}
		}
	}
}

void GUIElementBase::SetActiveInternal(bool active)
{
	bool isActive = (mFlags & GUIElem_Inactive) == 0;
	if(isActive == active)
		return;

	if(!active)
	{
		MarkLayoutAsDirtyInternal();

		mFlags |= GUIElem_Inactive;

		for(auto& child : mChildren)
			child->SetActiveInternal(false);
	}
	else
	{
		bool childActiveSelf = (mFlags & GUIElem_InactiveSelf) == 0;
		if(childActiveSelf)
		{
			mFlags &= ~GUIElem_Inactive;
			MarkLayoutAsDirtyInternal();

			for(auto& child : mChildren)
				child->SetActiveInternal(true);
		}
	}
}

void GUIElementBase::SetDisabled(bool disabled)
{
	bool disabledSelf = (mFlags & GUIElem_DisabledSelf) != 0;
	if(disabledSelf != disabled)
	{
		if(!disabled)
			mFlags &= ~GUIElem_DisabledSelf;
		else
			mFlags |= GUIElem_DisabledSelf;

		SetDisabledInternal(disabled);
	}
}

void GUIElementBase::SetDisabledInternal(bool disabled)
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
				child->SetDisabledInternal(false);
		}
	}
	else
	{
		mFlags |= GUIElem_Disabled;

		for(auto& child : mChildren)
			child->SetDisabledInternal(true);
	}

	if(IsVisibleInternal())
		MarkContentAsDirtyInternal();
}

void GUIElementBase::UpdateLayoutInternal(const GUILayoutData& data)
{
	UpdateOptimalLayoutSizesInternal(); // We calculate optimal sizes of all layouts as a pre-processing step, as they are requested often during update
	UpdateLayoutInternalInternal(data);
}

void GUIElementBase::UpdateOptimalLayoutSizesInternal()
{
	for(auto& child : mChildren)
	{
		child->UpdateOptimalLayoutSizesInternal();
	}
}

void GUIElementBase::UpdateLayoutInternalInternal(const GUILayoutData& data)
{
	for(auto& child : mChildren)
	{
		child->UpdateLayoutInternalInternal(data);
	}
}

LayoutSizeRange GUIElementBase::CalculateLayoutSizeRangeInternal() const
{
	const GUIDimensions& dimensions = GetDimensionsInternal();
	return dimensions.CalculateSizeRange(GetOptimalSizeInternal());
}

LayoutSizeRange GUIElementBase::GetLayoutSizeRangeInternal() const
{
	return CalculateLayoutSizeRangeInternal();
}

void GUIElementBase::GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
{
	B3D_ASSERT(mChildren.size() == 0);
}

void GUIElementBase::SetParentInternal(GUIElementBase* parent)
{
	if(mParentElement != parent)
	{
		mParentElement = parent;
		UpdateAUParentsInternal();

		if(parent != nullptr)
		{
			if(GetParentWidgetInternal() != parent->GetParentWidgetInternal())
				ChangeParentWidgetInternal(parent->GetParentWidgetInternal());
		}
		else
			ChangeParentWidgetInternal(nullptr);
	}
}

void GUIElementBase::RegisterChildElementInternal(GUIElementBase* element)
{
	B3D_ASSERT(!element->IsDestroyedInternal());

	GUIElementBase* parentElement = element->GetParentInternal();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElementInternal(element);
	}

	element->SetParentInternal(this);
	mChildren.push_back(element);

	element->SetActiveInternal(IsActiveInternal());
	element->SetVisibleInternal(IsVisibleInternal());
	element->SetDisabledInternal(IsDisabledInternal());

	// No need to mark ourselves as dirty. If we're part of the element's update chain, this will do it for us.
	element->MarkLayoutAsDirtyInternal();
}

void GUIElementBase::UnregisterChildElementInternal(GUIElementBase* element)
{
	bool foundElem = false;
	for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
	{
		GUIElementBase* child = *iter;

		if(child == element)
		{
			element->MarkLayoutAsDirtyInternal();

			mChildren.erase(iter);
			element->SetParentInternal(nullptr);
			foundElem = true;

			break;
		}
	}

	if(!foundElem)
		B3D_EXCEPT(InvalidParametersException, "Provided element is not a part of this element.");
}

void GUIElementBase::DestroyChildElements()
{
	Vector<GUIElementBase*> childCopy = mChildren;
	for(auto& child : childCopy)
	{
		if(child->GetTypeInternal() == Type::Element)
		{
			const auto element = static_cast<GUIElement*>(child);
			GUIElement::Destroy(element);
		}
		else if(child->GetTypeInternal() == Type::Layout || child->GetTypeInternal() == GUIElementBase::Type::Panel)
		{
			const auto layout = static_cast<GUILayout*>(child);
			GUILayout::Destroy(layout);
		}
		else if(child->GetTypeInternal() == Type::FixedSpace)
		{
			const auto space = static_cast<GUIFixedSpace*>(child);
			GUIFixedSpace::Destroy(space);
		}
		else if(child->GetTypeInternal() == Type::FlexibleSpace)
		{
			const auto space = static_cast<GUIFlexibleSpace*>(child);
			GUIFlexibleSpace::Destroy(space);
		}
	}

	B3D_ASSERT(mChildren.empty());
}

void GUIElementBase::ChangeParentWidgetInternal(GUIWidget* widget)
{
	B3D_ASSERT(!IsDestroyedInternal());

	if(mParentWidget != widget)
	{
		if(mParentWidget != nullptr)
			mParentWidget->UnregisterElementInternal(this);

		if(widget != nullptr)
			widget->RegisterElementInternal(this);
	}

	mParentWidget = widget;

	for(auto& child : mChildren)
	{
		child->ChangeParentWidgetInternal(widget);
	}

	MarkLayoutAsDirtyInternal();
}

void GUIElementBase::UpdateAUParentsInternal()
{
	GUIElementBase* updateParent = nullptr;
	if(mParentElement != nullptr)
	{
		updateParent = mParentElement->FindUpdateParent();

		// If parent is a panel then we can do an optimization and only update
		// one child instead of all of them, so change parent to that child.
		if(updateParent != nullptr && updateParent->GetTypeInternal() == GUIElementBase::Type::Panel)
		{
			GUIElementBase* optimizedUpdateParent = this;
			while(optimizedUpdateParent->GetParentInternal() != updateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParentInternal();

			updateParent = optimizedUpdateParent;
		}
	}

	GUIPanel* anchorParent = nullptr;
	GUIElementBase* currentParent = mParentElement;
	while(currentParent != nullptr)
	{
		if(currentParent->GetTypeInternal() == Type::Panel)
		{
			anchorParent = static_cast<GUIPanel*>(currentParent);
			break;
		}

		currentParent = currentParent->mParentElement;
	}

	SetAnchorParent(anchorParent);
	SetUpdateParent(updateParent);
}

GUIElementBase* GUIElementBase::FindUpdateParent()
{
	GUIElementBase* currentElement = this;
	while(currentElement != nullptr)
	{
		const GUIDimensions& parentDimensions = currentElement->GetDimensionsInternal();
		bool boundsDependOnChildren = !parentDimensions.FixedHeight() || !parentDimensions.FixedWidth();

		if(!boundsDependOnChildren)
			return currentElement;

		currentElement = currentElement->mParentElement;
	}

	return nullptr;
}

void GUIElementBase::RefreshChildUpdateParents()
{
	GUIElementBase* updateParent = FindUpdateParent();

	for(auto& child : mChildren)
	{
		GUIElementBase* childUpdateParent = updateParent;

		// If parent is a panel then we can do an optimization and only update
		// one child instead of all of them, so change parent to that child.
		if(childUpdateParent != nullptr && childUpdateParent->GetTypeInternal() == GUIElementBase::Type::Panel)
		{
			GUIElementBase* optimizedUpdateParent = child;
			while(optimizedUpdateParent->GetParentInternal() != childUpdateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParentInternal();

			childUpdateParent = optimizedUpdateParent;
		}

		child->SetUpdateParent(childUpdateParent);
	}
}

void GUIElementBase::SetAnchorParent(GUIPanel* anchorParent)
{
	mAnchorParent = anchorParent;

	if(GetTypeInternal() == Type::Panel)
		return;

	for(auto& child : mChildren)
		child->SetAnchorParent(anchorParent);
}

void GUIElementBase::SetUpdateParent(GUIElementBase* updateParent)
{
	mUpdateParent = updateParent;

	const GUIDimensions& dimensions = GetDimensionsInternal();
	bool boundsDependOnChildren = !dimensions.FixedHeight() || !dimensions.FixedWidth();

	if(!boundsDependOnChildren)
		return;

	for(auto& child : mChildren)
		child->SetUpdateParent(updateParent);
}
