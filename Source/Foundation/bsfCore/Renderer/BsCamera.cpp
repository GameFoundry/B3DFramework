//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsCamera.h"

#include "BsCoreApplication.h"
#include "BsRendererScene.h"
#include "Private/RTTI/BsCameraRTTI.h"
#include "Math/BsMath.h"
#include "Math/BsMatrix3.h"
#include "Math/BsVector2.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Error/BsException.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Scene/BsSceneManager.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "RenderAPI/BsGpuDevice.h"
#include "RenderAPI/BsGpuDeviceCapabilities.h"
#include "BsRenderSettings.implementation.h"

using namespace b3d;

const float CameraBase::kInfiniteFarPlaneAdjust = 0.00001f;

CameraBase::CameraBase()
	: mRecalcFrustum(true), mRecalcFrustumPlanes(true)
{
	InvalidateFrustum();
}

void CameraBase::SetFlags(CameraFlags flags)
{
	mCameraFlags = flags;
	MarkRenderProxyDataDirty();
}

void CameraBase::SetHorzFov(const Radian& fov)
{
	mHorzFOV = fov;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::SetFarClipDistance(float farPlane)
{
	mFarDist = farPlane;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::SetNearClipDistance(float nearPlane)
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

const Matrix4& CameraBase::GetProjectionMatrix() const
{
	UpdateFrustum();

	return mProjMatrix;
}

const Matrix4& CameraBase::GetProjectionMatrixInv() const
{
	UpdateFrustum();

	return mProjMatrixInv;
}

const Matrix4& CameraBase::GetProjectionMatrixRs() const
{
	UpdateFrustum();

	return mProjMatrixRS;
}

const Matrix4& CameraBase::GetProjectionMatrixRsInv() const
{
	UpdateFrustum();

	return mProjMatrixRSInv;
}

const ConvexVolume& CameraBase::GetFrustum() const
{
	// Make any pending updates to the calculated frustum planes
	UpdateFrustumPlanes();

	return mFrustum;
}

void CameraBase::CalcProjectionParameters(float& left, float& right, float& bottom, float& top) const
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

			float half_w = tanThetaX * mNearDist;
			float half_h = tanThetaY * mNearDist;

			left = -half_w;
			right = half_w;
			bottom = -half_h;
			top = half_h;

			mLeft = left;
			mRight = right;
			mTop = top;
			mBottom = bottom;
		}
		else
		{
			float half_w = GetOrthoWindowWidth() * 0.5f;
			float half_h = GetOrthoWindowHeight() * 0.5f;

			left = -half_w;
			right = half_w;
			bottom = -half_h;
			top = half_h;

			mLeft = left;
			mRight = right;
			mTop = top;
			mBottom = bottom;
		}
	}
}

void CameraBase::UpdateFrustum() const
{
	if(IsFrustumOutOfDate())
	{
		float left, right, bottom, top;

		CalcProjectionParameters(left, right, bottom, top);

		if(!mCustomProjMatrix)
		{
			float inv_w = 1 / (right - left);
			float inv_h = 1 / (top - bottom);
			float inv_d = 1 / (mFarDist - mNearDist);

			if(mProjType == PT_PERSPECTIVE)
			{
				float A = 2 * mNearDist * inv_w;
				float B = 2 * mNearDist * inv_h;
				float C = (right + left) * inv_w;
				float D = (top + bottom) * inv_h;
				float q, qn;

				if(mFarDist == 0)
				{
					// Infinite far plane
					q = CameraBase::kInfiniteFarPlaneAdjust - 1;
					qn = mNearDist * (CameraBase::kInfiniteFarPlaneAdjust - 2);
				}
				else
				{
					q = -(mFarDist + mNearDist) * inv_d;
					qn = -2 * (mFarDist * mNearDist) * inv_d;
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
				float A = 2 * inv_w;
				float B = 2 * inv_h;
				float C = -(right + left) * inv_w;
				float D = -(top + bottom) * inv_h;
				float q, qn;

				if(mFarDist == 0)
				{
					// Can not do infinite far plane here, avoid divided zero only
					q = -CameraBase::kInfiniteFarPlaneAdjust / mNearDist;
					qn = -CameraBase::kInfiniteFarPlaneAdjust - 1;
				}
				else
				{
					q = -2 * inv_d;
					qn = -(mFarDist + mNearDist) * inv_d;
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

		if (const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice())
			gpuDevice->ConvertProjectionMatrix(mProjMatrix, mProjMatrixRS);
		else
			mProjMatrixRS = mProjMatrix;

		mProjMatrixInv = mProjMatrix.Inverse();
		mProjMatrixRSInv = mProjMatrixRS.Inverse();

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
			float radio = farDist / mNearDist;
			min.Min(Vector3(left * radio, bottom * radio, -farDist));
			max.Max(Vector3(right * radio, top * radio, 0));
		}

		mBoundingBox.SetExtents(min, max);

		mRecalcFrustum = false;
		mRecalcFrustumPlanes = true;
	}
}

bool CameraBase::IsFrustumOutOfDate() const
{
	return mRecalcFrustum;
}

void CameraBase::UpdateFrustumPlanes() const
{
	UpdateFrustum();

	if(mRecalcFrustumPlanes)
	{
		mFrustum = ConvexVolume(mProjMatrix);
		mRecalcFrustumPlanes = false;
	}
}

float CameraBase::GetAspectRatio() const
{
	return mAspect;
}

void CameraBase::SetAspectRatio(float r)
{
	mAspect = r;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

const AABox& CameraBase::GetBoundingBox() const
{
	UpdateFrustum();

	return mBoundingBox;
}

void CameraBase::SetProjectionType(ProjectionType pt)
{
	mProjType = pt;
	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

ProjectionType CameraBase::GetProjectionType() const
{
	return mProjType;
}

void CameraBase::SetCustomProjectionMatrix(bool enable, const Matrix4& projMatrix)
{
	mCustomProjMatrix = enable;

	if(enable)
		mProjMatrix = projMatrix;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::SetOrthoWindow(float w, float h)
{
	mOrthoHeight = h;
	mAspect = w / h;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::SetOrthoWindowHeight(float h)
{
	mOrthoHeight = h;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::SetOrthoWindowWidth(float w)
{
	mOrthoHeight = w / mAspect;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

float CameraBase::GetOrthoWindowHeight() const
{
	return mOrthoHeight;
}

float CameraBase::GetOrthoWindowWidth() const
{
	return mOrthoHeight * mAspect;
}

void CameraBase::SetFrustumExtents(float left, float right, float top, float bottom)
{
	mFrustumExtentsManuallySet = true;
	mLeft = left;
	mRight = right;
	mTop = top;
	mBottom = bottom;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::ResetFrustumExtents()
{
	mFrustumExtentsManuallySet = false;

	InvalidateFrustum();
	MarkRenderProxyDataDirty();
}

void CameraBase::GetFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const
{
	UpdateFrustum();

	outleft = mLeft;
	outright = mRight;
	outtop = mTop;
	outbottom = mBottom;
}

void CameraBase::InvalidateFrustum() const
{
	mRecalcFrustum = true;
	mRecalcFrustumPlanes = true;
}

template<bool IsRenderProxy>
Vector2I TCamera<IsRenderProxy>::WorldToScreenPoint(const Vector3& worldPoint) const
{
	Vector2 ndcPoint = WorldToNdcPoint(worldPoint);
	return NdcToScreenPoint(ndcPoint);
}

template<bool IsRenderProxy>
Vector2 TCamera<IsRenderProxy>::WorldToNdcPoint(const Vector3& worldPoint) const
{
	Vector3 viewPoint = WorldToViewPoint(worldPoint);
	return ViewToNdcPoint(viewPoint);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::WorldToViewPoint(const Vector3& worldPoint) const
{
	return GetViewMatrix().MultiplyAffine(worldPoint);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ScreenToWorldPoint(const Vector2I& screenPoint, float depth) const
{
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);
	return NdcToWorldPoint(ndcPoint, depth);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ScreenToWorldPointDeviceDepth(const Vector2I& screenPoint, float deviceDepth) const
{
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);
	Vector4 worldPoint(ndcPoint.X, ndcPoint.Y, deviceDepth, 1.0f);
	worldPoint = GetProjectionMatrixRs().Inverse().Multiply(worldPoint);

	Vector3 worldPoint3D;
	if(Math::Abs(worldPoint.W) > 1e-7f)
	{
		float invW = 1.0f / worldPoint.W;

		worldPoint3D.X = worldPoint.X * invW;
		worldPoint3D.Y = worldPoint.Y * invW;
		worldPoint3D.Z = worldPoint.Z * invW;
	}

	return ViewToWorldPoint(worldPoint3D);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ScreenToViewPoint(const Vector2I& screenPoint, float depth) const
{
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);
	return NdcToViewPoint(ndcPoint, depth);
}

template<bool IsRenderProxy>
Vector2 TCamera<IsRenderProxy>::ScreenToNdcPoint(const Vector2I& screenPoint) const
{
	Area2I viewport = GetViewportRect();

	Vector2 ndcPoint;
	ndcPoint.X = (float)(((screenPoint.X - viewport.X) / (float)viewport.Width) * 2.0f - 1.0f);

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
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
	Vector2 ndcPoint = ViewToNdcPoint(viewPoint);
	return NdcToScreenPoint(ndcPoint);
}

template<bool IsRenderProxy>
Vector2 TCamera<IsRenderProxy>::ViewToNdcPoint(const Vector3& viewPoint) const
{
	Vector3 projPoint = ProjectPoint(viewPoint);

	return Vector2(projPoint.X, projPoint.Y);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::NdcToWorldPoint(const Vector2& ndcPoint, float depth) const
{
	Vector3 viewPoint = NdcToViewPoint(ndcPoint, depth);
	return ViewToWorldPoint(viewPoint);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::NdcToViewPoint(const Vector2& ndcPoint, float depth) const
{
	return UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, depth));
}

template<bool IsRenderProxy>
Vector2I TCamera<IsRenderProxy>::NdcToScreenPoint(const Vector2& ndcPoint) const
{
	Area2I viewport = GetViewportRect();

	Vector2I screenPoint;
	screenPoint.X = Math::RoundToI32(viewport.X + ((ndcPoint.X + 1.0f) * 0.5f) * viewport.Width);

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
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
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);

	Vector3 near = UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, mNearDist));
	Vector3 far = UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, mNearDist + 1.0f));

	Ray ray(near, Vector3::Normalize(far - near));
	ray.TransformAffine(GetViewMatrix().InverseAffine());

	return ray;
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::ProjectPoint(const Vector3& point) const
{
	Vector4 projPoint4(point.X, point.Y, point.Z, 1.0f);
	projPoint4 = GetProjectionMatrixRs().Multiply(projPoint4);

	if(Math::Abs(projPoint4.W) > 1e-7f)
	{
		float invW = 1.0f / projPoint4.W;
		projPoint4.X *= invW;
		projPoint4.Y *= invW;
		projPoint4.Z *= invW;
	}
	else
	{
		projPoint4.X = 0.0f;
		projPoint4.Y = 0.0f;
		projPoint4.Z = 0.0f;
	}

	return Vector3(projPoint4.X, projPoint4.Y, projPoint4.Z);
}

template<bool IsRenderProxy>
Vector3 TCamera<IsRenderProxy>::UnprojectPoint(const Vector3& point) const
{
	// Point.z is expected to be in view space, so we need to do some extra work to get the proper coordinates
	// (as opposed to if point.z was in device coordinates, in which case we could just inverse project)

	// Get world position for a point near the far plane (0.95f)
	Vector4 farAwayPoint(point.X, point.Y, 0.95f, 1.0f);
	farAwayPoint = GetProjectionMatrixRs().Inverse().Multiply(farAwayPoint);

	// Can't proceed if w is too small
	if(Math::Abs(farAwayPoint.W) > 1e-7f)
	{
		// Perspective divide, to get the values that make sense in 3D space
		float invW = 1.0f / farAwayPoint.W;

		Vector3 farAwayPoint3D;
		farAwayPoint3D.X = farAwayPoint.X * invW;
		farAwayPoint3D.Y = farAwayPoint.Y * invW;
		farAwayPoint3D.Z = farAwayPoint.Z * invW;

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
TCamera<IsRenderProxy>::TCamera(const SPtr<SceneInstanceType>& scene)
	:Super(scene), mRecalcView(true)
{
	mRenderSettings = B3DMakeShared<RenderSettingsType>();
}

template <bool IsRenderProxy>
TCamera<IsRenderProxy>::TCamera()
	:TCamera(nullptr)
{ }


template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetTransform(const Transform& transform)
{
	SceneActor::SetTransform(transform);

	mRecalcView = true;
}

template <bool IsRenderProxy>
const Matrix4& TCamera<IsRenderProxy>::GetViewMatrix() const
{
	UpdateView();

	return mViewMatrix;
}

template <bool IsRenderProxy>
const Matrix4& TCamera<IsRenderProxy>::GetViewMatrixInv() const
{
	UpdateView();

	return mViewMatrixInv;
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::SetCustomViewMatrix(bool enable, const Matrix4& viewMatrix)
{
	mCustomViewMatrix = enable;
	if(enable)
	{
		mViewMatrix = viewMatrix;
		mViewMatrixInv = mViewMatrix.InverseAffine();
	}

	MarkRenderProxyDataDirty();
}

template <bool IsRenderProxy>
ConvexVolume TCamera<IsRenderProxy>::GetWorldFrustum() const
{
	const Vector<Plane>& frustumPlanes = GetFrustum().GetPlanes();

	const Transform& transform = this->GetTransform();

	Matrix4 worldMatrix;
	worldMatrix.SetTrs(transform.GetPosition(), transform.GetRotation(), Vector3::kOne);

	Vector<Plane> worldPlanes(frustumPlanes.size());
	u32 i = 0;
	for(auto& plane : frustumPlanes)
	{
		worldPlanes[i] = worldMatrix.MultiplyAffine(plane);
		i++;
	}

	return ConvexVolume(worldPlanes);
}

template <bool IsRenderProxy>
void TCamera<IsRenderProxy>::UpdateView() const
{
	if(!mCustomViewMatrix && mRecalcView)
	{
		mViewMatrix.MakeView(this->mTransform.GetPosition(), this->mTransform.GetRotation());
		mViewMatrixInv = mViewMatrix.InverseAffine();
		mRecalcView = false;
	}
}

template class TCamera<false>;
template class TCamera<true>;

Camera::Camera(const SPtr<SceneInstance>& scene)
	:TCamera(scene)
{ }

SPtr<Camera> Camera::Create(const SPtr<SceneInstance>& scene)
{
	Camera* camera = new(B3DAllocate<Camera>()) Camera(scene);
	SPtr<Camera> cameraShared = B3DMakeSharedFromExisting<Camera>(camera);
	cameraShared->SetShared(cameraShared);
	cameraShared->Initialize();

	return cameraShared;
}

SPtr<Camera> Camera::CreateEmpty()
{
	Camera* camera = new(B3DAllocate<Camera>()) Camera();
	SPtr<Camera> cameraShared = B3DMakeSharedFromExisting<Camera>(camera);
	cameraShared->SetShared(cameraShared);

	return cameraShared;
}

SPtr<render::RenderProxy> Camera::CreateRenderProxy() const
{
	render::Camera* renderProxy = new(B3DAllocate<render::Camera>()) render::Camera(B3DGetRenderProxy(mSceneInstance), B3DGetRenderProxy(mViewport));
	SPtr<render::Camera> renderProxyShared = B3DMakeSharedFromExisting<render::Camera>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

void Camera::Initialize()
{
	mViewport = Viewport::Create(nullptr);

	CoreObject::Initialize();

	GetSceneManager().RegisterCameraInternal(std::static_pointer_cast<Camera>(GetShared()));
}

void Camera::Destroy()
{
	if(IsInitialized())
		GetSceneManager().UnregisterCameraInternal(std::static_pointer_cast<Camera>(GetShared()));

	CoreObject::Destroy();
}

void Camera::SetMain(bool main)
{
	mMain = main;
	GetSceneManager().NotifyMainCameraStateChangedInternal(std::static_pointer_cast<Camera>(GetShared()));
}

Area2I Camera::GetViewportRect() const
{
	return mViewport->GetPixelArea();
}

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
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Camera, RedrawSyncPacket)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(Camera, TransformSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* Camera::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(flags == 0)
		return nullptr;

	if((flags & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		if(flags != (u32)ActorDirtyFlag::Transform)
		{
			FullSyncPacket* const syncPacket = allocator.Construct<FullSyncPacket>(*this, allocator, flags);
			syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);
			syncPacket->RenderSettingsPacket = allocator.Construct<RenderSettings::SyncPacket>(*mRenderSettings, allocator, flags);

			return syncPacket;
		}

		TransformSyncPacket* const transformSyncPacket = allocator.Construct<TransformSyncPacket>(*this, allocator, flags);
		transformSyncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

		return transformSyncPacket;
	}

	// Redraw only
	return allocator.Construct<RedrawSyncPacket>(*this, allocator, flags);
}

void Camera::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	dependencies.push_back(mViewport.get());
}

void Camera::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flag)
{
	CoreObject::MarkRenderProxyDataDirty((u32)flag);
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
	: TCamera(scene), mRendererId(0)
{
	mViewport = Viewport::Create(target, left, top, width, height);
}

Camera::Camera(const SPtr<SceneInstance>& scene, const SPtr<Viewport>& viewport)
	: TCamera(scene), mRendererId(0)
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

Area2I Camera::GetViewportRect() const
{
	return mViewport->GetPixelArea();
}

void Camera::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<RenderProxySyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);

	if((syncPacket->Flags & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		if(syncPacket->Flags != (u32)ActorDirtyFlag::Transform)
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
