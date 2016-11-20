//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUILayoutUtility.h"
#include "BsGUIElementBase.h"
#include "BsGUILayout.h"

namespace bs
{
	Vector2I GUILayoutUtility::calcOptimalSize(const GUIElementBase* elem)
	{
		return elem->_calculateLayoutSizeRange().optimal;
	}

	Vector2I GUILayoutUtility::calcActualSize(UINT32 width, UINT32 height, GUILayout* layout, bool updateOptimalSizes)
	{
		if (updateOptimalSizes)
			layout->_updateOptimalLayoutSizes();

		return calcActualSizeInternal(width, height, layout);
	}

	Vector2I GUILayoutUtility::calcActualSizeInternal(UINT32 width, UINT32 height, GUILayout* layout)
	{
		UINT32 numElements = (UINT32)layout->_getNumChildren();
		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = bs_stack_new<Rect2I>(numElements);

		Rect2I parentArea;
		parentArea.width = width;
		parentArea.height = height;

		layout->_getElementAreas(parentArea, elementAreas, numElements, layout->_getCachedChildSizeRanges(), layout->_getCachedSizeRange());

		Rect2I* actualAreas = elementAreas; // We re-use the same array
		for (UINT32 i = 0; i < numElements; i++)
		{
			GUIElementBase* child = layout->_getChild(i);
			Rect2I childArea = elementAreas[i];

			if (child->_getType() == GUIElementBase::Type::Layout || child->_getType() == GUIElementBase::Type::Panel)
			{
				Vector2I childActualSize = calcActualSizeInternal(childArea.width, childArea.height, static_cast<GUILayout*>(child));
				actualAreas[i].width = (UINT32)childActualSize.x;
				actualAreas[i].height = (UINT32)childActualSize.y;
			}
			else if (child->_getType() == GUIElementBase::Type::Element)
			{
				RectOffset padding = child->_getPadding();

				actualAreas[i].width = elementAreas[i].width + padding.left + padding.right;
				actualAreas[i].height = elementAreas[i].height + padding.top + padding.bottom;
			}
			else
			{
				actualAreas[i].width = elementAreas[i].width;
				actualAreas[i].height = elementAreas[i].height;
			}
		}

		Vector2I min;
		Vector2I max;

		if (numElements > 0)
		{
			Rect2I childArea = actualAreas[0];

			min = Vector2I(childArea.x, childArea.y);
			max = Vector2I(childArea.x + childArea.width, childArea.y + childArea.height);
		}

		for (UINT32 i = 1; i < numElements; i++)
		{
			Rect2I childArea = actualAreas[i];

			min.x = std::min(min.x, childArea.x);
			min.y = std::min(min.y, childArea.y);

			max.x = std::max(max.x, childArea.x + childArea.width);
			max.y = std::max(max.y, childArea.y + childArea.height);
		}

		Vector2I actualSize = max - min;

		if (elementAreas != nullptr)
			bs_stack_free(elementAreas);

		return actualSize;
	}
}