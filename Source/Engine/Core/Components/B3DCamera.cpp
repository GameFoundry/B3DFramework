//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DCamera.h"
#include "B3DApplication.h"
#include "RTTI/B3DCameraRTTI.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneInstance.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "CoreObject/B3DRenderThread.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Renderer/B3DRenderSettings.h"
#include "Renderer/B3DRenderSettings.implementation.h"

using namespace b3d;

template<bool IsRenderProxy>
const float TCamera<IsRenderProxy>::kInfiniteFarPlaneAdjust = 0.00001f;

template <bool IsRenderProxy>
TCamera<IsRenderProxy>::TCamera()
	:Super(), mRecalcView(true), mRecalcFrustum(true), mRecalcFrustumPlanes(true)
{
	mRenderSettings = B3DMakeShared<RenderSettingsType>();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetFlags(CameraFlags flags)
{
	mCameraFlags = flags;
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetHorizontalFOV(const Radian& fov)
{
	mHorzFOV = fov;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetFarClipDistance(float farPlane)
{
	mFarDist = farPlane;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetNearClipDistance(float nearPlane)
{
	if(nearPlane <= 0)
	{
		B3D_LOG(Error, Renderer, "Near clip distance must be greater than zero.");
		return;
	}

	mNearDist = nearPlane;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
const Matrix4& TCamera<IsRenderProxy>::GetUnadjustedProjectionMatrix() const
{
	UpdateFrustum();

	return mProjMatrix;
}

template <bool IsRenderProxy>
const Matrix4& TCamera<IsRenderProxy>::GetProjectionMatrix() const
{
	UpdateFrustum();

	return mProjMatrixRS;
}

template <bool IsRenderProxy>
const Matrix4& TCamera<IsRenderProxy>::GetViewMatrix() const
{
	if(!mCustomViewMatrix && mRecalcView)
	{
		const Transform& transform = GetTransform();

		mViewMatrix.MakeView(transform.GetPosition(), transform.GetRotation());
		mRecalcView = false;
	}

	return mViewMatrix;
}

template <bool IsRenderProxy>
const ConvexVolume& TCamera<IsRenderProxy>::GetFrustum() const
{
	// Make any pending updates to the calculated frustum planes
	UpdateFrustumPlanes();

	return mFrustum;
}

template <bool IsRenderProxy>
ConvexVolume TCamera<IsRenderProxy>::GetWorldFrustum() const
{
	const Vector<Plane>& frustumPlanes = GetFrustum().GetPlanes();
	const Transform& transform = GetTransform();

	Matrix4 worldMatrix;
	worldMatrix.SetTrs(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	Vector<Plane> worldPlanes(frustumPlanes.size());
	u32 planeIndex = 0;
	for(const auto& plane : frustumPlanes)
	{
		worldPlanes[planeIndex] = worldMatrix.MultiplyAffine(plane);
		planeIndex++;
	}

	return ConvexVolume(worldPlanes);
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::CalculateProjectionParameters(float& left, float& right, float& bottom, float& top) const
{
	if(mCustomProjMatrix)
	{
		// Convert clipspace corners to camera space
		Matrix4 invProj = mProjMatrix.Inverse();
		Vector3 topLeft(-0.5f, 0.5f, 0.0f);
		Vector3 bottomRight(0.5f, -0.5f, 0.0f);

		topLeft = invProj.Multiply(topLeft);
		bottomRight = invProj.Multiply(bottomRight);

		left = topLeft.X;
		top = topLeft.Y;
		right = bottomRight.X;
		bottom = bottomRight.Y;
	}
	else
	{
		if(mFrustumExtentsManuallySet)
		{
			left = mLeft;
			right = mRight;
			top = mTop;
			bottom = mBottom;
		}
		else if(mProjType == PT_PERSPECTIVE)
		{
			Radian thetaX(mHorzFOV * 0.5f);
			float tanThetaX = Math::Tan(thetaX);
			float tanThetaY = tanThetaX / mAspect;

			float halfWidth = tanThetaX * mNearDist;
			float halfHeight = tanThetaY * mNearDist;

			left = -halfWidth;
			right = halfWidth;
			bottom = -halfHeight;
			top = halfHeight;

			mLeft = left;
			mRight = right;
			mTop = top;
			mBottom = bottom;
		}
		else
		{
			float halfWidth = GetOrthographicWidth() * 0.5f;
			float halfHeight = GetOrthographicHeight() * 0.5f;

			left = -halfWidth;
			right = halfWidth;
			bottom = -halfHeight;
			top = halfHeight;

			mLeft = left;
			mRight = right;
			mTop = top;
			mBottom = bottom;
		}
	}
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::UpdateFrustum() const
{
	if(IsFrustumOutOfDate())
	{
		float left, right, bottom, top;

		CalculateProjectionParameters(left, right, bottom, top);

		if(!mCustomProjMatrix)
		{
			float inverseWidth = 1 / (right - left);
			float inverseHeight = 1 / (top - bottom);
			float inverseDepth = 1 / (mFarDist - mNearDist);

			if(mProjType == PT_PERSPECTIVE)
			{
				float A = 2 * mNearDist * inverseWidth;
				float B = 2 * mNearDist * inverseHeight;
				float C = (right + left) * inverseWidth;
				float D = (top + bottom) * inverseHeight;
				float q, qn;

				if(mFarDist == 0)
				{
					// Infinite far plane
					q = kInfiniteFarPlaneAdjust - 1;
					qn = mNearDist * (kInfiniteFarPlaneAdjust - 2);
				}
				else
				{
					q = -(mFarDist + mNearDist) * inverseDepth;
					qn = -2 * (mFarDist * mNearDist) * inverseDepth;
				}

				mProjMatrix = Matrix4::kZero;
				mProjMatrix[0][0] = A;
				mProjMatrix[0][2] = C;
				mProjMatrix[1][1] = B;
				mProjMatrix[1][2] = D;
				mProjMatrix[2][2] = q;
				mProjMatrix[2][3] = qn;
				mProjMatrix[3][2] = -1;
			}
			else if(mProjType == PT_ORTHOGRAPHIC)
			{
				float A = 2 * inverseWidth;
				float B = 2 * inverseHeight;
				float C = -(right + left) * inverseWidth;
				float D = -(top + bottom) * inverseHeight;
				float q, qn;

				if(mFarDist == 0)
				{
					// Can not do infinite far plane here, avoid divided zero only
					q = -kInfiniteFarPlaneAdjust / mNearDist;
					qn = -kInfiniteFarPlaneAdjust - 1;
				}
				else
				{
					q = -2 * inverseDepth;
					qn = -(mFarDist + mNearDist) * inverseDepth;
				}

				mProjMatrix = Matrix4::kZero;
				mProjMatrix[0][0] = A;
				mProjMatrix[0][3] = C;
				mProjMatrix[1][1] = B;
				mProjMatrix[1][3] = D;
				mProjMatrix[2][2] = q;
				mProjMatrix[2][3] = qn;
				mProjMatrix[3][3] = 1;
			}
		}

		if (const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice())
			gpuDevice->ConvertProjectionMatrix(mProjMatrix, mProjMatrixRS);
		else
			mProjMatrixRS = mProjMatrix;

		// Calculate bounding box (local)
		// Box is from 0, down -Z, max dimensions as determined from far plane
		// If infinite view frustum just pick a far value
		float farDist = (mFarDist == 0) ? 100000 : mFarDist;

		// Near plane bounds
		Vector3 min(left, bottom, -farDist);
		Vector3 max(right, top, 0);

		if(mCustomProjMatrix)
		{
			// Some custom projection matrices can have unusual inverted settings
			// So make sure the AABB is the right way around to start with
			Vector3 tmp = min;
			min.Min(max);
			max.Max(tmp);
		}

		if(mProjType == PT_PERSPECTIVE)
		{
			// Merge with far plane bounds
			float ratio = farDist / mNearDist;
			min.Min(Vector3(left * ratio, bottom * ratio, -farDist));
			max.Max(Vector3(right * ratio, top * ratio, 0));
		}

		mBoundingBox.SetExtents(min, max);

		mRecalcFrustum = false;
		mRecalcFrustumPlanes = true;
	}
}

template <bool IsRenderProxy>
bool TCamera<IsRenderProxy>::IsFrustumOutOfDate() const
{
	return mRecalcFrustum;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::UpdateFrustumPlanes() const
{
	UpdateFrustum();

	if(mRecalcFrustumPlanes)
	{
		mFrustum = ConvexVolume(mProjMatrix);
		mRecalcFrustumPlanes = false;
	}
}

template <bool IsRenderProxy>
float TCamera<IsRenderProxy>::GetAspectRatio() const
{
	return mAspect;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetAspectRatio(float ratio)
{
	mAspect = ratio;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
const AABox& TCamera<IsRenderProxy>::GetBoundingBox() const
{
	UpdateFrustum();

	return mBoundingBox;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetProjectionType(ProjectionType pt)
{
	mProjType = pt;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
ProjectionType TCamera<IsRenderProxy>::GetProjectionType() const
{
	return mProjType;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetCustomProjectionMatrix(bool enable, const Matrix4& projMatrix)
{
	mCustomProjMatrix = enable;

	if(enable)
		mProjMatrix = projMatrix;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetOrthographicSize(float width, float height)
{
	mOrthoHeight = height;
	mAspect = width / height;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetOrthographicHeight(float height)
{
	mOrthoHeight = height;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetOrthographicWidth(float width)
{
	mOrthoHeight = width / mAspect;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
float TCamera<IsRenderProxy>::GetOrthographicHeight() const
{
	return mOrthoHeight;
}

template <bool IsRenderProxy>
float TCamera<IsRenderProxy>::GetOrthographicWidth() const
{
	return mOrthoHeight * mAspect;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetFrustumExtents(float left, float right, float top, float bottom)
{
	mFrustumExtentsManuallySet = true;
	mLeft = left;
	mRight = right;
	mTop = top;
	mBottom = bottom;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::ResetFrustumExtents()
{
	mFrustumExtentsManuallySet = false;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::GetFrustumExtents(float& outLeft, float& outRight, float& outTop, float& outBottom) const
{
	UpdateFrustum();

	outLeft = mLeft;
	outRight = mRight;
	outTop = mTop;
	outBottom = mBottom;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::InvalidateFrustum() const
{
	mRecalcFrustum = true;
	mRecalcFrustumPlanes = true;
}

template <bool IsRenderProxy>
Area2I TCamera<IsRenderProxy>::GetViewportArea() const
{
	return mViewport->GetPixelArea();
}

template<bool IsRenderProxy>
Vector2I TCamera<IsRenderProxy>::WorldToScreenPoint(const Vector3& worldPoint) const
{
	Vector2 ndcPoint = WorldToNDCPoint(worldPoint);
	return NDCToScreenPoint(ndcPoint);
}

template<bool IsRenderProxy>
Vector2 TCamera<IsRenderProxy>::WorldToNDCPoint(const Vector3& worldPoint) const
{
	Vector3 viewPoint = WorldToViewPoint(worldPoint);
	return ViewToNDCPoint(viewPoint);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::WorldToViewPoint(const Vector3& worldPoint) const
{
	return GetViewMatrix().MultiplyAffine(worldPoint);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ScreenToWorldPoint(const Vector2I& screenPoint, float depth) const
{
	Vector2 ndcPoint = ScreenToNDCPoint(screenPoint);
	return NDCToWorldPoint(ndcPoint, depth);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ScreenToWorldPointDeviceDepth(const Vector2I& screenPoint, float deviceDepth) const
{
	Vector2 ndcPoint = ScreenToNDCPoint(screenPoint);
	Vector4 worldPoint(ndcPoint.X, ndcPoint.Y, deviceDepth, 1.0f);
	worldPoint = GetProjectionMatrix().Inverse().Multiply(worldPoint);

	Vector3 worldPoint3D;
	if(Math::Abs(worldPoint.W) > 1e-7f)
	{
		float inverseW = 1.0f / worldPoint.W;

		worldPoint3D.X = worldPoint.X * inverseW;
		worldPoint3D.Y = worldPoint.Y * inverseW;
		worldPoint3D.Z = worldPoint.Z * inverseW;
	}

	return ViewToWorldPoint(worldPoint3D);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ScreenToViewPoint(const Vector2I& screenPoint, float depth) const
{
	Vector2 ndcPoint = ScreenToNDCPoint(screenPoint);
	return NDCToViewPoint(ndcPoint, depth);
}

template<bool IsRenderProxy>
Vector2 TCamera<IsRenderProxy>::ScreenToNDCPoint(const Vector2I& screenPoint) const
{
	Area2I viewport = GetViewportArea();

	Vector2 ndcPoint;
	ndcPoint.X = (float)(((screenPoint.X - viewport.X) / (float)viewport.Width) * 2.0f - 1.0f);

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	if(gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down)
		ndcPoint.Y = (float)(((screenPoint.Y - viewport.Y) / (float)viewport.Height) * 2.0f - 1.0f);
	else
		ndcPoint.Y = (float)((1.0f - ((screenPoint.Y - viewport.Y) / (float)viewport.Height)) * 2.0f - 1.0f);

	return ndcPoint;
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ViewToWorldPoint(const Vector3& viewPoint) const
{
	return GetViewMatrix().InverseAffine().MultiplyAffine(viewPoint);
}

template<bool IsRenderProxy>
Vector2I TCamera<IsRenderProxy>::ViewToScreenPoint(const Vector3& viewPoint) const
{
	Vector2 ndcPoint = ViewToNDCPoint(viewPoint);
	return NDCToScreenPoint(ndcPoint);
}

template<bool IsRenderProxy>
Vector2 TCamera<IsRenderProxy>::ViewToNDCPoint(const Vector3& viewPoint) const
{
	Vector3 projPoint = ProjectPoint(viewPoint);

	return Vector2(projPoint.X, projPoint.Y);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::NDCToWorldPoint(const Vector2& ndcPoint, float depth) const
{
	Vector3 viewPoint = NDCToViewPoint(ndcPoint, depth);
	return ViewToWorldPoint(viewPoint);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::NDCToViewPoint(const Vector2& ndcPoint, float depth) const
{
	return UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, depth));
}

template<bool IsRenderProxy>
Vector2I TCamera<IsRenderProxy>::NDCToScreenPoint(const Vector2& ndcPoint) const
{
	Area2I viewport = GetViewportArea();

	Vector2I screenPoint;
	screenPoint.X = Math::RoundToI32(viewport.X + ((ndcPoint.X + 1.0f) * 0.5f) * viewport.Width);

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	if(gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down)
		screenPoint.Y = Math::RoundToI32(viewport.Y + (ndcPoint.Y + 1.0f) * 0.5f * viewport.Height);
	else
		screenPoint.Y = Math::RoundToI32(viewport.Y + (1.0f - (ndcPoint.Y + 1.0f) * 0.5f) * viewport.Height);

	return screenPoint;
}

template<bool IsRenderProxy>
Ray TCamera<IsRenderProxy>::ScreenPointToRay(const Vector2I& screenPoint) const
{
	Vector2 ndcPoint = ScreenToNDCPoint(screenPoint);

	Vector3 near = UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, mNearDist));
	Vector3 far = UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, mNearDist + 1.0f));

	Ray ray(near, Vector3::Normalize(far - near));
	ray.TransformAffine(GetViewMatrix().InverseAffine());

	return ray;
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ProjectPoint(const Vector3& point) const
{
	Vector4 projectedPoint4(point.X, point.Y, point.Z, 1.0f);
	projectedPoint4 = GetProjectionMatrix().Multiply(projectedPoint4);

	if(Math::Abs(projectedPoint4.W) > 1e-7f)
	{
		float inverseW = 1.0f / projectedPoint4.W;
		projectedPoint4.X *= inverseW;
		projectedPoint4.Y *= inverseW;
		projectedPoint4.Z *= inverseW;
	}
	else
	{
		projectedPoint4.X = 0.0f;
		projectedPoint4.Y = 0.0f;
		projectedPoint4.Z = 0.0f;
	}

	return Vector3(projectedPoint4.X, projectedPoint4.Y, projectedPoint4.Z);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::UnprojectPoint(const Vector3& point) const
{
	// Point.z is expected to be in view space, so we need to do some extra work to get the proper coordinates
	// (as opposed to if point.z was in device coordinates, in which case we could just inverse project)

	// Get world position for a point near the far plane (0.95f)
	Vector4 farAwayPoint(point.X, point.Y, 0.95f, 1.0f);
	farAwayPoint = GetProjectionMatrix().Inverse().Multiply(farAwayPoint);

	// Can't proceed if w is too small
	if(Math::Abs(farAwayPoint.W) > 1e-7f)
	{
		// Perspective divide, to get the values that make sense in 3D space
		float inverseW = 1.0f / farAwayPoint.W;

		Vector3 farAwayPoint3D;
		farAwayPoint3D.X = farAwayPoint.X * inverseW;
		farAwayPoint3D.Y = farAwayPoint.Y * inverseW;
		farAwayPoint3D.Z = farAwayPoint.Z * inverseW;

		// Find the distance to the far point along the camera's viewing axis
		float distAlongZ = farAwayPoint3D.Dot(-Vector3::kUnitZ);

		// Do nothing if point is behind the camera
		if(distAlongZ >= 0.0f)
		{
			if(mProjType == PT_PERSPECTIVE)
			{
				// Direction from origin to our point
				Vector3 dir = farAwayPoint3D; // Camera is at (0, 0, 0) so it's the same vector

				// Our view space depth (point.z) is distance along the camera's viewing axis. Since our direction
				// vector is not parallel to the viewing axis, instead of normalizing it with its own length, we
				// "normalize" with the length projected along the camera's viewing axis.
				dir /= distAlongZ;

				// And now we just find the final position along the direction
				return dir * point.Z;
			}
			else // Ortographic
			{
				// Depth difference between our arbitrary point and actual depth
				float depthDiff = distAlongZ - point.Z;

				// Depth difference along viewing direction
				Vector3 depthDiffVec = depthDiff * -Vector3::kUnitZ;

				// Return point that is depthDiff closer than our arbitrary point
				return farAwayPoint3D - depthDiffVec;
			}
		}
	}

	return Vector3(0.0f, 0.0f, 0.0f);
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetCustomViewMatrix(bool enable, const Matrix4& viewMatrix)
{
	mCustomViewMatrix = enable;
	if(enable)
	{
		mViewMatrix = viewMatrix;
	}

	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
const Transform& TCamera<IsRenderProxy>::GetTransform() const
{
	if constexpr(!IsRenderProxy)
		return static_cast<const Camera*>(this)->SceneObject()->GetTransform();
	else
		return static_cast<const render::Camera*>(this)->GetWorldTransform();
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if constexpr(!IsRenderProxy)
		CoreObject::MarkRenderProxyDataDirty((u32)flag);
}

template class TCamera<false>;
template class TCamera<true>;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Camera, FullSyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mLayers)
		B3D_SYNC_BLOCK_ENTRY(mProjType)
		B3D_SYNC_BLOCK_ENTRY(mHorzFOV)
		B3D_SYNC_BLOCK_ENTRY(mFarDist)
		B3D_SYNC_BLOCK_ENTRY(mNearDist)
		B3D_SYNC_BLOCK_ENTRY(mAspect)
		B3D_SYNC_BLOCK_ENTRY(mOrthoHeight)
		B3D_SYNC_BLOCK_ENTRY(mPriority)
		B3D_SYNC_BLOCK_ENTRY(mCustomViewMatrix)
		B3D_SYNC_BLOCK_ENTRY(mCustomProjMatrix)
		B3D_SYNC_BLOCK_ENTRY(mFrustumExtentsManuallySet)
		B3D_SYNC_BLOCK_ENTRY(mMSAA)
		B3D_SYNC_BLOCK_ENTRY(mMain)
		B3D_SYNC_BLOCK_ENTRY(mCameraFlags)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(RenderProxySyncPacket*, RenderSettingsPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Camera, RedrawSyncPacket)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Camera, TransformSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END
}


Camera::Camera(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Camera");
	mNotifyFlags = TCF_Transform;

	mViewport = Viewport::Create(nullptr);
}

Camera::Camera()
	: Camera(nullptr)
{ }

void Camera::SetMain(bool main)
{
	mMain = main;

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->NotifyMainCameraStateChanged(B3DStaticGameObjectCast<Camera>(GetHandle()));
}

TAsyncOp<SPtr<PixelData>> Camera::RequestCapture()
{
	SPtr<Viewport> viewport = GetViewport();
	if (viewport == nullptr || viewport->GetTarget() == nullptr)
	{
		B3D_LOG(Warning, Renderer, "RequestCapture called on camera with no viewport");
		TAsyncOp<SPtr<PixelData>> op;
		op.CompleteOperation(nullptr);
		return op;
	}

	TAsyncOp<SPtr<PixelData>> asyncOp;
	SPtr<render::Camera> renderCamera = B3DGetRenderProxy(this);

	auto fnRequestCapture = [renderCamera, asyncOp]() mutable
	{
		SPtr<render::Renderer> renderer = render::GetRenderer();
		if (renderer != nullptr)
			renderer->RequestScreenCapture(renderCamera.get(), asyncOp);
		else
			asyncOp.CompleteOperation(nullptr);
	};

	GetRenderThread().PostCommand(fnRequestCapture, "Camera::RequestCapture", false, GetName());
	return asyncOp;
}

void Camera::Initialize()
{
	SetShared(B3DStaticGameObjectCast<Camera>(mThisHandle).GetShared());

	Component::Initialize();
	CoreObject::Initialize();
}

void Camera::OnCreated()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->RegisterCamera(B3DStaticGameObjectCast<Camera>(GetHandle()));
}

void Camera::OnBeginPlay()
{
	// Make sure primary RT gets applied if camera gets deserialized with main camera state
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->NotifyMainCameraStateChanged(B3DStaticGameObjectCast<Camera>(GetHandle()));
}

void Camera::OnEnabled()
{
	MarkRenderProxyDataDirty();
	
}

void Camera::OnDisabled()
{
	MarkRenderProxyDataDirty();
}

void Camera::OnDestroyed()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->UnregisterCamera(B3DStaticGameObjectCast<Camera>(GetHandle()));

	CoreObject::Destroy();
}

void Camera::OnTransformChanged(TransformChangedFlags flags)
{
	mRecalcView = true;
	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
}

SPtr<render::RenderProxy> Camera::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	render::Camera* renderProxy = new(B3DAllocate<render::Camera>()) render::Camera(B3DGetRenderProxy(scene), B3DGetRenderProxy(mViewport));
	SPtr<render::Camera> renderProxyShared = B3DMakeSharedFromExisting<render::Camera>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* Camera::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(flags == 0)
		return nullptr;

	if((flags & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		if(flags != (u32)ComponentDirtyFlag::Transform)
		{
			FullSyncPacket* const syncPacket = allocator.Construct<FullSyncPacket>(*this, allocator, flags);
			syncPacket->RenderSettingsPacket = allocator.Construct<RenderSettings::SyncPacket>(*mRenderSettings, allocator, flags);
			syncPacket->mActive = GetEnabled();
			syncPacket->mSceneInstance = B3DGetRenderProxy(SceneObject()->GetScene());
			syncPacket->mTransform = SceneObject()->GetTransform();

			return syncPacket;
		}

		TransformSyncPacket* const transformSyncPacket = allocator.Construct<TransformSyncPacket>(*this, allocator, flags);
		transformSyncPacket->mTransform = SceneObject()->GetTransform();

		return transformSyncPacket;
	}

	// Redraw only
	return allocator.Construct<RedrawSyncPacket>(*this, allocator, flags);
}

void Camera::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	dependencies.push_back(mViewport.get());
}

RTTIType* Camera::GetRttiStatic()
{
	return CameraRTTI::Instance();
}

RTTIType* Camera::GetRtti() const
{
	return Camera::GetRttiStatic();
}

namespace b3d { namespace render
{
Camera::Camera(const SPtr<SceneInstance>& scene, const SPtr<RenderTarget>& target, float left, float top, float width, float height)
	: mRendererId(0), mSceneInstance(scene)
{
	mViewport = Viewport::Create(target, left, top, width, height);
}

Camera::Camera(const SPtr<SceneInstance>& scene, const SPtr<Viewport>& viewport)
	: mRendererId(0), mSceneInstance(scene)
{
	mViewport = viewport;
}

Camera::~Camera()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UnregisterCamera(this);
}

void Camera::Initialize()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterCamera(this);

	RenderProxy::Initialize();
}

void Camera::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<RenderProxySyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);

	if((syncPacket->Flags & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		if(syncPacket->Flags != (u32)ComponentDirtyFlag::Transform)
		{
			auto* const fullSyncPacket = static_cast<b3d::Camera::FullSyncPacket*>(syncPacket);
			fullSyncPacket->RenderSettingsPacket->ApplySyncData(mRenderSettings.get());

			allocator.Destruct(fullSyncPacket->RenderSettingsPacket);
			fullSyncPacket->RenderSettingsPacket = nullptr;
		}

		mRecalcFrustum = true;
		mRecalcFrustumPlanes = true;
		mRecalcView = true;
	}

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UpdateCamera(this, (u32)syncPacket->Flags);
}
}}
