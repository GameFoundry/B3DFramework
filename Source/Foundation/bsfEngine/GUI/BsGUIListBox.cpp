//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIListBox.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUIDropDownBoxManager.h"

using namespace bs;

const String& GUIListBox::GetGuiTypeName()
{
	static String name = "ListBox";
	return name;
}

GUIListBox::GUIListBox(const String& styleName, const Vector<HString>& elements, bool isMultiselect, const GUIDimensions& dimensions)
	: GUIButtonBase(styleName, GUIContent(HString("")), dimensions), mElements(elements), mIsMultiselect(isMultiselect)
{
	mElementStates.resize(elements.size(), false);
	if(!mIsMultiselect && mElementStates.size() > 0)
		mElementStates[0] = true;

	UpdateContents();
}

GUIListBox::~GUIListBox()
{
	CloseListBox();
}

GUIListBox* GUIListBox::Create(const Vector<HString>& elements, bool isMultiselect, const String& styleName)
{
	return new(B3DAllocate<GUIListBox>()) GUIListBox(GetStyleName<GUIListBox>(styleName), elements, isMultiselect, GUIDimensions::Create());
}

GUIListBox* GUIListBox::Create(const Vector<HString>& elements, bool isMultiselect, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIListBox>()) GUIListBox(GetStyleName<GUIListBox>(styleName), elements, isMultiselect, GUIDimensions::Create(options));
}

GUIListBox* GUIListBox::Create(const Vector<HString>& elements, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIListBox>()) GUIListBox(GetStyleName<GUIListBox>(styleName), elements, false, GUIDimensions::Create(options));
}

void GUIListBox::SetElements(const Vector<HString>& elements)
{
	bool wasOpen = mDropDownBox != nullptr;

	if(wasOpen)
		CloseListBox();

	mElements = elements;

	mElementStates.clear();
	mElementStates.resize(mElements.size(), false);
	if(!mIsMultiselect && mElementStates.size() > 0)
		mElementStates[0] = true;

	UpdateContents();

	if(wasOpen)
		OpenListBox();
}

void GUIListBox::SelectElement(u32 idx)
{
	if(idx >= (u32)mElements.size())
		return;

	if(mElementStates[idx] != true)
		ElementSelected(idx);
}

void GUIListBox::DeselectElement(u32 idx)
{
	if(!mIsMultiselect || idx >= (u32)mElements.size())
		return;

	if(mElementStates[idx] != false)
		ElementSelected(idx);
}

void GUIListBox::SetElementStates(const Vector<bool>& states)
{
	u32 numElements = (u32)mElementStates.size();
	u32 min = std::min(numElements, (u32)states.size());

	bool anythingModified = min != numElements;
	if(!anythingModified)
	{
		for(u32 i = 0; i < numElements; i++)
		{
			if(mElementStates[i] != states[i])
			{
				anythingModified = true;
				break;
			}
		}
	}

	if(!anythingModified)
		return;

	bool wasOpen = mDropDownBox != nullptr;

	if(wasOpen)
		CloseListBox();

	for(u32 i = 0; i < min; i++)
	{
		mElementStates[i] = states[i];

		if(mElementStates[i] && !mIsMultiselect)
		{
			for(u32 j = i + 1; j < numElements; j++)
				mElementStates[j] = false;

			break;
		}
	}

	UpdateContents();

	if(wasOpen)
		OpenListBox();
}

bool GUIListBox::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	bool processed = GUIButtonBase::DoOnMouseEvent(ev);

	if(ev.GetType() == GUIMouseEventType::MouseDown)
	{
		if(!IsDisabled())
		{
			if(mDropDownBox == nullptr)
				OpenListBox();
			else
				CloseListBox();
		}

		processed = true;
	}

	return processed;
}

bool GUIListBox::DoOnCommandEvent(const GUICommandEvent& ev)
{
	const bool processed = GUIButtonBase::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::Confirm)
	{
		if(!IsDisabled())
		{
			if(mDropDownBox == nullptr)
				OpenListBox();
			else
				CloseListBox();
		}

		return true;
	}

	return processed;
}

void GUIListBox::ElementSelected(u32 idx)
{
	if(idx >= (u32)mElements.size())
		return;

	if(mIsMultiselect)
	{
		bool selected = mElementStates[idx];
		mElementStates[idx] = !selected;

		if(!OnSelectionToggled.Empty())
			OnSelectionToggled(idx, !selected);
	}
	else
	{
		for(u32 i = 0; i < (u32)mElementStates.size(); i++)
			mElementStates[i] = false;

		mElementStates[idx] = true;

		if(!OnSelectionToggled.Empty())
			OnSelectionToggled(idx, true);

		CloseListBox();
	}

	UpdateContents();
}

void GUIListBox::OpenListBox()
{
	CloseListBox();

	DROP_DOWN_BOX_DESC desc;

	u32 i = 0;
	for(auto& elem : mElements)
	{
		String identifier = ToString(i);
		desc.DropDownData.Entries.push_back(GUIDropDownDataEntry::Button(identifier, std::bind(&GUIListBox::ElementSelected, this, i)));
		desc.DropDownData.LocalizedNames[identifier] = elem;
		i++;
	}

	GUIWidget* widget = GetParentWidget();

	desc.Camera = widget->GetCamera();
	desc.Skin = widget->GetSkinResource();
	desc.Placement = DropDownAreaPlacement::AroundBoundsHorz(mClippedBounds);
	desc.DropDownData.States = mElementStates;

	GUIDropDownType type;
	if(mIsMultiselect)
		type = GUIDropDownType::MultiListBox;
	else
		type = GUIDropDownType::ListBox;

	mDropDownBox = GUIDropDownBoxManager::Instance().OpenDropDownBox(
		desc, type, std::bind(&GUIListBox::OnListBoxClosed, this));

	SetOnInternal(true);
}

void GUIListBox::CloseListBox()
{
	if(mDropDownBox != nullptr)
	{
		GUIDropDownBoxManager::Instance().CloseDropDownBox();

		SetOnInternal(false);
		mDropDownBox = nullptr;
	}
}

void GUIListBox::UpdateContents()
{
	u32 selectedIdx = 0;
	u32 numSelected = 0;
	for(u32 i = 0; i < (u32)mElementStates.size(); i++)
	{
		if(mElementStates[i])
		{
			selectedIdx = i;
			numSelected++;
		}
	}

	if(mIsMultiselect)
	{
		if(numSelected == 1)
			SetContent(GUIContent(mElements[selectedIdx]));
		else if(numSelected == 0)
			SetContent(GUIContent(HEString("None")));
		else
			SetContent(GUIContent(HEString("Multiple")));
	}
	else
	{
		if(!mElements.empty())
			SetContent(GUIContent(mElements[selectedIdx]));
		else
			SetContent(GUIContent(HEString("None")));
	}
}

void GUIListBox::OnListBoxClosed()
{
	SetOnInternal(false);
	mDropDownBox = nullptr;
}
