//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIToggleGroup.h"
#include "BsGUICommandEvent.h"

namespace bs
{
	const String& GUIToggle::GetGuiTypeName()
	{
		static String name = "Toggle";
		return name;
	}

	GUIToggle::GUIToggle(const String& styleName, const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup, const GUIDimensions& dimensions)
		: GUIButtonBase(styleName, content, dimensions), mToggleGroup(nullptr), mIsToggled(false)
	{
		if(toggleGroup != nullptr)
			toggleGroup->AddInternal(this);
	}

	GUIToggle::~GUIToggle()
	{
		if(mToggleGroup != nullptr)
		{
			mToggleGroup->RemoveInternal(this);
		}
	}

	GUIToggle* GUIToggle::Create(const HString& text, const String& styleName)
	{
		return Create(GUIContent(text), styleName);
	}

	GUIToggle* GUIToggle::Create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return Create(GUIContent(text), options, styleName);
	}

	GUIToggle* GUIToggle::Create(const HString& text, SPtr<GUIToggleGroup> toggleGroup, const String& styleName)
	{
		return Create(GUIContent(text), toggleGroup, styleName);
	}

	GUIToggle* GUIToggle::Create(const HString& text, SPtr<GUIToggleGroup> toggleGroup, const GUIOptions& options, const String& styleName)
	{
		return Create(GUIContent(text), toggleGroup, options, styleName);
	}

	GUIToggle* GUIToggle::Create(const GUIContent& content, const String& styleName)
	{
		return new(bs_alloc<GUIToggle>()) GUIToggle(GetStyleName<GUIToggle>(styleName), content, nullptr, GUIDimensions::Create());
	}

	GUIToggle* GUIToggle::Create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIToggle>()) GUIToggle(GetStyleName<GUIToggle>(styleName), content, nullptr, GUIDimensions::Create(options));
	}

	GUIToggle* GUIToggle::Create(const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup, const String& styleName)
	{
		return new(bs_alloc<GUIToggle>()) GUIToggle(GetStyleName<GUIToggle>(styleName), content, toggleGroup, GUIDimensions::Create());
	}

	GUIToggle* GUIToggle::Create(const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup, const GUIOptions& options, const String& styleName)
	{
		return new(bs_alloc<GUIToggle>()) GUIToggle(GetStyleName<GUIToggle>(styleName), content, toggleGroup, GUIDimensions::Create(options));
	}

	SPtr<GUIToggleGroup> GUIToggle::CreateToggleGroup(bool allowAllOff)
	{
		SPtr<GUIToggleGroup> toggleGroup = bs_shared_ptr<GUIToggleGroup>(new(bs_alloc<GUIToggleGroup>()) GUIToggleGroup(allowAllOff));
		toggleGroup->Initialize(toggleGroup);

		return toggleGroup;
	}

	void GUIToggle::SetToggleGroupInternal(SPtr<GUIToggleGroup> toggleGroup)
	{
		mToggleGroup = toggleGroup;

		bool isToggled = false;
		if(mToggleGroup != nullptr) // If in group ensure at least one element is toggled on
		{
			for(auto& toggleElem : mToggleGroup->mButtons)
			{
				if(isToggled)
				{
					if(toggleElem->mIsToggled)
						toggleElem->ToggleOff();
				}
				else
				{
					if(toggleElem->mIsToggled)
						isToggled = true;
				}
			}

			if(!isToggled && !toggleGroup->mAllowAllOff)
				ToggleOn();
		}
	}

	void GUIToggle::ToggleOnInternal(bool triggerEvent)
	{
		if(mIsToggled)
			return;

		mIsToggled = true;

		if(triggerEvent)
		{
			if(!OnToggled.Empty())
				OnToggled(mIsToggled);
		}

		if(mToggleGroup != nullptr)
		{
			for(auto& toggleElem : mToggleGroup->mButtons)
			{
				if(toggleElem != this)
					toggleElem->ToggleOffInternal(triggerEvent);
			}
		}

		SetOnInternal(true);
	}

	void GUIToggle::ToggleOffInternal(bool triggerEvent)
	{
		if(!mIsToggled)
			return;

		bool canBeToggledOff = false;
		if(mToggleGroup != nullptr) // If in group ensure at least one element is toggled on
		{
			for(auto& toggleElem : mToggleGroup->mButtons)
			{
				if(toggleElem != this)
				{
					if(toggleElem->mIsToggled)
					{
						canBeToggledOff = true;
						break;
					}
				}
			}
		}
		else
			canBeToggledOff = true;

		if(canBeToggledOff || mToggleGroup->mAllowAllOff)
		{
			mIsToggled = false;

			if(triggerEvent)
			{
				if(!OnToggled.Empty())
					OnToggled(mIsToggled);
			}

			SetOnInternal(false);
		}
	}

	bool GUIToggle::MouseEventInternal(const GUIMouseEvent& ev)
	{
		bool processed = GUIButtonBase::MouseEventInternal(ev);

		if(ev.GetType() == GUIMouseEventType::MouseUp)
		{
			if(!IsDisabledInternal())
			{
				if(mIsToggled)
					ToggleOffInternal(true);
				else
					ToggleOnInternal(true);
			}

			processed = true;
		}

		return processed;
	}

	bool GUIToggle::CommandEventInternal(const GUICommandEvent& ev)
	{
		const bool processed = GUIButtonBase::CommandEventInternal(ev);

		if(ev.GetType() == GUICommandEventType::Confirm)
		{
			if(!IsDisabledInternal())
			{
				if(mIsToggled)
					ToggleOffInternal(true);
				else
					ToggleOnInternal(true);
			}

			return true;
		}

		return processed;
	}
} // namespace bs
