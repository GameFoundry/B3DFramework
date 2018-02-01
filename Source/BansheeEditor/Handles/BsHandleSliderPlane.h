//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "Handles/BsHandleSlider.h"
#include "Math/BsRect3.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	/**
	 * Handle slider that returns a delta value as you drag the pointer	along a plane. For intersection purposes the line
	 * is internally by a quadrilateral.
	 */
	class BS_ED_EXPORT HandleSliderPlane : public HandleSlider
	{
	public:
		/**
		 * Constructs a new plane slider. The plane is constructed from two direction vectors.
		 *
		 * @param[in]	dir1		First direction of the plane. The x component of returned delta value will be in this
		 *							direction. Should be perpendicular to @p dir2.
		 * @param[in]	dir2		Second direction of the plane. The y component of returned delta value will be in this
		 *							direction. Should be perpendicular to @p dir1.
		 * @param[in]	length		Determines size of the plane. 
		 * @param[in]	fixedScale	If true the handle slider will always try to maintain the same visible area in the
		 *							viewport regardless of distance from camera.
		 * @param[in]	layer		Layer that allows filtering of which sliders are interacted with from a specific camera.
		 */
		HandleSliderPlane(const Vector3& dir1, const Vector3& dir2, float length, bool fixedScale, UINT64 layer);
		~HandleSliderPlane();

		/** @copydoc HandleSlider::intersects */
		bool intersects(const Vector2I& screenPos, const Ray& ray, float& t) const override;

		/** @copydoc HandleSlider::handleInput */
		void handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta) override;

		/**
		 * Returns a delta value that is the result of dragging/sliding the pointer along the plane. Returned movement is
		 * in terms of the two directions originally provided when constructing the slider. This changes every frame and
		 * will be zero unless the slider is active.
		 */
		Vector2 getDelta() const { return mDelta; }
	protected:
		/** @copydoc HandleSlider::activate */
		void activate(const SPtr<Camera>& camera, const Vector2I& pointerPos) override;

		/** @copydoc HandleSlider::reset */
		void reset() override { mDelta = Vector2::ZERO; }

		/**
		 * Returns the position on plane based on pointer position.
		 *
		 * @param[in]	camera		Camera we're interacting through.
		 * @param[in]	pointerPos	Position of the pointer in pixels relative to the provided camera's viewport.
		 */
		Vector3 getPositionOnPlane(const SPtr<Camera>& camera, const Vector2I& pointerPos) const;

		Vector3 mDirection1;
		Vector3 mDirection2;
		float mLength;

		Rect3 mCollider;

		Vector2 mDelta;
		Vector3 mStartPlanePosition;
		Vector3 mStartClickPosition;
	};

	/** @} */
}