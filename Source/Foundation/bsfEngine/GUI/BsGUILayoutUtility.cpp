//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIUtility.h"
#include "GUI/BsGUIElementBase.h"
#include "GUI/BsGUILayout.h"

using namespace bs;

Vector2I GUIUtility::CalcOptimalSize(const GUIElementBase* elem)
{
	return elem->CalculateConstrainedSize().Optimal;
}

Vector2I GUIUtility::CalcActualSize(u32 width, u32 height, GUILayout* layout, bool updateOptimalSizes)
{
	if(updateOptimalSizes)
		layout->UpdateOptimalLayoutSizes();

	return CalcActualSizeInternal(width, height, layout);
}

Vector2I GUIUtility::CalcActualSizeInternal(u32 width, u32 height, GUILayout* layout)
{
	u32 numElements = (u32)layout->GetChildCount();
	Rect2I* elementAreas = nullptr;

	if(numElements > 0)
		elementAreas = B3DStackNew<Rect2I>(numElements);

	Rect2I parentArea;
	parentArea.Width = width;
	parentArea.Height = height;

	layout->GetChildLayoutAreas(parentArea, elementAreas, numElements, layout->GetChildrenConstrainedSizes(), layout->GetCachedConstrainedSize());

	Rect2I* actualAreas = elementAreas; // We re-use the same array
	for(u32 i = 0; i < numElements; i++)
	{
		GUIElementBase* child = layout->GetChild(i);
		Rect2I childArea = elementAreas[i];

		if(child->GetType() == GUIElementBase::Type::Layout || child->GetType() == GUIElementBase::Type::Panel)
		{
			Vector2I childActualSize = CalcActualSizeInternal(childArea.Width, childArea.Height, static_cast<GUILayout*>(child));
			actualAreas[i].Width = (u32)childActualSize.X;
			actualAreas[i].Height = (u32)childActualSize.Y;
		}
		else if(child->GetType() == GUIElementBase::Type::Interactable)
		{
			RectOffset padding = child->GetMargins();

			actualAreas[i].Width = elementAreas[i].Width + padding.Left + padding.Right;
			actualAreas[i].Height = elementAreas[i].Height + padding.Top + padding.Bottom;
		}
		else
		{
			actualAreas[i].Width = elementAreas[i].Width;
			actualAreas[i].Height = elementAreas[i].Height;
		}
	}

	Vector2I min;
	Vector2I max;

	if(numElements > 0)
	{
		Rect2I childArea = actualAreas[0];

		min = Vector2I(childArea.X, childArea.Y);
		max = Vector2I(childArea.X + childArea.Width, childArea.Y + childArea.Height);
	}

	for(u32 i = 1; i < numElements; i++)
	{
		Rect2I childArea = actualAreas[i];

		min.X = std::min(min.X, childArea.X);
		min.Y = std::min(min.Y, childArea.Y);

		max.X = std::max(max.X, childArea.X + (i32)childArea.Width);
		max.Y = std::max(max.Y, childArea.Y + (i32)childArea.Height);
	}

	Vector2I actualSize = max - min;

	if(elementAreas != nullptr)
		B3DStackFree(elementAreas);

	return actualSize;
}
