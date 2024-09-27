//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsTorus.h"
#include "Math/BsRay.h"
#include "Math/BsMath.h"
#include "Debug/BsDebug.h"

using namespace bs;

std::pair<bool, float> Torus::Intersects(const Ray& ray) const
{
	const Vector3& org = ray.Origin;
	const Vector3& dir = ray.Direction;

	float u = Normal.Dot(org);
	float v = Normal.Dot(dir);

	float a = dir.Dot(dir) - v * v;
	float b = 2 * (org.Dot(dir) - u * v);
	float c = org.Dot(org) - u * u;
	float d = org.Dot(org) + OuterRadius * OuterRadius - InnerRadius * InnerRadius;

	float A = 1.0f;
	float B = 4 * org.Dot(dir);
	float C = 2 * d + 0.25f * B * B - 4 * OuterRadius * OuterRadius * a;
	float D = B * d - 4 * OuterRadius * OuterRadius * b;
	float E = d * d - 4 * OuterRadius * OuterRadius * c;

	float roots[4];
	u32 numRoots = Math::SolveQuartic(A, B, C, D, E, roots);

	if(numRoots > 0)
	{
		float nearestT = std::numeric_limits<float>::max();

		for(u32 i = 0; i < numRoots; i++)
		{
			float t = roots[i];
			if(t > 0 && t < nearestT)
				nearestT = t;
		}

		if(nearestT > std::numeric_limits<float>::epsilon())
			return std::make_pair(true, nearestT);
	}

	return std::make_pair(false, 0.0f);
}
