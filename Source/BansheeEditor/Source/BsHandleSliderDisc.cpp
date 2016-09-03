//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHandleSliderDisc.h"
#include "BsHandleManager.h"
#include "BsHandleSliderManager.h"
#include "BsRay.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsCCamera.h"

namespace BansheeEngine
{
	const float HandleSliderDisc::TORUS_RADIUS = 0.1f;

	HandleSliderDisc::HandleSliderDisc(const Vector3& normal, float radius, bool fixedScale, UINT64 layer)
		: HandleSlider(fixedScale, layer), mNormal(normal), mRadius(radius), mHasCutoffPlane(false), mDirection(BsZero)
		, mStartPosition(BsZero), mDelta(0.0f)
	{
		mCollider = Torus(normal, radius, TORUS_RADIUS);

		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._registerSlider(this);
	}

	HandleSliderDisc::~HandleSliderDisc()
	{
		HandleSliderManager& sliderManager = HandleManager::instance().getSliderManager();
		sliderManager._unregisterSlider(this);
	}

	void HandleSliderDisc::setCutoffPlane(Degree angle, bool enabled)
	{
		mHasCutoffPlane = enabled;

		if (mHasCutoffPlane)
		{
			Vector3 up = mNormal;

			Quaternion alignWithStart = Quaternion(-Vector3::UNIT_Y, angle);
			Quaternion alignWithUp = Quaternion::getRotationFromTo(Vector3::UNIT_Y, up);

			Vector3 right = alignWithUp.rotate(alignWithStart.rotate(Vector3::UNIT_X));
			right.normalize();

			Vector3 planeNormal = right.cross(up);

			mCutoffPlane = Plane(planeNormal, 0.0f);
		}
	}

	bool HandleSliderDisc::intersects(const Ray& ray, float& t) const
	{
		Ray localRay = ray;
		localRay.transform(getTransformInv());

		auto intersect = mCollider.intersects(localRay);
		if (intersect.first)
		{
			t = intersect.second;

			if (mHasCutoffPlane)
			{
				auto cutoffIntersect = mCutoffPlane.intersects(localRay);
				if (cutoffIntersect.first && cutoffIntersect.second < t)
					return false;
			}

			return true;
		}

		return false;
	}

	Vector3 HandleSliderDisc::calculateClosestPointOnArc(const Ray& inputRay, const Vector3& center, const Vector3& up,
		float radius, Degree startAngle, Degree angleAmount)
	{
		Vector3 arcBasis[3];
		arcBasis[1] = up;
		arcBasis[1].orthogonalComplement(arcBasis[2], arcBasis[0]);

		Matrix4 worldToPlane = Matrix4::IDENTITY;
		worldToPlane.setColumn(0, (Vector4)arcBasis[0]);
		worldToPlane.setColumn(1, (Vector4)arcBasis[1]);
		worldToPlane.setColumn(2, (Vector4)arcBasis[2]);
		worldToPlane.setColumn(3, (Vector4)worldToPlane.multiplyAffine(-center));
		worldToPlane[3][3] = 1;

		Plane plane(up, (-center).dot(up));
		Vector3 pointOnPlane;

		auto intersectResult = plane.intersects(inputRay);

		if (intersectResult.first)
			pointOnPlane = inputRay.getPoint(intersectResult.second);
		else
			pointOnPlane = Vector3::ZERO;

		pointOnPlane = worldToPlane.multiplyAffine(pointOnPlane);
		Vector2 pointOnPlane2D(pointOnPlane.x, pointOnPlane.z); // y always 0

		Vector2 closestPoint2D;
		float dist = pointOnPlane2D.length();
		if (dist > 0.0f)
			closestPoint2D = mRadius * (pointOnPlane2D / dist);
		else
			closestPoint2D = Vector2(mRadius, 0);

		Vector2 negClosestPoint2D = -closestPoint2D;
		Radian angle = Math::atan2(negClosestPoint2D.y, negClosestPoint2D.x) + Radian(Math::PI);

		float angleRad = angle.valueRadians();
		float angleAmountRad = Math::clamp(angleAmount.valueRadians(), 0.0f, Math::PI * 2);

		float startAngleRad = startAngle.wrap().valueRadians();
		float endAngleRad = startAngleRad + angleAmountRad;

		float clampedAngle = angleRad;
		if (endAngleRad <= Math::PI * 2)
		{
			clampedAngle = Math::clamp(angleRad, startAngleRad, endAngleRad);
		}
		else
		{
			if (angleRad >= startAngleRad)
				clampedAngle = Math::clamp(angleRad, startAngleRad, Math::PI * 2);
			else
			{
				endAngleRad -= Math::PI * 2;
				if (angleRad > endAngleRad)
				{
					if ((startAngleRad - angleRad) > (angleRad - endAngleRad))
						clampedAngle = endAngleRad;
					else
						clampedAngle = startAngleRad;
				}
				else
					clampedAngle = angleRad;
			}
		}

		Vector3 clampedAnglePoint;
		clampedAnglePoint.x = Math::cos(clampedAngle) * radius;
		clampedAnglePoint.y = 0.0f;
		clampedAnglePoint.z = Math::sin(clampedAngle) * radius;

		return worldToPlane.inverseAffine().multiplyAffine(clampedAnglePoint);
	}

	Degree HandleSliderDisc::pointOnCircleToAngle(Vector3 up, Vector3 point)
	{
		Quaternion rot = Quaternion::getRotationFromTo(up, Vector3::UNIT_Y);

		Matrix4 worldToPlane = Matrix4::TRS(Vector3::ZERO, rot, Vector3::ONE);
		point = worldToPlane.multiplyAffine(point);

		Vector3 negPoint = -point;
		return Radian(Math::atan2(negPoint.z, negPoint.x) + Radian(Math::PI));
	}

	void HandleSliderDisc::activate(const SPtr<Camera>& camera, const Vector2I& pointerPos)
	{
		Ray localRay = camera->screenPointToRay(pointerPos);
		localRay.transformAffine(getTransformInv());

		mStartPosition = calculateClosestPointOnArc(localRay, Vector3::ZERO, mNormal, mRadius, Degree(0.0f), Degree(360.0f));
		mStartAngle = pointOnCircleToAngle(mNormal, mStartPosition);
		mStartPosition = getTransform().multiplyAffine(mStartPosition);

		Vector3 worldNormal = getTransform().multiplyDirection(mNormal);
		worldNormal.normalize();

		Vector3 toStart = mStartPosition - getPosition();
		mDirection = worldNormal.cross(toStart);
		mDirection.normalize();
	}

	void HandleSliderDisc::handleInput(const SPtr<Camera>& camera, const Vector2I& inputDelta)
	{
		assert(getState() == State::Active);

		mCurrentPointerPos += inputDelta;
		mDelta = calcDelta(camera, mStartPosition, mDirection, mStartPointerPos, mCurrentPointerPos) * Math::RAD2DEG;
	}
}