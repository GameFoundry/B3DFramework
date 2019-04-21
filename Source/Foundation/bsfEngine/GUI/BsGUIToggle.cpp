//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIToggleGroup.h"
#include "BsGUICommandEvent.h"

namespace bs
{
	const String& GUIToggle::getGUITypeName()
	{
		static String name = "Toggle";
		return name;
	}

	GUIToggle::GUIToggle(const String& styleName, const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup, const GUIDimensions& dimensions)
		:GUIButtonBase(styleName, content, dimensions), mToggleGroup(nullptr), mIsToggled(false)
	{
		if(toggleGroup != nullptr)
			toggleGroup->_add(this);
	}

	GUIToggle::~GUIToggle()
	{
		if(mToggleGroup != nullptr)
		{
			mToggleGroup->_remove(this);
		}
	}

	GUIToggle* GUIToggle::create(const HString& text, const String& styleName)
	{
		return create(GUIContent(text), styleName);
	}

	GUIToggle* GUIToggle::create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return create(GUIContent(text), options, styleName);
	}

	GUIToggle* GUIToggle::create(const HString& text, SPtr<GUIToggleGroup> toggleGroup, const String& styleName)
	{
		return create(GUIContent(text), toggleGroup, styleName);
	}

	GUIToggle* GUIToggle::create(const HString& text, SPtr<GUIToggleGroup> toggleGroup,
		const GUIOptions& options, const String& styleName)
	{
		return create(GUIContent(text), toggleGroup, options, styleName);
	}

	GUIToggle* GUIToggle::create(const GUIContent& content, const String& styleName)
	{
		return new (bs_alloc<GUIToggle>()) GUIToggle(getStyleName<GUIToggle>(styleName), content, nullptr, GUIDimensions::create());
	}

	GUIToggle* GUIToggle::create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIToggle>()) GUIToggle(getStyleName<GUIToggle>(styleName), content, nullptr, GUIDimensions::create(options));
	}

	GUIToggle* GUIToggle::create(const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup, const String& styleName)
	{
		return new (bs_alloc<GUIToggle>()) GUIToggle(getStyleName<GUIToggle>(styleName), content, toggleGroup, GUIDimensions::create());
	}

	GUIToggle* GUIToggle::create(const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup,
		const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIToggle>()) GUIToggle(getStyleName<GUIToggle>(styleName), content, toggleGroup, GUIDimensions::create(options));
	}

	SPtr<GUIToggleGroup> GUIToggle::createToggleGroup(bool allowAllOff)
	{
		SPtr<GUIToggleGroup> toggleGroup = bs_shared_ptr<GUIToggleGroup>(new (bs_alloc<GUIToggleGroup>()) GUIToggleGroup(allowAllOff));
		toggleGroup->initialize(toggleGroup);

		return toggleGroup;
	}

	void GUIToggle::_setToggleGroup(SPtr<GUIToggleGroup> toggleGroup)
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
						toggleElem->toggleOff();
				}
				else
				{
					if(toggleElem->mIsToggled)
						isToggled = true;
				}

			}

			if(!isToggled && !toggleGroup->mAllowAllOff)
				toggleOn();
		}
	}

	void GUIToggle::_toggleOn(bool triggerEvent)
	{
		if(mIsToggled)
			return;

		mIsToggled = true;

		if(triggerEvent)
		{
			if (!onToggled.empty())
				onToggled(mIsToggled);
		}

		if(mToggleGroup != nullptr)
		{
			for(auto& toggleElem : mToggleGroup->mButtons)
			{
				if(toggleElem != this)
					toggleElem->_toggleOff(triggerEvent);
			}
		}

		_setOn(true);
	}

	void GUIToggle::_toggleOff(bool triggerEvent)
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

		if (canBeToggledOff || mToggleGroup->mAllowAllOff)
		{
			mIsToggled = false;

			if(triggerEvent)
			{
				if (!onToggled.empty())
					onToggled(mIsToggled);
			}

			_setOn(false);
		}
	}

	bool GUIToggle::_mouseEvent(const GUIMouseEvent& ev)
	{
		bool processed = GUIButtonBase::_mouseEvent(ev);

		if(ev.getType() == GUIMouseEventType::MouseUp)
		{
			if (!_isDisabled())
			{
				if (mIsToggled)
					_toggleOff(true);
				else
					_toggleOn(true);
			}

			processed = true;
		}

		return processed;
	}

	bool GUIToggle::_commandEvent(const GUICommandEvent& ev)
	{
		const bool processed = GUIButtonBase::_commandEvent(ev);

		if(ev.getType() == GUICommandEventType::Confirm)
		{
			if(!_isDisabled())
			{
				if(mIsToggled)
					_toggleOff(true);
				else
					_toggleOn(true);
			}

			return true;
		}

		return processed;
	}
}
