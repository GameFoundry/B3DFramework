//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIToggleGroup.h"
#include "GUI/B3DGUIToggle.h"

using namespace b3d;

GUIToggleGroup::GUIToggleGroup(bool allowAllOff)
	: mAllowAllOff(allowAllOff)
{}

GUIToggleGroup::~GUIToggleGroup()
{
	for(auto& button : mButtons)
	{
		button->SetToggleGroupInternal(nullptr);
	}
}

void GUIToggleGroup::Initialize(const SPtr<GUIToggleGroup>& sharedPtr)
{
	mThis = sharedPtr;
}

SPtr<GUIToggleGroup> GUIToggleGroup::Create(bool allowAllOff)
{
	SPtr<GUIToggleGroup> toggleGroup = B3DMakeSharedFromExisting<GUIToggleGroup>(new(B3DAllocate<GUIToggleGroup>()) GUIToggleGroup(allowAllOff));
	toggleGroup->Initialize(toggleGroup);

	return toggleGroup;
}

void GUIToggleGroup::AddInternal(GUIToggleable* toggle)
{
	auto found = std::find(begin(mButtons), end(mButtons), toggle);
	if(found != end(mButtons))
		return;

	mButtons.push_back(toggle);
	toggle->SetToggleGroupInternal(mThis.lock());
}

void GUIToggleGroup::RemoveInternal(GUIToggleable* toggle)
{
	auto sharedPtr = mThis.lock(); // Make sure we keep a reference because calling SetToggleGroupInternal(nullptr)
								   // may otherwise clear the last reference and cause us to destruct

	auto found = std::find(begin(mButtons), end(mButtons), toggle);
	if(found == end(mButtons))
		return;

	(*found)->SetToggleGroupInternal(nullptr);
	mButtons.erase(found);
}
