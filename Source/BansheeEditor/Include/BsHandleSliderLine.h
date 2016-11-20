//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsHandleSlider.h"
#include "BsCapsule.h"
#include "BsSphere.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	/**
	 * Handle slider that returns a delta value as you drag the pointer along a line. For intersection purposes the line
	 * is internally by a capsule and a sphere at its cap (assuming this will be used for arrow-like handles).
	 */
	class BS_ED_EXPORT HandleSliderLine : public HandleSlider
	{
	public:
		/**
		 * Constructs a new line slider.
		 *
		 * @param[in]	direction	Direction of the line.
		 * @param[in]	length		Length of the slider.
		 * @param[in]	fixedScale	If true the handle slider will always try to maintain the same visible area in the
		 *							viewport regardless of distance from camera.
		 * @param[in]	layer		Layer that allows filtering of which sliders are interacted with from a specific camera.
		 */
		HandleSliderLine(const Vector3& direction, float length, bool fixedScale, UINT64 layer);
		~HandleSliderLine();

		/** @copydoc	HandleSlider::intersects */
		bool intersects(const Ray& ray, float& t) const override;

		/** @copydoc	HandleSlider::handleInput */
		void handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta) override;

		/**
		 * Returns a delta value that is the result of dragging/sliding the pointer along the line. This changes every
		 * frame and will be zero unless the slider is active.
		 */
		float getDelta() const { return mDelta; }

	protected:
		/** @copydoc HandleSlider::activate */
		void activate(const SPtr<Camera>& camera, const Vector2I& pointerPos) override { mStartPosition = getPosition(); }

		/** @copydoc HandleSlider::reset */
		void reset() override { mDelta = 0.0f; }

		static const float CAPSULE_RADIUS;
		static const float SPHERE_RADIUS;

		Vector3 mDirection;
		float mLength;

		float mDelta;
		Vector3 mStartPosition;

		Capsule mCapsuleCollider;
		Sphere mSphereCollider;
	};

	/** @} */
}