//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsBounds.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsSphere.h"

namespace bs
{
	Bounds::Bounds(const AABox& box, const Sphere& sphere)
		:mBox(box), mSphere(sphere)
	{ }

	void Bounds::SetBounds(const AABox& box, const Sphere& sphere)
	{
		mBox = box;
		mSphere = sphere;
	}

	void Bounds::Merge(const Bounds& rhs)
	{
		mBox.Merge(rhs.mBox);
		mSphere.Merge(rhs.mSphere);
	}

	void Bounds::Merge(const Vector3& point)
	{
		mBox.Merge(point);
		mSphere.Merge(point);
	}

	void Bounds::Transform(const Matrix4& matrix)
	{
		mBox.Transform(matrix);
		mSphere.Transform(matrix);
	}

	void Bounds::TransformAffine(const Matrix4& matrix)
	{
		mBox.TransformAffine(matrix);
		mSphere.Transform(matrix);
	}
}

