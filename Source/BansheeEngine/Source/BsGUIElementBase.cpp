//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIElementBase.h"
#include "BsGUILayout.h"
#include "BsGUIPanel.h"
#include "BsGUISpace.h"
#include "BsGUIElement.h"
#include "BsException.h"
#include "BsGUIWidget.h"

namespace bs
{
	GUIElementBase::GUIElementBase()
		: mParentWidget(nullptr), mAnchorParent(nullptr), mUpdateParent(nullptr), mParentElement(nullptr)
		, mFlags(GUIElem_Dirty)
	{

	}

	GUIElementBase::GUIElementBase(const GUIDimensions& dimensions)
		: mParentWidget(nullptr), mAnchorParent(nullptr), mUpdateParent(nullptr), mParentElement(nullptr)
		, mFlags(GUIElem_Dirty), mDimensions(dimensions)
	{

	}

	GUIElementBase::~GUIElementBase()
	{
		Vector<GUIElementBase*> childCopy = mChildren;
		for (auto& child : childCopy)
		{
			if (child->_getType() == Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);
				GUIElement::destroy(element);
			}
			else if (child->_getType() == Type::Layout || child->_getType() == GUIElementBase::Type::Panel)
			{
				GUILayout* layout = static_cast<GUILayout*>(child);
				GUILayout::destroy(layout);
			}
			else if (child->_getType() == Type::FixedSpace)
			{
				GUIFixedSpace* space = static_cast<GUIFixedSpace*>(child);
				GUIFixedSpace::destroy(space);
			}
			else if (child->_getType() == Type::FlexibleSpace)
			{
				GUIFlexibleSpace* space = static_cast<GUIFlexibleSpace*>(child);
				GUIFlexibleSpace::destroy(space);
			}
		}
	}

	void GUIElementBase::setPosition(INT32 x, INT32 y)
	{
		mDimensions.x = x;
		mDimensions.y = y;

		// Note: I could call _markMeshAsDirty with a little more work. If parent is layout then this call can be ignored
		// and if it's a panel, we can immediately change the position without a full layout rebuild.
		_markLayoutAsDirty();
	}

	void GUIElementBase::setWidth(UINT32 width)
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_FixedWidth | GUIDF_OverWidth;
		mDimensions.minWidth = mDimensions.maxWidth = width;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();
			
		_markLayoutAsDirty();
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

		_markLayoutAsDirty();
	}

	void GUIElementBase::setHeight(UINT32 height)
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions.flags |= GUIDF_FixedHeight | GUIDF_OverHeight;
		mDimensions.minHeight = mDimensions.maxHeight = height;

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		_markLayoutAsDirty();
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

		_markLayoutAsDirty();
	}

	void GUIElementBase::resetDimensions()
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions = GUIDimensions::create();

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		_markLayoutAsDirty();
	}

	Rect2I GUIElementBase::getBounds(GUIPanel* relativeTo)
	{
		if (relativeTo == nullptr)
			relativeTo = mAnchorParent;

		Rect2I anchorBounds;
		if (relativeTo != nullptr)
			anchorBounds = relativeTo->getGlobalBounds();

		if (mUpdateParent != nullptr && mUpdateParent->_isDirty() && mParentWidget != nullptr)
			mParentWidget->_updateLayout(mUpdateParent);

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
		if (mUpdateParent != nullptr && mUpdateParent->_isDirty() && mParentWidget != nullptr)
			mParentWidget->_updateLayout(mUpdateParent);

		return mLayoutData.area;
	}

	Rect2I GUIElementBase::getVisibleBounds()
	{
		return getBounds();
	}
	
	void GUIElementBase::_markAsClean()
	{
		mFlags &= ~GUIElem_Dirty;
	}

	void GUIElementBase::_markLayoutAsDirty() 
	{ 
		if(!_isVisible())
			return;

		if (mUpdateParent != nullptr)
			mUpdateParent->mFlags |= GUIElem_Dirty;
		else
			mFlags |= GUIElem_Dirty;
	}

	void GUIElementBase::_markContentAsDirty()
	{
		if (!_isVisible())
			return;

		if (mParentWidget != nullptr)
			mParentWidget->_markContentDirty(this);
	}

	void GUIElementBase::_markMeshAsDirty()
	{
		if(!_isVisible())
			return;

		if (mParentWidget != nullptr)
			mParentWidget->_markMeshDirty(this);
	}

	void GUIElementBase::setVisible(bool visible)
	{
		// No visibility states matter if object is not active
		if (!_isActive())
			return;

		bool visibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
		if (visibleSelf != visible)
		{
			// If making an element visible make sure to mark layout as dirty, as we didn't track any dirty flags while the element was inactive
			if (!visible)
			{
				mFlags |= GUIElem_HiddenSelf;
				_setVisible(false);
			}
			else
			{
				mFlags &= ~GUIElem_HiddenSelf;

				if (mParentElement == nullptr || mParentElement->_isVisible())
					_setVisible(true);
			}
		}
	}

	void GUIElementBase::_setVisible(bool visible)
	{
		bool isVisible = (mFlags & GUIElem_Hidden) == 0;
		if (isVisible == visible)
			return;

		if (!visible)
		{
			_markMeshAsDirty();

			mFlags |= GUIElem_Hidden;

			for (auto& child : mChildren)
				child->_setVisible(false);
		}
		else
		{
			bool childVisibleSelf = (mFlags & GUIElem_HiddenSelf) == 0;
			if (childVisibleSelf)
			{
				mFlags &= ~GUIElem_Hidden;
				_markLayoutAsDirty();

				for (auto& child : mChildren)
					child->_setVisible(true);
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

				_setActive(false);
				_setVisible(false);
			}
			else
			{
				mFlags &= ~ACTIVE_FLAGS;

				if (mParentElement != nullptr)
				{
					if (mParentElement->_isActive())
					{
						_setActive(true);

						if (mParentElement->_isVisible())
							_setVisible(true);
					}
				}
				else
				{
					_setActive(true);
					_setVisible(true);
				}
			}
		}
	}

	void GUIElementBase::_setActive(bool active)
	{
		bool isActive = (mFlags & GUIElem_Inactive) == 0;
		if (isActive == active)
			return;
		
		if (!active)
		{
			_markLayoutAsDirty();

			mFlags |= GUIElem_Inactive;

			for (auto& child : mChildren)
				child->_setActive(false);
		}
		else
		{
			bool childActiveSelf = (mFlags & GUIElem_InactiveSelf) == 0;
			if (childActiveSelf)
			{
				mFlags &= ~GUIElem_Inactive;
				_markLayoutAsDirty();

				for (auto& child : mChildren)
					child->_setActive(true);
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

			_setDisabled(disabled);
		}
	}

	void GUIElementBase::_setDisabled(bool disabled)
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
					child->_setDisabled(false);
			}
		}
		else
		{
			mFlags |= GUIElem_Disabled;

			for (auto& child : mChildren)
				child->_setDisabled(true);
		}

		if (_isVisible())
			_markContentAsDirty();
	}

	void GUIElementBase::_updateLayout(const GUILayoutData& data)
	{
		_updateOptimalLayoutSizes(); // We calculate optimal sizes of all layouts as a pre-processing step, as they are requested often during update
		_updateLayoutInternal(data);
	}

	void GUIElementBase::_updateOptimalLayoutSizes()
	{
		for(auto& child : mChildren)
		{
			child->_updateOptimalLayoutSizes();
		}
	}

	void GUIElementBase::_updateLayoutInternal(const GUILayoutData& data)
	{
		for(auto& child : mChildren)
		{
			child->_updateLayoutInternal(data);
		}
	}

	LayoutSizeRange GUIElementBase::_calculateLayoutSizeRange() const
	{
		const GUIDimensions& dimensions = _getDimensions();
		return dimensions.calculateSizeRange(_getOptimalSize());
	}

	LayoutSizeRange GUIElementBase::_getLayoutSizeRange() const
	{
		return _calculateLayoutSizeRange();
	}

	void GUIElementBase::_getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == 0);
	}

	void GUIElementBase::_setParent(GUIElementBase* parent) 
	{ 
		if(mParentElement != parent)
		{
			mParentElement = parent; 
			_updateAUParents();

			if (parent != nullptr)
			{
				if (_getParentWidget() != parent->_getParentWidget())
					_changeParentWidget(parent->_getParentWidget());
			}
			else
				_changeParentWidget(nullptr);
		}
	}

	void GUIElementBase::_registerChildElement(GUIElementBase* element)
	{
		assert(!element->_isDestroyed());

		GUIElementBase* parentElement = element->_getParent();
		if(parentElement != nullptr)
		{
			parentElement->_unregisterChildElement(element);
		}

		element->_setParent(this);
		mChildren.push_back(element);

		element->_setActive(_isActive());
		element->_setVisible(_isVisible());
		element->_setDisabled(_isDisabled());

		_markLayoutAsDirty();
	}

	void GUIElementBase::_unregisterChildElement(GUIElementBase* element)
	{
		bool foundElem = false;
		for(auto iter = mChildren.begin(); iter != mChildren.end(); ++iter)
		{
			GUIElementBase* child = *iter;

			if (child == element)
			{
				mChildren.erase(iter);
				element->_setParent(nullptr);
				foundElem = true;

				_markLayoutAsDirty();
				break;
			}
		}

		if(!foundElem)
			BS_EXCEPT(InvalidParametersException, "Provided element is not a part of this element.");
	}

	void GUIElementBase::_changeParentWidget(GUIWidget* widget)
	{
		assert(!_isDestroyed());

		if (mParentWidget != widget)
		{
			if (mParentWidget != nullptr)
				mParentWidget->_unregisterElement(this);

			if (widget != nullptr)
				widget->_registerElement(this);
		}

		mParentWidget = widget;

		for(auto& child : mChildren)
		{
			child->_changeParentWidget(widget);
		}

		_markLayoutAsDirty();
	}

	void GUIElementBase::_updateAUParents()
	{
		GUIElementBase* updateParent = nullptr;
		if (mParentElement != nullptr)
		{
			updateParent = mParentElement->findUpdateParent();

			// If parent is a panel then we can do an optimization and only update
			// one child instead of all of them, so change parent to that child.
			if (updateParent != nullptr && updateParent->_getType() == GUIElementBase::Type::Panel)
			{
				GUIElementBase* optimizedUpdateParent = this;
				while (optimizedUpdateParent->_getParent() != updateParent)
					optimizedUpdateParent = optimizedUpdateParent->_getParent();

				updateParent = optimizedUpdateParent;
			}
		}

		GUIPanel* anchorParent = nullptr;
		GUIElementBase* currentParent = mParentElement;
		while (currentParent != nullptr)
		{
			if (currentParent->_getType() == Type::Panel)
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
			const GUIDimensions& parentDimensions = currentElement->_getDimensions();
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
			if (childUpdateParent != nullptr && childUpdateParent->_getType() == GUIElementBase::Type::Panel)
			{
				GUIElementBase* optimizedUpdateParent = child;
				while (optimizedUpdateParent->_getParent() != childUpdateParent)
					optimizedUpdateParent = optimizedUpdateParent->_getParent();

				childUpdateParent = optimizedUpdateParent;
			}

			child->setUpdateParent(childUpdateParent);
		}
	}

	void GUIElementBase::setAnchorParent(GUIPanel* anchorParent)
	{
		mAnchorParent = anchorParent;

		if (_getType() == Type::Panel)
			return;

		for (auto& child : mChildren)
			child->setAnchorParent(anchorParent);
	}

	void GUIElementBase::setUpdateParent(GUIElementBase* updateParent)
	{
		mUpdateParent = updateParent;

		const GUIDimensions& dimensions = _getDimensions();
		bool boundsDependOnChildren = !dimensions.fixedHeight() || !dimensions.fixedWidth();

		if (!boundsDependOnChildren)
			return;

		for (auto& child : mChildren)
			child->setUpdateParent(updateParent);
	}
}