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
	MarkLayoutAsDirty();
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

	MarkLayoutAsDirty();
}

void GUIElementBase::SetWidth(u32 width)
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_FixedWidth | GUIDF_OverWidth;
	mDimensions.MinWidth = mDimensions.MaxWidth = width;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirty();
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

	MarkLayoutAsDirty();
}

void GUIElementBase::SetHeight(u32 height)
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions.Flags |= GUIDF_FixedHeight | GUIDF_OverHeight;
	mDimensions.MinHeight = mDimensions.MaxHeight = height;

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirty();
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

	MarkLayoutAsDirty();
}

void GUIElementBase::ResetDimensions()
{
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions = GUIDimensions::Create();

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshChildUpdateParents();

	MarkLayoutAsDirty();
}

Rect2I GUIElementBase::GetBounds(GUIPanel* relativeTo)
{
	if(relativeTo == nullptr)
		relativeTo = mAnchorParent;

	Rect2I anchorBounds;
	if(relativeTo != nullptr)
		anchorBounds = relativeTo->GetGlobalBounds();

	if(mUpdateParent != nullptr && mUpdateParent->IsDirty() && mParentWidget != nullptr)
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
	if(mUpdateParent != nullptr && mUpdateParent->IsDirty() && mParentWidget != nullptr)
		mParentWidget->UpdateLayoutInternal(mUpdateParent);

	return mLayoutData.Area;
}

Rect2I GUIElementBase::GetScreenBounds() const
{
	if(mUpdateParent != nullptr && mUpdateParent->IsDirty() && mParentWidget != nullptr)
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

void GUIElementBase::MarkAsClean()
{
	mFlags &= ~GUIElem_Dirty;
}

void GUIElementBase::MarkLayoutAsDirty()
{
	if(!IsVisible())
		return;

	if(mUpdateParent != nullptr)
		mUpdateParent->mFlags |= GUIElem_Dirty;
	else
		mFlags |= GUIElem_Dirty;
}

void GUIElementBase::MarkContentAsDirty()
{
	if(!IsVisible())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkContentDirtyInternal(this);
}

void GUIElementBase::MarkMeshAsDirty()
{
	if(!IsVisible())
		return;

	if(mParentWidget != nullptr)
		mParentWidget->MarkMeshDirtyInternal(this);
}

void GUIElementBase::SetVisible(bool visible)
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

			if(mParentElement == nullptr || mParentElement->IsVisible())
				SetVisibleRecursive(true);
		}
	}
}

void GUIElementBase::SetVisibleRecursive(bool visible)
{
	bool isVisible = (mFlags & GUIElem_Hidden) == 0;
	if(isVisible == visible)
		return;

	if(!visible)
	{
		MarkMeshAsDirty();

		//if(mParentWidget)
		//	mParentWidget->NotifyVisibilityChanged(this, false);

		mFlags |= GUIElem_Hidden;

		for(auto& child : mChildren)
			child->SetVisibleRecursive(false);
	}
	else
	{
		bool childVisibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
		if(childVisibleSelf)
		{
			mFlags &= ~GUIElem_Hidden;

			//if(mParentWidget)
			//	mParentWidget->NotifyVisibilityChanged(this, true);

			MarkLayoutAsDirty();

			for(auto& child : mChildren)
				child->SetVisibleRecursive(true);
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

			SetActiveRecursive(false);
			SetVisibleRecursive(false);
		}
		else
		{
			mFlags &= ~kActiveFlags;

			if(mParentElement != nullptr)
			{
				if(mParentElement->IsActive())
				{
					SetActiveRecursive(true);

					if(mParentElement->IsVisible())
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

void GUIElementBase::SetActiveRecursive(bool active)
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

void GUIElementBase::SetDisabled(bool disabled)
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

void GUIElementBase::SetDisabledRecursive(bool disabled)
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

void GUIElementBase::UpdateLayout(const GUILayoutData& data)
{
	UpdateOptimalLayoutSizes(); // We calculate optimal sizes of all layouts as a pre-processing step, as they are requested often during update
	UpdateLayoutRecursive(data);
}

void GUIElementBase::UpdateOptimalLayoutSizes()
{
	for(auto& child : mChildren)
	{
		child->UpdateOptimalLayoutSizes();
	}
}

void GUIElementBase::UpdateLayoutRecursive(const GUILayoutData& data)
{
	for(auto& child : mChildren)
	{
		child->UpdateLayoutRecursive(data);
	}
}

LayoutSizeRange GUIElementBase::CalculateLayoutSizeRange() const
{
	const GUIDimensions& dimensions = GetDimensions();
	return dimensions.CalculateSizeRange(GetOptimalSize());
}

LayoutSizeRange GUIElementBase::GetLayoutSizeRange() const
{
	return CalculateLayoutSizeRange();
}

void GUIElementBase::GetElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
{
	B3D_ASSERT(mChildren.size() == 0);
}

void GUIElementBase::SetParent(GUIElementBase* parent)
{
	if(mParentElement != parent)
	{
		mParentElement = parent;
		UpdateAUParentsInternal();

		if(parent != nullptr)
		{
			if(GetParentWidget() != parent->GetParentWidget())
				ChangeParentWidget(parent->GetParentWidget());
		}
		else
			ChangeParentWidget(nullptr);
	}
}

void GUIElementBase::RegisterChildElement(GUIElementBase* element)
{
	B3D_ASSERT(!element->IsDestroyed());

	GUIElementBase* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.push_back(element);

	element->SetActiveRecursive(IsActive());
	element->SetVisibleRecursive(IsVisible());
	element->SetDisabledRecursive(IsDisabled());

	// No need to mark ourselves as dirty. If we're part of the element's update chain, this will do it for us.
	element->MarkLayoutAsDirty();
}

void GUIElementBase::UnregisterChildElement(GUIElementBase* element)
{
	bool foundElem = false;
	for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
	{
		GUIElementBase* child = *iter;

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

void GUIElementBase::ChangeParentWidget(GUIWidget* widget)
{
	B3D_ASSERT(!IsDestroyed());

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
		child->ChangeParentWidget(widget);
	}

	MarkLayoutAsDirty();
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
			while(optimizedUpdateParent->GetParent() != updateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParent();

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
		const GUIDimensions& parentDimensions = currentElement->GetDimensions();
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
			while(optimizedUpdateParent->GetParent() != childUpdateParent)
				optimizedUpdateParent = optimizedUpdateParent->GetParent();

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

	const GUIDimensions& dimensions = GetDimensions();
	bool boundsDependOnChildren = !dimensions.FixedHeight() || !dimensions.FixedWidth();

	if(!boundsDependOnChildren)
		return;

	for(auto& child : mChildren)
		child->SetUpdateParent(updateParent);
}
