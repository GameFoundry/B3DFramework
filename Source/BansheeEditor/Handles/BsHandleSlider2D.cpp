//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2018 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Handles/BsHandleSlider2D.h"
#include "Handles/BsHandleManager.h"
#include "Handles/BsHandleSliderManager.h"
#include "Math/BsRay.h"
#include "Math/BsRect2I.h"
#include "Renderer/BsCamera.h"

namespace bs
{
	HandleSlider2D::HandleSlider2D(UINT32 width, UINT32 height, UINT64 layer, Slider2DConstraint constraint)
		: HandleSlider(true, layer), mWidth(width), mHeight(height), mConstraint(constraint), mDelta(BsZero)
		, mStartPosition(BsZero)
	{
		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._registerSlider(this);
	}

	HandleSlider2D::~HandleSlider2D()
	{
		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._unregisterSlider(this);
	}

	bool HandleSlider2D::intersects(const Vector2I& screenPos, const Ray& ray, float& t) const
	{
		Vector2I center((INT32)mPosition.x, (INT32)mPosition.y);

		Rect2I currentArea(center.x - mWidth / 2, center.y - mHeight, mWidth, mHeight);
		return currentArea.contains(screenPos);
	}

	void HandleSlider2D::handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta)
	{
		assert(getState() == State::Active);

		mCurrentPointerPos += inputDelta;
		mDelta = mCurrentPointerPos - mStartPosition;

		if(mConstraint == Slider2DConstraint::Horizontal)
			mDelta.y = 0;
		else if(mConstraint == Slider2DConstraint::Vertical)
			mDelta.x = 0;
	}

	void HandleSlider2D::activate(const SPtr<Camera>& camera, const Vector2I& pointerPos)
	{
		mStartPosition = pointerPos;
	}
}
