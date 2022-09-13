//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsLine2.h"
#include "Math/BsMath.h"

namespace bs
{
	std::pair<bool, float> Line2::Intersects(const Line2& rhs) const
	{
		Vector2 diff = rhs.GetOrigin() - GetOrigin();
		Vector2 perpDir = rhs.GetDirection();
		perpDir = Vector2(perpDir.y, -perpDir.x);

		float dot = GetDirection().Dot(perpDir);
		if (std::abs(dot) > 1.0e-4f) // Not parallel
		{
			float distance = diff.Dot(perpDir) / dot;

			return std::make_pair(true, distance);
		}
		else // Parallel
			return std::make_pair(true, 0.0f);
	}
}
