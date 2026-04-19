//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector2.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A line in 2D space represented with an origin and direction. */
	class B3D_EXPORT Line2
	{
	public:
		Line2() = default;

		Line2(const Vector2& origin, const Vector2& direction)
			: mOrigin(origin), mDirection(direction)
		{}

		void SetOrigin(const Vector2& origin) { mOrigin = origin; }

		const Vector2& GetOrigin(void) const { return mOrigin; }

		void SetDirection(const Vector2& dir) { mDirection = dir; }

		const Vector2& GetDirection(void) const { return mDirection; }

		/** Gets the position of a point t units along the line. */
		Vector2 GetPoint(float t) const
		{
			return Vector2(mOrigin + (mDirection * t));
		}

		/** Gets the position of a point t units along the line. */
		Vector2 operator*(float t) const
		{
			return GetPoint(t);
		}

		/** Line/Line intersection, returns boolean result and distance to intersection point. */
		std::pair<bool, float> Intersects(const Line2& line) const;

	protected:
		Vector2 mOrigin = Vector2::kZero;
		Vector2 mDirection = Vector2::kUnitX;
	};

	/** @} */
} // namespace b3d
