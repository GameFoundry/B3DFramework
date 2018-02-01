//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Handles/BsHandleSliderSphere.h"
#include "Handles/BsHandleManager.h"
#include "Handles/BsHandleSliderManager.h"
#include "Math/BsSphere.h"
#include "Math/BsRay.h"

namespace bs
{
	HandleSliderSphere::HandleSliderSphere(float radius, bool fixedScale, UINT64 layer)
		: HandleSlider(fixedScale, layer)
	{
		mSphereCollider = Sphere(Vector3::ZERO, radius);

		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._registerSlider(this);
	}

	HandleSliderSphere::~HandleSliderSphere()
	{
		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._unregisterSlider(this);
	}

	bool HandleSliderSphere::intersects(const Vector2I& screenPos, const Ray& ray, float& t) const
	{
		Ray localRay = ray;
		localRay.transformAffine(getTransformInv());

		auto intersect = mSphereCollider.intersects(localRay);

		t = std::numeric_limits<float>::max();

		if (intersect.first)
		{
			Vector3 intrPoint = localRay.getPoint(t);
			intrPoint = getTransform().multiplyAffine(intrPoint);
			t = (intrPoint - ray.getOrigin()).length(); // Get distance in world space

			return true;
		}

		return false;
	}

	void HandleSliderSphere::handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta)
	{
		// Do nothing
	}
}
