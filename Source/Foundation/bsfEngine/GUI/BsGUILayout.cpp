//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUISpace.h"
#include "Error/BsException.h"

using namespace bs;

GUILayout::GUILayout(const GUIDimensions& dimensions)
	: GUIElementBase(dimensions)
{}

GUILayout::GUILayout()
{}

GUILayout::~GUILayout()
{
	if(mParent != nullptr)
		mParent->UnregisterChildElement(this);
}

void GUILayout::AddElement(GUIElementBase* element)
{
	if(!element->IsDestroyed())
		RegisterChildElement(element);
}

void GUILayout::RemoveElement(GUIElementBase* element)
{
	UnregisterChildElement(element);
}

void GUILayout::InsertElement(u32 idx, GUIElementBase* element)
{
	if(idx > (u32)mChildren.size())
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((u32)mChildren.size()));

	if(element->IsDestroyed())
		return;

	GUIElementBase* parentElement = element->GetParent();
	if(parentElement != nullptr)
	{
		parentElement->UnregisterChildElement(element);
	}

	element->SetParent(this);
	mChildren.Insert(mChildren.begin() + idx, element);

	element->SetActiveRecursive(IsActive());
	element->SetVisibleRecursive(IsVisible());
	element->SetDisabledRecursive(IsDisabled());

	MarkLayoutAsDirty();
}

void GUILayout::Clear()
{
	DestroyChildElements();
}

void GUILayout::RemoveElementAt(u32 idx)
{
	if(idx >= (u32)mChildren.size())
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + ToString(idx) + ". Valid range: 0 .. " + ToString((u32)mChildren.size()));

	GUIElementBase* child = mChildren[idx];
	mChildren.erase(mChildren.begin() + idx);

	child->SetParent(nullptr);

	MarkLayoutAsDirty();
}

const RectOffset& GUILayout::GetPadding() const
{
	static RectOffset padding;

	return padding;
}

void GUILayout::Destroy(GUILayout* layout)
{
	B3DDelete(layout);
}
