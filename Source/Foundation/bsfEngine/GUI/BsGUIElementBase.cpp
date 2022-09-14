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

namespace bs
{
	GUIElementBase::GUIElementBase(const GUIDimensions& dimensions)
		: mDimensions(dimensions)
	{ }

	GUIElementBase::~GUIElementBase()
	{
		destroyChildElements();
	}

	void GUIElementBase::setPosition(INT32 x, INT32 y)
	{
		mDimensions.x = x;
		mDimensions.y = y;

		// Note: I could call _markMeshAsDirty with a little more work. If parent is layout then this call can be ignored
		// and if it's a panel, we can immediately change the position without a full layout rebuild.
		MarkLayoutAsDirtyInternal();
	}

	void GUIElementBase::setSize(UINT32 width, UINT32 height)
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_FixedWidth | GUIDF_OverWidth | GUIDF_FixedHeight | GUIDF_OverHeight;
		mDimensions.minWidth = mDimensions.maxWidth = width;
		mDimensions.minHeight = mDimensions.maxHeight = height;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		MarkLayoutAsDirtyInternal();
	}

	void GUIElementBase::setWidth(UINT32 width)
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_FixedWidth | GUIDF_OverWidth;
		mDimensions.minWidth = mDimensions.maxWidth = width;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();
			
		MarkLayoutAsDirtyInternal();
	}

	void GUIElementBase::setFlexibleWidth(UINT32 minWidth, UINT32 maxWidth)
	{
		if (maxWidth < minWidth)
			std::swap(minWidth, maxWidth);

		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_OverWidth;
		mDimensions.flags &= ~GUIDF_FixedWidth;
		mDimensions.minWidth = minWidth;
		mDimensions.maxWidth = maxWidth;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		MarkLayoutAsDirtyInternal();
	}

	void GUIElementBase::setHeight(UINT32 height)
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_FixedHeight | GUIDF_OverHeight;
		mDimensions.minHeight = mDimensions.maxHeight = height;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		MarkLayoutAsDirtyInternal();
	}

	void GUIElementBase::setFlexibleHeight(UINT32 minHeight, UINT32 maxHeight)
	{
		if (maxHeight < minHeight)
			std::swap(minHeight, maxHeight);

		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_OverHeight;
		mDimensions.flags &= ~GUIDF_FixedHeight;
		mDimensions.minHeight = minHeight;
		mDimensions.maxHeight = maxHeight;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		MarkLayoutAsDirtyInternal();
	}

	void GUIElementBase::resetDimensions()
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions = GUIDimensions::create();

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		MarkLayoutAsDirtyInternal();
	}

	Rect2I GUIElementBase::getBounds(GUIPanel* relativeTo)
	{
		if (relativeTo == nullptr)
			relativeTo = mAnchorParent;

		Rect2I anchorBounds;
		if (relativeTo != nullptr)
			anchorBounds = relativeTo->getGlobalBounds();

		if (mUpdateParent != nullptr && mUpdateParent->IsDirtyInternal() && mParentWidget != nullptr)
			mParentWidget->UpdateLayoutInternal(mUpdateParent);

		Rect2I bounds = mLayoutData.area;
		bounds.x -= anchorBounds.x;
		bounds.y -= anchorBounds.y;
		
		return bounds;
	}

	void GUIElementBase::setBounds(const Rect2I& bounds)
	{
		setPosition(bounds.x, bounds.y);
		setWidth(bounds.width);
		setHeight(bounds.height);
	}

	Rect2I GUIElementBase::getGlobalBounds()
	{
		if (mUpdateParent != nullptr && mUpdateParent->IsDirtyInternal() && mParentWidget != nullptr)
			mParentWidget->UpdateLayoutInternal(mUpdateParent);

		return mLayoutData.area;
	}

	Rect2I GUIElementBase::getScreenBounds() const
	{
		if (mUpdateParent != nullptr && mUpdateParent->IsDirtyInternal() && mParentWidget != nullptr)
			mParentWidget->UpdateLayoutInternal(mUpdateParent);

		Rect2I area = mLayoutData.area;
		if(mParentWidget)
		{
			const Matrix4& widgetTfrm = mParentWidget->getWorldTfrm();
			Vector2I localPos(area.x, area.y);

			const Vector4 widgetPosFlt = widgetTfrm.multiplyAffine(Vector4((float)localPos.x, (float)localPos.y, 0.0f, 1.0f));
			const Vector2I widgetPos(Math::roundToInt(widgetPosFlt.x), Math::roundToInt(widgetPosFlt.y));

			const RenderWindow* parentWindow = GUIManager::instance().getWidgetWindow(*mParentWidget);
			if(parentWindow)
			{
				const Vector2I windowPos = parentWindow->windowToScreenPos(widgetPos);
				area.x = windowPos.x;
				area.y = windowPos.y;
			}
			else
			{
				area.x = widgetPos.x;
				area.y = widgetPos.y;
			}
		}

		return area;
	}

	Rect2I GUIElementBase::getVisibleBounds()
	{
		return getBounds();
	}
	
	void GUIElementBase::MarkAsCleanInternal()
	{
		mFlags &= ~GUIElem_Dirty;
	}

	void GUIElementBase::MarkLayoutAsDirtyInternal()
	{
		if(!IsVisibleInternal())
			return;

		if (mUpdateParent != nullptr)
			mUpdateParent->mFlags |= GUIElem_Dirty;
		else
			mFlags |= GUIElem_Dirty;
	}

	void GUIElementBase::MarkContentAsDirtyInternal()
	{
		if (!IsVisibleInternal())
			return;

		if (mParentWidget != nullptr)
			mParentWidget->MarkContentDirtyInternal(this);
	}

	void GUIElementBase::MarkMeshAsDirtyInternal()
	{
		if(!IsVisibleInternal())
			return;

		if (mParentWidget != nullptr)
			mParentWidget->MarkMeshDirtyInternal(this);
	}

	void GUIElementBase::setVisible(bool visible)
	{
		// No visibility states matter if object is not active
		if (!IsActiveInternal())
			return;

		bool visibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
		if (visibleSelf != visible)
		{
			// If making an element visible make sure to mark layout as dirty, as we didn't track any dirty flags while the element was inactive
			if (!visible)
			{
				mFlags |= GUIElem_HiddenSelf;
				SetVisibleInternal(false);
			}
			else
			{
				mFlags &= ~GUIElem_HiddenSelf;

				if (mParentElement == nullptr || mParentElement->IsVisibleInternal())
					SetVisibleInternal(true);
			}
		}
	}

	void GUIElementBase::SetVisibleInternal(bool visible)
	{
		bool isVisible = (mFlags & GUIElem_Hidden) == 0;
		if (isVisible == visible)
			return;

		if (!visible)
		{
			MarkMeshAsDirtyInternal();

			mFlags |= GUIElem_Hidden;

			for (auto& child : mChildren)
				child->SetVisibleInternal(false);
		}
		else
		{
			bool childVisibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
			if (childVisibleSelf)
			{
				mFlags &= ~GUIElem_Hidden;
				MarkLayoutAsDirtyInternal();

				for (auto& child : mChildren)
					child->SetVisibleInternal(true);
			}
		}
	}

	void GUIElementBase::setActive(bool active)
	{
		static const UINT8 ACTIVE_FLAGS = GUIElem_InactiveSelf | GUIElem_HiddenSelf;

		bool activeSelf = (mFlags & GUIElem_InactiveSelf) == 0;
		if (activeSelf != active)
		{
			if (!active)
			{
				mFlags |= ACTIVE_FLAGS;

				SetActiveInternal(false);
				SetVisibleInternal(false);
			}
			else
			{
				mFlags &= ~ACTIVE_FLAGS;

				if (mParentElement != nullptr)
				{
					if (mParentElement->IsActiveInternal())
					{
						SetActiveInternal(true);

						if (mParentElement->IsVisibleInternal())
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
		if (isActive == active)
			return;
		
		if (!active)
		{
			MarkLayoutAsDirtyInternal();

			mFlags |= GUIElem_Inactive;

			for (auto& child : mChildren)
				child->SetActiveInternal(false);
		}
		else
		{
			bool childActiveSelf = (mFlags & GUIElem_InactiveSelf) == 0;
			if (childActiveSelf)
			{
				mFlags &= ~GUIElem_Inactive;
				MarkLayoutAsDirtyInternal();

				for (auto& child : mChildren)
					child->SetActiveInternal(true);
			}
		}
	}

	void GUIElementBase::setDisabled(bool disabled)
	{
		bool disabledSelf = (mFlags & GUIElem_DisabledSelf) != 0;
		if (disabledSelf != disabled)
		{
			if (!disabled)
				mFlags &= ~GUIElem_DisabledSelf;
			else
				mFlags |= GUIElem_DisabledSelf;

			SetDisabledInternal(disabled);
		}
	}

	void GUIElementBase::SetDisabledInternal(bool disabled)
	{
		bool isDisabled = (mFlags & GUIElem_Disabled) != 0;
		if (isDisabled == disabled)
			return;

		if (!disabled)
		{
			bool disabledSelf = (mFlags & GUIElem_DisabledSelf) != 0;
			if (!disabledSelf)
			{
				mFlags &= ~GUIElem_Disabled;

				for (auto& child : mChildren)
					child->SetDisabledInternal(false);
			}
		}
		else
		{
			mFlags |= GUIElem_Disabled;

			for (auto& child : mChildren)
				child->SetDisabledInternal(true);
		}

		if (IsVisibleInternal())
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
		return dimensions.calculateSizeRange(GetOptimalSizeInternal());
	}

	LayoutSizeRange GUIElementBase::GetLayoutSizeRangeInternal() const
	{
		return CalculateLayoutSizeRangeInternal();
	}

	void GUIElementBase::GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == 0);
	}

	void GUIElementBase::SetParentInternal(GUIElementBase* parent)
	{
		if(mParentElement != parent)
		{
			mParentElement = parent;
			UpdateAUParentsInternal();

			if (parent != nullptr)
			{
				if (GetParentWidgetInternal() != parent->GetParentWidgetInternal())
					ChangeParentWidgetInternal(parent->GetParentWidgetInternal());
			}
			else
				ChangeParentWidgetInternal(nullptr);
		}
	}

	void GUIElementBase::RegisterChildElementInternal(GUIElementBase* element)
	{
		assert(!element->IsDestroyedInternal());

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

			if (child == element)
			{
				element->MarkLayoutAsDirtyInternal();

				mChildren.erase(iter);
				element->SetParentInternal(nullptr);
				foundElem = true;

				break;
			}
		}

		if(!foundElem)
			BS_EXCEPT(InvalidParametersException, "Provided element is not a part of this element.");
	}

	void GUIElementBase::destroyChildElements()
	{
		Vector<GUIElementBase*> childCopy = mChildren;
		for (auto& child : childCopy)
		{
			if (child->GetTypeInternal() == Type::Element)
			{
				const auto element = static_cast<GUIElement*>(child);
				GUIElement::destroy(element);
			}
			else if (child->GetTypeInternal() == Type::Layout || child->GetTypeInternal() == GUIElementBase::Type::Panel)
			{
				const auto layout = static_cast<GUILayout*>(child);
				GUILayout::destroy(layout);
			}
			else if (child->GetTypeInternal() == Type::FixedSpace)
			{
				const auto space = static_cast<GUIFixedSpace*>(child);
				GUIFixedSpace::destroy(space);
			}
			else if (child->GetTypeInternal() == Type::FlexibleSpace)
			{
				const auto space = static_cast<GUIFlexibleSpace*>(child);
				GUIFlexibleSpace::destroy(space);
			}
		}

		assert(mChildren.empty());
	}

	void GUIElementBase::ChangeParentWidgetInternal(GUIWidget* widget)
	{
		assert(!IsDestroyedInternal());

		if (mParentWidget != widget)
		{
			if (mParentWidget != nullptr)
				mParentWidget->UnregisterElementInternal(this);

			if (widget != nullptr)
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
		if (mParentElement != nullptr)
		{
			updateParent = mParentElement->findUpdateParent();

			// If parent is a panel then we can do an optimization and only update
			// one child instead of all of them, so change parent to that child.
			if (updateParent != nullptr && updateParent->GetTypeInternal() == GUIElementBase::Type::Panel)
			{
				GUIElementBase* optimizedUpdateParent = this;
				while (optimizedUpdateParent->GetParentInternal() != updateParent)
					optimizedUpdateParent = optimizedUpdateParent->GetParentInternal();

				updateParent = optimizedUpdateParent;
			}
		}

		GUIPanel* anchorParent = nullptr;
		GUIElementBase* currentParent = mParentElement;
		while (currentParent != nullptr)
		{
			if (currentParent->GetTypeInternal() == Type::Panel)
			{
				anchorParent = static_cast<GUIPanel*>(currentParent);
				break;
			}

			currentParent = currentParent->mParentElement;
		}

		setAnchorParent(anchorParent);
		setUpdateParent(updateParent);
	}

	GUIElementBase* GUIElementBase::findUpdateParent()
	{
		GUIElementBase* currentElement = this;
		while (currentElement != nullptr)
		{
			const GUIDimensions& parentDimensions = currentElement->GetDimensionsInternal();
			bool boundsDependOnChildren = !parentDimensions.fixedHeight() || !parentDimensions.fixedWidth();

			if (!boundsDependOnChildren)
				return currentElement;

			currentElement = currentElement->mParentElement;
		}

		return nullptr;
	}

	void GUIElementBase::refreshChildUpdateParents()
	{
		GUIElementBase* updateParent = findUpdateParent();

		for (auto& child : mChildren)
		{
			GUIElementBase* childUpdateParent = updateParent;

			// If parent is a panel then we can do an optimization and only update
			// one child instead of all of them, so change parent to that child.
			if (childUpdateParent != nullptr && childUpdateParent->GetTypeInternal() == GUIElementBase::Type::Panel)
			{
				GUIElementBase* optimizedUpdateParent = child;
				while (optimizedUpdateParent->GetParentInternal() != childUpdateParent)
					optimizedUpdateParent = optimizedUpdateParent->GetParentInternal();

				childUpdateParent = optimizedUpdateParent;
			}

			child->setUpdateParent(childUpdateParent);
		}
	}

	void GUIElementBase::setAnchorParent(GUIPanel* anchorParent)
	{
		mAnchorParent = anchorParent;

		if (GetTypeInternal() == Type::Panel)
			return;

		for (auto& child : mChildren)
			child->setAnchorParent(anchorParent);
	}

	void GUIElementBase::setUpdateParent(GUIElementBase* updateParent)
	{
		mUpdateParent = updateParent;

		const GUIDimensions& dimensions = GetDimensionsInternal();
		bool boundsDependOnChildren = !dimensions.fixedHeight() || !dimensions.fixedWidth();

		if (!boundsDependOnChildren)
			return;

		for (auto& child : mChildren)
			child->setUpdateParent(updateParent);
	}
}
