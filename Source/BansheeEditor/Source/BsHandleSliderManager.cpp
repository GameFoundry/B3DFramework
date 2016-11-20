//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHandleSliderManager.h"
#include "BsBuiltinEditorResources.h"
#include "BsCCamera.h"
#include "BsHandleSlider.h"

using namespace std::placeholders;

namespace bs
{
	void HandleSliderManager::update(const SPtr<Camera>& camera, const Vector2I& inputPos, const Vector2I& inputDelta)
	{
		for (auto& slider : mSliders)
		{
			bool layerMatches = (camera->getLayers() & slider->getLayer()) != 0;

			if(layerMatches)
				slider->update(camera);
		}

		StatePerCamera& state = mStates[camera->getInternalID()];
		if (state.activeSlider != nullptr)
		{
			bool layerMatches = (camera->getLayers() & state.activeSlider->getLayer()) != 0;

			if (layerMatches)
				state.activeSlider->handleInput(camera, inputDelta);
		}
		else
		{
			HandleSlider* newHoverSlider = findUnderCursor(camera, inputPos);

			if (newHoverSlider != state.hoverSlider)
			{
				if (state.hoverSlider != nullptr)
				{
					state.hoverSlider->setInactive();
					state.hoverSlider = nullptr;
				}

				if (newHoverSlider != nullptr)
				{
					state.hoverSlider = newHoverSlider;
					state.hoverSlider->setHover();
				}
			}
		}
	}

	void HandleSliderManager::trySelect(const SPtr<Camera>& camera, const Vector2I& inputPos)
	{
		HandleSlider* newActiveSlider = findUnderCursor(camera, inputPos);

		StatePerCamera& state = mStates[camera->getInternalID()];
		if (state.hoverSlider != nullptr)
		{
			state.hoverSlider->setInactive();
			state.hoverSlider = nullptr;
		}

		if (newActiveSlider != state.activeSlider)
		{
			if (state.activeSlider != nullptr)
			{
				state.activeSlider->setInactive();
				state.activeSlider = nullptr;
			}

			if (newActiveSlider != nullptr)
			{
				state.activeSlider = newActiveSlider;
				state.activeSlider->setActive(camera, inputPos);
			}
		}
	}

	bool HandleSliderManager::isSliderActive(const SPtr<Camera>& camera) const
	{
		auto iterFind = mStates.find(camera->getInternalID());
		if (iterFind == mStates.end())
			return false;

		return iterFind->second.activeSlider != nullptr;
	}

	void HandleSliderManager::clearSelection(const SPtr<Camera>& camera)
	{
		StatePerCamera& state = mStates[camera->getInternalID()];
		if (state.activeSlider != nullptr)
		{
			state.activeSlider->setInactive();
			state.activeSlider = nullptr;
		}
	}

	HandleSlider* HandleSliderManager::findUnderCursor(const SPtr<Camera>& camera, const Vector2I& inputPos) const
	{
		Ray inputRay = camera->screenPointToRay(inputPos);

		float nearestT = std::numeric_limits<float>::max();
		HandleSlider* overSlider = nullptr;
		
		for (auto& slider : mSliders)
		{
			if (!slider->getEnabled())
				continue;

			bool layerMatches = (camera->getLayers() & slider->getLayer()) != 0;

			float t;
			if (layerMatches && slider->intersects(inputRay, t))
			{
				if (t < nearestT)
				{
					overSlider = slider;
					nearestT = t;
				}
			}
		}

		return overSlider;
	}

	void HandleSliderManager::_registerSlider(HandleSlider* slider)
	{
		mSliders.insert(slider);
	}

	void HandleSliderManager::_unregisterSlider(HandleSlider* slider)
	{
		mSliders.erase(slider);

		for(auto& entry : mStates)
		{
			if (entry.second.activeSlider == slider)
				entry.second.activeSlider = nullptr;

			if (entry.second.hoverSlider == slider)
				entry.second.hoverSlider = nullptr;
		}
	}
}