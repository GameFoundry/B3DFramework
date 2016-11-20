//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsHandleSlider.h"
#include "BsTorus.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	/**
	 * Handle slider that returns a delta value as you drag the pointer along a disc. For intersection purposes the disc is
	 * internally represented by a torus.
	 */
	class BS_ED_EXPORT HandleSliderDisc : public HandleSlider
	{
	public:
		/**
		 * Constructs a new disc slider.
		 *
		 * @param[in]	normal		Normal that determines the orientation of the disc.
		 * @param[in]	radius		Radius of the disc.
		 * @param[in]	fixedScale	If true the handle slider will always try to maintain the same visible area in the
		 *							viewport regardless of distance from camera.
		 * @param[in]	layer		Layer that allows filtering of which sliders are interacted with from a specific camera.
		 */
		HandleSliderDisc(const Vector3& normal, float radius, bool fixedScale, UINT64 layer);
		~HandleSliderDisc();

		/** @copydoc	HandleSlider::intersects */
		bool intersects(const Ray& ray, float& t) const override;

		/** @copydoc	HandleSlider::handleInput */
		void handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta) override;

		/**
		 * Enables or disables a cut-off plane that can allow the disc to be intersected with only in an 180 degree arc.
		 *
		 * @param[in]	angle	Angle at which to start the cut-off. Points on the dist at the specified angle and the next
		 *						180 degrees won't be interactable.
		 * @param[in]	enabled	True to enable the cutoff plane, false otherwise.
		 */
		void setCutoffPlane(Degree angle, bool enabled);

		/**
		 * Returns a delta value that is the result of dragging/sliding the pointer along the disc. This changes every frame
		 * and will be zero unless the slider is active.
		 */
		Radian getDelta() const { return mDelta; }

		/**
		 * Gets the initial angle at which the drag/slide operation started. This is only valid when the slider is active.
		 */
		Radian getStartAngle() const { return mStartAngle; }

	protected:

		/** @copydoc HandleSlider::activate */
		void activate(const SPtr<Camera>& camera, const Vector2I& pointerPos) override;

		/** @copydoc HandleSlider::reset */
		void reset() override { mDelta = 0.0f; }

		/**
		 * Calculates the closest point on an arc from a ray.
		 *
		 * @param[in]	inputRay	Ray to use for determining the point.
		 * @param[in]	center		Center of the arc.
		 * @param[in]	up			Normal vector of the arc. Must be normalized.
		 * @param[in]	radius		Radius of the arc.
		 * @param[in]	startAngle	Starting angle of the arc.
		 * @param[in]	angleAmount	Length of the arc.
		 * @return					A point on the arc closest to the provided ray.
		 */
		Vector3 calculateClosestPointOnArc(const Ray& inputRay, const Vector3& center, const Vector3& up,
			float radius, Degree startAngle, Degree angleAmount);

		/**
		 * Determines an angle of a point on a circle.
		 *
		 * @param[in]	up		Normal vector of the circle. Must be normalized.
		 * @param[in]	point	Point to try to find the angle for. Caller must ensure the point is actually somewhere on
		 *						the circle otherwise the result is undefined.
		 * @return				Angle at which the provided point lies on the circle.
		 */
		Degree pointOnCircleToAngle(Vector3 up, Vector3 point);

		static const float TORUS_RADIUS;

		Vector3 mNormal;
		float mRadius;
		bool mHasCutoffPlane;
		Plane mCutoffPlane;

		Vector3 mDirection;
		Vector3 mStartPosition;
		Degree mStartAngle;
		Degree mDelta;

		Torus mCollider;
	};
	
	/** @} */
}