//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHandleSliderPlane.h"
#include "BsHandleManager.h"
#include "BsHandleSliderManager.h"
#include "BsVector3.h"
#include "BsRay.h"
#include "BsPlane.h"
#include "BsCamera.h"

namespace bs
{
	HandleSliderPlane::HandleSliderPlane(const Vector3& dir1, const Vector3& dir2, float length, bool fixedScale, UINT64 layer)
		: HandleSlider(fixedScale, layer), mDirection1(Vector3::normalize(dir1)), mLength(length)
		, mDirection2(Vector3::normalize(dir2)), mDelta(BsZero), mStartPlanePosition(BsZero), mStartClickPosition(BsZero)
	{
		float halfLength = length * 0.5f;
		std::array<Vector3, 2> axes = {{ mDirection1, mDirection2 }};
		std::array<float, 2> extents = {{ halfLength, halfLength }};

		Vector3 center = (dir1 * length + dir2 * length) * 0.5f;
		mCollider = Rect3(center, axes, extents);

		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._registerSlider(this);
	}

	HandleSliderPlane::~HandleSliderPlane()
	{
		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._unregisterSlider(this);
	}

	bool HandleSliderPlane::intersects(const Ray& ray, float& t) const
	{
		Ray localRay = ray;
		localRay.transform(getTransformInv());

		auto intersect = mCollider.intersects(localRay);

		if (intersect.first)
		{
			t = intersect.second;

			return true;
		}

		return false;
	}

	void HandleSliderPlane::activate(const SPtr<Camera>& camera, const Vector2I& pointerPos)
	{
		mStartPlanePosition = getPosition();
		mStartClickPosition = getPositionOnPlane(camera, pointerPos);
	}

	void HandleSliderPlane::handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta)
	{
		assert(getState() == State::Active);

		mCurrentPointerPos += inputDelta;

		Vector3 worldDir1 = getRotation().rotate(mDirection1);
		Vector3 worldDir2 = getRotation().rotate(mDirection2);

		Vector3 intersectPosition = getPositionOnPlane(camera, mCurrentPointerPos);
		Vector3 positionDelta = intersectPosition - mStartClickPosition;

		mDelta.x = positionDelta.dot(worldDir1);
		mDelta.y = positionDelta.dot(worldDir2);
	}

	Vector3 HandleSliderPlane::getPositionOnPlane(const SPtr<Camera>& camera, const Vector2I& pointerPos) const
	{
		Vector3 worldDir1 = getRotation().rotate(mDirection1);
		Vector3 worldDir2 = getRotation().rotate(mDirection2);

		Vector3 normal = worldDir1.cross(worldDir2);
		float dot = normal.dot(camera->getForward());
		if (dot > 0)
			normal = -normal;

		Plane plane(normal, mStartPlanePosition);

		Ray clickRay = camera->screenPointToRay(pointerPos);
		auto intersectResult = plane.intersects(clickRay);
		if (intersectResult.first)
			return clickRay.getPoint(intersectResult.second);

		return mStartClickPosition;
	}
}