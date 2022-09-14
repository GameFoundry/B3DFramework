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
			mParentElement->UnregisterChildElementInternal(this);
	}

	void GUILayout::addElement(GUIElementBase* element)
	{
		if (!element->IsDestroyedInternal())
			RegisterChildElementInternal(element);
	}

	void GUILayout::removeElement(GUIElementBase* element)
	{
		UnregisterChildElementInternal(element);
	}

	void GUILayout::insertElement(UINT32 idx, GUIElementBase* element)
	{
		if(idx > (UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((UINT32)mChildren.size()));

		if (element->IsDestroyedInternal())
			return;

		GUIElementBase* parentElement = element->GetParentInternal();
		if(parentElement != nullptr)
		{
			parentElement->UnregisterChildElementInternal(element);
		}

		element->SetParentInternal(this);
		mChildren.insert(mChildren.begin() + idx, element);
		
		element->SetActiveInternal(IsActiveInternal());
		element->SetVisibleInternal(IsVisibleInternal());
		element->SetDisabledInternal(IsDisabledInternal());

		MarkLayoutAsDirtyInternal();
	}

	void GUILayout::clear()
	{
		destroyChildElements();
	}

	void GUILayout::removeElementAt(UINT32 idx)
	{
		if(idx >= (UINT32)mChildren.size())
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + toString(idx) + ". Valid range: 0 .. " + toString((UINT32)mChildren.size()));

		GUIElementBase* child = mChildren[idx];
		mChildren.erase(mChildren.begin() + idx);

		child->SetParentInternal(nullptr);

		MarkLayoutAsDirtyInternal();
	}

	const RectOffset& GUILayout::GetPaddingInternal() const
	{
		static RectOffset padding;

		return padding;
	}

	void GUILayout::destroy(GUILayout* layout)
	{
		bs_delete(layout);
	}
}
