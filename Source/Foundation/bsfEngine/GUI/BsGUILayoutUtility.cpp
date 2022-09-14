//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayoutUtility.h"
#include "GUI/BsGUIElementBase.h"
#include "GUI/BsGUILayout.h"

namespace bs
{
	Vector2I GUILayoutUtility::calcOptimalSize(const GUIElementBase* elem)
	{
		return elem->CalculateLayoutSizeRangeInternal().optimal;
	}

	Vector2I GUILayoutUtility::calcActualSize(UINT32 width, UINT32 height, GUILayout* layout, bool updateOptimalSizes)
	{
		if (updateOptimalSizes)
			layout->UpdateOptimalLayoutSizesInternal();

		return calcActualSizeInternal(width, height, layout);
	}

	Vector2I GUILayoutUtility::calcActualSizeInternal(UINT32 width, UINT32 height, GUILayout* layout)
	{
		UINT32 numElements = (UINT32)layout->GetNumChildrenInternal();
		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = bs_stack_new<Rect2I>(numElements);

		Rect2I parentArea;
		parentArea.width = width;
		parentArea.height = height;

		layout->GetElementAreasInternal(parentArea, elementAreas, numElements, layout->GetCachedChildSizeRangesInternal(), layout->GetCachedSizeRangeInternal());

		Rect2I* actualAreas = elementAreas; // We re-use the same array
		for (UINT32 i = 0; i < numElements; i++)
		{
			GUIElementBase* child = layout->GetChildInternal(i);
			Rect2I childArea = elementAreas[i];

			if (child->GetTypeInternal() == GUIElementBase::Type::Layout || child->GetTypeInternal() == GUIElementBase::Type::Panel)
			{
				Vector2I childActualSize = calcActualSizeInternal(childArea.width, childArea.height, static_cast<GUILayout*>(child));
				actualAreas[i].width = (UINT32)childActualSize.x;
				actualAreas[i].height = (UINT32)childActualSize.y;
			}
			else if (child->GetTypeInternal() == GUIElementBase::Type::Element)
			{
				RectOffset padding = child->GetPaddingInternal();

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

			max.x = std::max(max.x, childArea.x + (INT32)childArea.width);
			max.y = std::max(max.y, childArea.y + (INT32)childArea.height);
		}

		Vector2I actualSize = max - min;

		if (elementAreas != nullptr)
			bs_stack_free(elementAreas);

		return actualSize;
	}
}
