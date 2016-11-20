//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTorus.h"
#include "BsRay.h"
#include "BsMath.h"
#include "BsDebug.h"

namespace bs
{
	Torus::Torus()
		:normal(BsZero), outerRadius(0.0f), innerRadius(0.0f)
	{ }

	Torus::Torus(const Vector3& normal, float outerRadius, float innerRadius)
		:normal(normal), outerRadius(outerRadius), innerRadius(innerRadius)
	{ }

	std::pair<bool, float> Torus::intersects(const Ray& ray) const
	{
		const Vector3& org = ray.getOrigin();
		const Vector3& dir = ray.getDirection();

		float u = normal.dot(org);
		float v = normal.dot(dir);

		float a = dir.dot(dir) - v * v;
		float b = 2 * (org.dot(dir) - u * v);
		float c = org.dot(org) - u * u;
		float d = org.dot(org) + outerRadius*outerRadius - innerRadius*innerRadius;

		float A = 1.0f;
		float B = 4 * org.dot(dir);
		float C = 2 * d + 0.25f * B * B - 4 * outerRadius * outerRadius * a;
		float D = B * d - 4 * outerRadius * outerRadius * b;
		float E = d * d - 4 * outerRadius * outerRadius * c;

		float roots[4];
		UINT32 numRoots = Math::solveQuartic(A, B, C, D, E, roots);

		if (numRoots > 0)
		{
			float nearestT = std::numeric_limits<float>::max();

			for (UINT32 i = 0; i < numRoots; i++)
			{
				float t = roots[i];
				if (t > 0 && t < nearestT)
					nearestT = t;
			}

			if (nearestT > std::numeric_limits<float>::epsilon())
				return std::make_pair(true, nearestT);
		}

		return std::make_pair(false, 0.0f); 
	}
}