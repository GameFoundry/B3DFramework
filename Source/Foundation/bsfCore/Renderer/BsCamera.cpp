//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsCamera.h"
#include "Private/RTTI/BsCameraRTTI.h"
#include "Math/BsMath.h"
#include "Math/BsMatrix3.h"
#include "Math/BsVector2.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Scene/BsSceneManager.h"
#include "CoreThread/BsCoreObjectSync.h"

using namespace bs;

const float CameraBase::kInfiniteFarPlaneAdjust = 0.00001f;

CameraBase::CameraBase()
	: mRecalcFrustum(true), mRecalcFrustumPlanes(true), mRecalcView(true)
{
	InvalidateFrustum();
}

void CameraBase::SetFlags(CameraFlags flags)
{
	mCameraFlags = flags;
	MarkCoreDirtyInternal();
}

void CameraBase::SetHorzFov(const Radian& fov)
{
	mHorzFOV = fov;
	InvalidateFrustum();
	MarkCoreDirtyInternal();
}

void CameraBase::SetFarClipDistance(float farPlane)
{
	mFarDist = farPlane;
	InvalidateFrustum();
	MarkCoreDirtyInternal();
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
	MarkCoreDirtyInternal();
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

const Matrix4& CameraBase::GetViewMatrix() const
{
	UpdateView();

	return mViewMatrix;
}

const Matrix4& CameraBase::GetViewMatrixInv() const
{
	UpdateView();

	return mViewMatrixInv;
}

const ConvexVolume& CameraBase::GetFrustum() const
{
	// Make any pending updates to the calculated frustum planes
	UpdateFrustumPlanes();

	return mFrustum;
}

ConvexVolume CameraBase::GetWorldFrustum() const
{
	const Vector<Plane>& frustumPlanes = GetFrustum().GetPlanes();

	const Transform& tfrm = GetTransform();

	Matrix4 worldMatrix;
	worldMatrix.SetTrs(tfrm.GetPosition(), tfrm.GetRotation(), Vector3::kOne);

	Vector<Plane> worldPlanes(frustumPlanes.size());
	u32 i = 0;
	for(auto& plane : frustumPlanes)
	{
		worldPlanes[i] = worldMatrix.MultiplyAffine(plane);
		i++;
	}

	return ConvexVolume(worldPlanes);
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

		ct::RenderAPI* renderAPI = ct::RenderAPI::InstancePtr();
		renderAPI->ConvertProjectionMatrix(mProjMatrix, mProjMatrixRS);
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

void CameraBase::UpdateView() const
{
	if(!mCustomViewMatrix && mRecalcView)
	{
		mViewMatrix.MakeView(mTransform.GetPosition(), mTransform.GetRotation());
		mViewMatrixInv = mViewMatrix.InverseAffine();
		mRecalcView = false;
	}
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
	MarkCoreDirtyInternal();
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
	MarkCoreDirtyInternal();
}

ProjectionType CameraBase::GetProjectionType() const
{
	return mProjType;
}

void CameraBase::SetCustomViewMatrix(bool enable, const Matrix4& viewMatrix)
{
	mCustomViewMatrix = enable;
	if(enable)
	{
		mViewMatrix = viewMatrix;
		mViewMatrixInv = mViewMatrix.InverseAffine();
	}

	MarkCoreDirtyInternal();
}

void CameraBase::SetCustomProjectionMatrix(bool enable, const Matrix4& projMatrix)
{
	mCustomProjMatrix = enable;

	if(enable)
		mProjMatrix = projMatrix;

	InvalidateFrustum();
	MarkCoreDirtyInternal();
}

void CameraBase::SetOrthoWindow(float w, float h)
{
	mOrthoHeight = h;
	mAspect = w / h;

	InvalidateFrustum();
	MarkCoreDirtyInternal();
}

void CameraBase::SetOrthoWindowHeight(float h)
{
	mOrthoHeight = h;

	InvalidateFrustum();
	MarkCoreDirtyInternal();
}

void CameraBase::SetOrthoWindowWidth(float w)
{
	mOrthoHeight = w / mAspect;

	InvalidateFrustum();
	MarkCoreDirtyInternal();
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
	MarkCoreDirtyInternal();
}

void CameraBase::ResetFrustumExtents()
{
	mFrustumExtentsManuallySet = false;

	InvalidateFrustum();
	MarkCoreDirtyInternal();
}

void CameraBase::GetFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const
{
	UpdateFrustum();

	outleft = mLeft;
	outright = mRight;
	outtop = mTop;
	outbottom = mBottom;
}

void CameraBase::SetTransform(const Transform& transform)
{
	SceneActor::SetTransform(transform);

	mRecalcView = true;
}

void CameraBase::InvalidateFrustum() const
{
	mRecalcFrustum = true;
	mRecalcFrustumPlanes = true;
}

Vector2I CameraBase::WorldToScreenPoint(const Vector3& worldPoint) const
{
	Vector2 ndcPoint = WorldToNdcPoint(worldPoint);
	return NdcToScreenPoint(ndcPoint);
}

Vector2 CameraBase::WorldToNdcPoint(const Vector3& worldPoint) const
{
	Vector3 viewPoint = WorldToViewPoint(worldPoint);
	return ViewToNdcPoint(viewPoint);
}

Vector3 CameraBase::WorldToViewPoint(const Vector3& worldPoint) const
{
	return GetViewMatrix().MultiplyAffine(worldPoint);
}

Vector3 CameraBase::ScreenToWorldPoint(const Vector2I& screenPoint, float depth) const
{
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);
	return NdcToWorldPoint(ndcPoint, depth);
}

Vector3 CameraBase::ScreenToWorldPointDeviceDepth(const Vector2I& screenPoint, float deviceDepth) const
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

Vector3 CameraBase::ScreenToViewPoint(const Vector2I& screenPoint, float depth) const
{
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);
	return NdcToViewPoint(ndcPoint, depth);
}

Vector2 CameraBase::ScreenToNdcPoint(const Vector2I& screenPoint) const
{
	Rect2I viewport = GetViewportRect();

	Vector2 ndcPoint;
	ndcPoint.X = (float)(((screenPoint.X - viewport.X) / (float)viewport.Width) * 2.0f - 1.0f);

	const Conventions& rapiConventions = ct::GetRenderBackendCapabilities().Conventions;
	if(rapiConventions.NdcYAxis == Conventions::Axis::Down)
		ndcPoint.Y = (float)(((screenPoint.Y - viewport.Y) / (float)viewport.Height) * 2.0f - 1.0f);
	else
		ndcPoint.Y = (float)((1.0f - ((screenPoint.Y - viewport.Y) / (float)viewport.Height)) * 2.0f - 1.0f);

	return ndcPoint;
}

Vector3 CameraBase::ViewToWorldPoint(const Vector3& viewPoint) const
{
	return GetViewMatrix().InverseAffine().MultiplyAffine(viewPoint);
}

Vector2I CameraBase::ViewToScreenPoint(const Vector3& viewPoint) const
{
	Vector2 ndcPoint = ViewToNdcPoint(viewPoint);
	return NdcToScreenPoint(ndcPoint);
}

Vector2 CameraBase::ViewToNdcPoint(const Vector3& viewPoint) const
{
	Vector3 projPoint = ProjectPoint(viewPoint);

	return Vector2(projPoint.X, projPoint.Y);
}

Vector3 CameraBase::NdcToWorldPoint(const Vector2& ndcPoint, float depth) const
{
	Vector3 viewPoint = NdcToViewPoint(ndcPoint, depth);
	return ViewToWorldPoint(viewPoint);
}

Vector3 CameraBase::NdcToViewPoint(const Vector2& ndcPoint, float depth) const
{
	return UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, depth));
}

Vector2I CameraBase::NdcToScreenPoint(const Vector2& ndcPoint) const
{
	Rect2I viewport = GetViewportRect();

	Vector2I screenPoint;
	screenPoint.X = Math::RoundToI32(viewport.X + ((ndcPoint.X + 1.0f) * 0.5f) * viewport.Width);

	const Conventions& rapiConventions = ct::GetRenderBackendCapabilities().Conventions;
	if(rapiConventions.NdcYAxis == Conventions::Axis::Down)
		screenPoint.Y = Math::RoundToI32(viewport.Y + (ndcPoint.Y + 1.0f) * 0.5f * viewport.Height);
	else
		screenPoint.Y = Math::RoundToI32(viewport.Y + (1.0f - (ndcPoint.Y + 1.0f) * 0.5f) * viewport.Height);

	return screenPoint;
}

Ray CameraBase::ScreenPointToRay(const Vector2I& screenPoint) const
{
	Vector2 ndcPoint = ScreenToNdcPoint(screenPoint);

	Vector3 near = UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, mNearDist));
	Vector3 far = UnprojectPoint(Vector3(ndcPoint.X, ndcPoint.Y, mNearDist + 1.0f));

	Ray ray(near, Vector3::Normalize(far - near));
	ray.TransformAffine(GetViewMatrix().InverseAffine());

	return ray;
}

Vector3 CameraBase::ProjectPoint(const Vector3& point) const
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

Vector3 CameraBase::UnprojectPoint(const Vector3& point) const
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

template <bool Core>
TCamera<Core>::TCamera()
{
	mRenderSettings = B3DMakeShared<RenderSettingsType>();
}

template <bool Core>
template <class P>
void TCamera<Core>::RttiEnumFields(P p)
{
	p(mLayers);
	p(mProjType);
	p(mHorzFOV);
	p(mFarDist);
	p(mNearDist);
	p(mAspect);
	p(mOrthoHeight);
	p(mPriority);
	p(mCustomViewMatrix);
	p(mCustomProjMatrix);
	p(mFrustumExtentsManuallySet);
	p(mMSAA);
	p(mMain);
	p(*mRenderSettings);
	p(mCameraFlags);
}

template class TCamera<false>;
template class TCamera<true>;

SPtr<ct::Camera> Camera::GetCore() const
{
	return std::static_pointer_cast<ct::Camera>(mCoreSpecific);
}

SPtr<Camera> Camera::Create()
{
	Camera* handler = new(B3DAllocate<Camera>()) Camera();
	SPtr<Camera> handlerPtr = B3DMakeCoreFromExisting<Camera>(handler);
	handlerPtr->SetThisPtrInternal(handlerPtr);
	handlerPtr->Initialize();

	return handlerPtr;
}

SPtr<Camera> Camera::CreateEmpty()
{
	Camera* handler = new(B3DAllocate<Camera>()) Camera();
	SPtr<Camera> handlerPtr = B3DMakeCoreFromExisting<Camera>(handler);
	handlerPtr->SetThisPtrInternal(handlerPtr);

	return handlerPtr;
}

SPtr<ct::CoreObject> Camera::CreateCore() const
{
	ct::Camera* handler = new(B3DAllocate<ct::Camera>()) ct::Camera(mViewport->GetCore());
	SPtr<ct::Camera> handlerPtr = B3DMakeSharedFromExisting<ct::Camera>(handler);
	handlerPtr->SetThisPtrInternal(handlerPtr);

	return handlerPtr;
}

void Camera::Initialize()
{
	mViewport = Viewport::Create(nullptr);

	CoreObject::Initialize();

	GetSceneManager().RegisterCameraInternal(std::static_pointer_cast<Camera>(GetThisPtr()));
}

void Camera::Destroy()
{
	if(IsInitialized())
		GetSceneManager().UnregisterCameraInternal(std::static_pointer_cast<Camera>(GetThisPtr()));

	CoreObject::Destroy();
}

void Camera::SetMain(bool main)
{
	mMain = main;
	GetSceneManager().NotifyMainCameraStateChangedInternal(std::static_pointer_cast<Camera>(GetThisPtr()));
}

Rect2I Camera::GetViewportRect() const
{
	return mViewport->GetPixelArea();
}

CoreSyncData Camera::SyncToCore(FrameAlloc* allocator)
{
	u32 dirtyFlag = GetCoreDirtyFlags();

	u32 size = B3DRTTISize(dirtyFlag).Bytes;

	if((dirtyFlag & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		size += CoreSyncGetSize((SceneActor&)*this);

		if(dirtyFlag != (u32)ActorDirtyFlag::Transform)
			size += CoreSyncGetSize(*this);
	}

	u8* buffer = allocator->Alloc(size);
	Bitstream stream(buffer, size);

	B3DRTTIWrite(dirtyFlag, stream);

	if((dirtyFlag & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		B3DCoreSyncWrite((SceneActor&)*this, stream);

		if(dirtyFlag != (u32)ActorDirtyFlag::Transform)
			B3DCoreSyncWrite(*this, stream);
	}

	return CoreSyncData(buffer, size);
}

void Camera::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	dependencies.push_back(mViewport.get());
}

void Camera::MarkCoreDirtyInternal(ActorDirtyFlag flag)
{
	MarkCoreDirty((u32)flag);
}

RTTITypeBase* Camera::GetRttiStatic()
{
	return CameraRTTI::Instance();
}

RTTITypeBase* Camera::GetRtti() const
{
	return Camera::GetRttiStatic();
}

namespace bs { namespace ct
{
Camera::~Camera()
{
	RendererManager::Instance().GetActive()->NotifyCameraRemoved(this);
}

Camera::Camera(SPtr<RenderTarget> target, float left, float top, float width, float height)
	: mRendererId(0)
{
	mViewport = Viewport::Create(target, left, top, width, height);
}

Camera::Camera(const SPtr<Viewport>& viewport)
	: mRendererId(0)
{
	mViewport = viewport;
}

void Camera::Initialize()
{
	RendererManager::Instance().GetActive()->NotifyCameraAdded(this);

	CoreObject::Initialize();
}

Rect2I Camera::GetViewportRect() const
{
	return mViewport->GetPixelArea();
}

void Camera::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	u32 dirtyFlag;
	B3DRTTIRead(dirtyFlag, stream);

	if((dirtyFlag & ~(i32)CameraDirtyFlag::Redraw) != 0)
	{
		B3DCoreSyncRead((SceneActor&)*this, stream);

		if(dirtyFlag != (u32)ActorDirtyFlag::Transform)
			B3DCoreSyncRead(*this, stream);

		mRecalcFrustum = true;
		mRecalcFrustumPlanes = true;
		mRecalcView = true;
	}

	RendererManager::Instance().GetActive()->NotifyCameraUpdated(this, (u32)dirtyFlag);
}
}}
