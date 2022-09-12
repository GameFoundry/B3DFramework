//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUISpace.h"
#include "Error/BsException.h"

namespace bs
{
	GUILayout::GUILayout(const GUIDimensions& dimensions)
		:GUIElementBase(dimensions)
	{ }

	GUILayout::GUILayout()
	{ }

	GUILayout::~GUILayout()
	{
		if (mParentElement != nullptr)
			mParentElement->_unregisterChildElement(this);
	}

	void GUILayout::AddElement(GUIElementBase* element)
	{
		if (!element->_isDestroyed())
			_registerChildElement(element);
	}

	void GUILayout::RemoveElement(GUIElementBase* element)
	{
		_unregisterChildElement(element);
	}

	void GUILayout::InsertElement(UINT32 idx, GUIElementBase* element)
	{
		if(idx > (UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " +
												  ToString((UINT32) mChildren.size()));

		if (element->_isDestroyed())
			return;

		GUIElementBase* parentElement = element->_getParent();
		if(parentElement != nullptr)
		{
			parentElement->_unregisterChildElement(element);
		}

		element->_setParent(this);
		mChildren.Insert(mChildren.begin() + idx, element);
		
		element->_setActive(_isActive());
		element->_setVisible(_isVisible());
		element->_setDisabled(_isDisabled());

		_markLayoutAsDirty();
	}

	void GUILayout::Clear()
	{
		destroyChildElements();
	}

	void GUILayout::RemoveElementAt(UINT32 idx)
	{
		if(idx >= (UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " +
												  ToString((UINT32) mChildren.size()));

		GUIElementBase* child = mChildren[idx];
		mChildren.erase(mChildren.begin() + idx);

		child->_setParent(nullptr);

		_markLayoutAsDirty();
	}

	const RectOffset& GUILayout::_getPadding() const
	{
		static RectOffset padding;

		return padding;
	}

	void GUILayout::Destroy(GUILayout* layout)
	{
		bs_delete(layout);
	}
}
