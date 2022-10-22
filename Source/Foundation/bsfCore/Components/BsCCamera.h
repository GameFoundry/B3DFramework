//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsCamera.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Camera
	 * 			
	 * Wraps a Camera as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering),ExportName(Camera)) CCamera : public Component
	{
	public:
		CCamera(const HSceneObject& parent);
		virtual ~CCamera() = default;

		/** @copydoc Camera::setFlags */
		BS_SCRIPT_EXPORT(ExportName(Flags),Property(Setter))
		void SetFlags(CameraFlags flags) { mInternal->SetFlags(flags); }

		/** @copydoc Camera::getFlags */
		BS_SCRIPT_EXPORT(ExportName(Flags),Property(Getter))
		CameraFlags GetFlags() const { return mInternal->GetFlags(); }
		
		/** @copydoc Camera::getViewport */
		BS_SCRIPT_EXPORT(ExportName(Viewport),Property(Getter))
		SPtr<Viewport> GetViewport() const { return mInternal->GetViewport(); }

		/** @copydoc Camera::setHorzFOV */
		BS_SCRIPT_EXPORT(ExportName(FieldOfView),Property(Setter),Range([1,360]),UI(AsSlider),UIOrder(-1))
		virtual void SetHorzFov(const Radian& fovy) { mInternal->SetHorzFov(fovy); }

		/** @copydoc Camera::getHorzFOV */
		BS_SCRIPT_EXPORT(ExportName(FieldOfView),Property(Getter),Range([1,360]),UI(AsSlider),UIOrder(-1))
		virtual const Radian& GetHorzFov() const { return mInternal->GetHorzFov(); }

		/** @copydoc Camera::setNearClipDistance */
		BS_SCRIPT_EXPORT(ExportName(NearClipPlane),Property(Setter))
		virtual void SetNearClipDistance(float nearDist) { mInternal->SetNearClipDistance(nearDist); }

		/** @copydoc Camera::getNearClipDistance */
		BS_SCRIPT_EXPORT(ExportName(NearClipPlane),Property(Getter))
		virtual float GetNearClipDistance() const { return mInternal->GetNearClipDistance(); }

		/** @copydoc Camera::setFarClipDistance */
		BS_SCRIPT_EXPORT(ExportName(FarClipPlane),Property(Setter))
		virtual void SetFarClipDistance(float farDist) { mInternal->SetFarClipDistance(farDist); }

		/** @copydoc Camera::getFarClipDistance */
		BS_SCRIPT_EXPORT(ExportName(FarClipPlane),Property(Getter))
		virtual float GetFarClipDistance() const { return mInternal->GetFarClipDistance(); }

		/** @copydoc Camera::setAspectRatio */
		BS_SCRIPT_EXPORT(ExportName(AspectRatio),Property(Setter))
		virtual void SetAspectRatio(float ratio) { mInternal->SetAspectRatio(ratio); }

		/** @copydoc Camera::getAspectRatio */
		BS_SCRIPT_EXPORT(ExportName(AspectRatio),Property(Getter))
		virtual float GetAspectRatio() const { return mInternal->GetAspectRatio(); }

		/** @copydoc Camera::setFrustumExtents */
		virtual void SetFrustumExtents(float left, float right, float top, float bottom)
		{
			mInternal->SetFrustumExtents(left, right, top, bottom);
		}

		/** @copydoc Camera::resetFrustumExtents */
		virtual void ResetFrustumExtents() { mInternal->ResetFrustumExtents(); }

		/** @copydoc Camera::getFrustumExtents */
		virtual void GetFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const
		{
			mInternal->GetFrustumExtents(outleft, outright, outtop, outbottom);
		}

		/** @copydoc Camera::getProjectionMatrixRS */
		BS_SCRIPT_EXPORT(ExportName(ProjMatrix),Property(Getter))
		virtual const Matrix4& GetProjectionMatrixRs() const { return mInternal->GetProjectionMatrixRs(); }

		/** @copydoc Camera::getProjectionMatrix */
		virtual const Matrix4& GetProjectionMatrix() const { return mInternal->GetProjectionMatrix(); }

		/** @copydoc Camera::getViewMatrix */
		BS_SCRIPT_EXPORT(ExportName(ViewMatrix),Property(Getter))
		virtual const Matrix4& GetViewMatrix() const { UpdateView(); return mInternal->GetViewMatrix(); }

		/** @copydoc Camera::setCustomViewMatrix */
		virtual void SetCustomViewMatrix(bool enable, const Matrix4& viewMatrix = Matrix4::IDENTITY)
		{
			mInternal->SetCustomViewMatrix(enable, viewMatrix);
		}

		/** @copydoc Camera::isCustomViewMatrixEnabled */
		virtual bool IsCustomViewMatrixEnabled() const { return mInternal->IsCustomViewMatrixEnabled(); }
		
		/** @copydoc Camera::setCustomProjectionMatrix */
		virtual void SetCustomProjectionMatrix(bool enable, const Matrix4& projectionMatrix = Matrix4::IDENTITY)
		{
			mInternal->SetCustomProjectionMatrix(enable, projectionMatrix);
		}

		/** @copydoc Camera::isCustomProjectionMatrixEnabled */
		virtual bool IsCustomProjectionMatrixEnabled() const { return mInternal->IsCustomProjectionMatrixEnabled(); }

		/** @copydoc Camera::getFrustum */
		virtual const ConvexVolume& GetFrustum() const { return mInternal->GetFrustum(); }

		/** @copydoc Camera::getWorldFrustum */
		virtual ConvexVolume GetWorldFrustum() const;

		/** @copydoc Camera::getBoundingBox */
		const AABox& GetBoundingBox() const { return mInternal->GetBoundingBox(); }

		/** @copydoc Camera::setProjectionType */
		BS_SCRIPT_EXPORT(ExportName(ProjectionType),Property(Setter),UIOrder(-2))
		virtual void SetProjectionType(ProjectionType pt) { mInternal->SetProjectionType(pt); }

		/** @copydoc Camera::getProjectionType */
		BS_SCRIPT_EXPORT(ExportName(ProjectionType),Property(Getter),UIOrder(-2))
		virtual ProjectionType GetProjectionType() const { return mInternal->GetProjectionType(); }

		/** @copydoc Camera::setOrthoWindow */
		virtual void SetOrthoWindow(float w, float h) { mInternal->SetOrthoWindow(w, h); }

		/** @copydoc Camera::setOrthoWindowHeight */
		BS_SCRIPT_EXPORT(ExportName(OrthoHeight),Property(Setter),UIOrder(-1))
		virtual void SetOrthoWindowHeight(float h) { mInternal->SetOrthoWindowHeight(h); }

		/** @copydoc Camera::getOrthoWindowHeight */
		BS_SCRIPT_EXPORT(ExportName(OrthoHeight),Property(Getter),UIOrder(-1))
		virtual float GetOrthoWindowHeight() const { return mInternal->GetOrthoWindowHeight(); }

		/** @copydoc Camera::setOrthoWindowWidth */
		BS_SCRIPT_EXPORT(ExportName(OrthoWidth),Property(Setter),UI(Hide))
		virtual void SetOrthoWindowWidth(float w) { mInternal->SetOrthoWindowWidth(w); }

		/** @copydoc Camera::getOrthoWindowWidth */
		BS_SCRIPT_EXPORT(ExportName(OrthoWidth),Property(Getter),UI(Hide))
		virtual float GetOrthoWindowWidth() const { return mInternal->GetOrthoWindowWidth(); }

		/** @copydoc Camera::setPriority */
		BS_SCRIPT_EXPORT(ExportName(Priority),Property(Setter))
		void SetPriority(i32 priority) { mInternal->SetPriority(priority); }

		/** @copydoc Camera::getPriority */
		BS_SCRIPT_EXPORT(ExportName(Priority),Property(Getter))
		i32 GetPriority() const { return mInternal->GetPriority(); }

		/** @copydoc Camera::setLayers */
		BS_SCRIPT_EXPORT(ExportName(Layers),Property(Setter),UI(AsLayerMask))
		void SetLayers(u64 layers) { mInternal->SetLayers(layers); }

		/** @copydoc Camera::getLayers */
		BS_SCRIPT_EXPORT(ExportName(Layers),Property(Getter),UI(AsLayerMask))
		u64 GetLayers() const { return mInternal->GetLayers(); }

		/** @copydoc Camera::setMSAACount */
		BS_SCRIPT_EXPORT(ExportName(SampleCount),Property(Setter))
		void SetMsaaCount(u32 count) { mInternal->SetMsaaCount(count); }

		/** @copydoc Camera::getMSAACount */
		BS_SCRIPT_EXPORT(ExportName(SampleCount),Property(Getter))
		u32 GetMsaaCount() const { return mInternal->GetMsaaCount(); }

		/** @copydoc Camera::setRenderSettings() */
		BS_SCRIPT_EXPORT(ExportName(RenderSettings),Property(Setter),ApplyOnDirty(true))
		void SetRenderSettings(const SPtr<RenderSettings>& settings) { mInternal->SetRenderSettings(settings); }

		/** @copydoc Camera::getRenderSettings() */
		BS_SCRIPT_EXPORT(ExportName(RenderSettings),Property(Getter),ApplyOnDirty(true))
		const SPtr<RenderSettings>& GetRenderSettings() const { return mInternal->GetRenderSettings(); }

		/** @copydoc Camera::notifyNeedsRedraw() */
		BS_SCRIPT_EXPORT()
		void NotifyNeedsRedraw() { mInternal->NotifyNeedsRedraw(); }
		
		/** @copydoc Camera::worldToScreenPoint */
		BS_SCRIPT_EXPORT()
		Vector2I WorldToScreenPoint(const Vector3& worldPoint) const { UpdateView(); return mInternal->WorldToScreenPoint(worldPoint); }

		/** @copydoc Camera::worldToNdcPoint */
		BS_SCRIPT_EXPORT()
		Vector2 WorldToNdcPoint(const Vector3& worldPoint) const { UpdateView(); return mInternal->WorldToNdcPoint(worldPoint); }

		/** @copydoc Camera::worldToViewPoint */
		BS_SCRIPT_EXPORT()
		Vector3 WorldToViewPoint(const Vector3& worldPoint) const { UpdateView(); return mInternal->WorldToViewPoint(worldPoint); }

		/** @copydoc Camera::screenToWorldPoint */
		BS_SCRIPT_EXPORT()
		Vector3 ScreenToWorldPoint(const Vector2I& screenPoint, float depth = 0.5f) const { UpdateView(); return mInternal->ScreenToWorldPoint(screenPoint, depth); }

		/** @copydoc Camera::screenToViewPoint */
		BS_SCRIPT_EXPORT()
		Vector3 ScreenToViewPoint(const Vector2I& screenPoint, float depth = 0.5f) const { return mInternal->ScreenToViewPoint(screenPoint, depth); }

		/** @copydoc Camera::screenToNdcPoint */
		BS_SCRIPT_EXPORT()
		Vector2 ScreenToNdcPoint(const Vector2I& screenPoint) const { return mInternal->ScreenToNdcPoint(screenPoint); }

		/** @copydoc Camera::viewToWorldPoint */
		BS_SCRIPT_EXPORT()
		Vector3 ViewToWorldPoint(const Vector3& viewPoint) const { UpdateView(); return mInternal->ViewToWorldPoint(viewPoint); }

		/** @copydoc Camera::viewToScreenPoint */
		BS_SCRIPT_EXPORT()
		Vector2I ViewToScreenPoint(const Vector3& viewPoint) const { return mInternal->ViewToScreenPoint(viewPoint); }

		/** @copydoc Camera::viewToNdcPoint */
		BS_SCRIPT_EXPORT()
		Vector2 ViewToNdcPoint(const Vector3& viewPoint) const { return mInternal->ViewToNdcPoint(viewPoint); }

		/** @copydoc Camera::ndcToWorldPoint */
		BS_SCRIPT_EXPORT()
		Vector3 NdcToWorldPoint(const Vector2& ndcPoint, float depth = 0.5f) const { UpdateView(); return mInternal->NdcToWorldPoint(ndcPoint, depth); }

		/** @copydoc Camera::ndcToViewPoint */
		BS_SCRIPT_EXPORT()
		Vector3 NdcToViewPoint(const Vector2& ndcPoint, float depth = 0.5f) const { return mInternal->NdcToViewPoint(ndcPoint, depth); }

		/** @copydoc Camera::ndcToScreenPoint */
		BS_SCRIPT_EXPORT()
		Vector2I NdcToScreenPoint(const Vector2& ndcPoint) const { return mInternal->NdcToScreenPoint(ndcPoint); }

		/** @copydoc Camera::screenPointToRay */
		BS_SCRIPT_EXPORT()
		Ray ScreenPointToRay(const Vector2I& screenPoint) const { UpdateView(); return mInternal->ScreenPointToRay(screenPoint); }

		/** @copydoc Camera::projectPoint */
		BS_SCRIPT_EXPORT()
		Vector3 ProjectPoint(const Vector3& point) const { return mInternal->ProjectPoint(point); }

		/** @copydoc Camera::unprojectPoint */
		BS_SCRIPT_EXPORT()
		Vector3 UnprojectPoint(const Vector3& point) const { return mInternal->UnprojectPoint(point); }

		/** @copydoc Camera::setMain */	
		BS_SCRIPT_EXPORT(ExportName(Main),Property(Setter))
		void SetMain(bool main);

		/** @copydoc Camera::isMain */
		BS_SCRIPT_EXPORT(ExportName(Main),Property(Getter))
		bool IsMain() const { return mInternal->IsMain(); }

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal camera that is used for majority of operations by this component. */
		SPtr<Camera> GetCameraInternal() const { UpdateView(); return mInternal; }

		/** @} */

	protected:
		/** Checks if the world transform of the camera changed, and if needed updates the view matrix. */
		void UpdateView() const;

		mutable SPtr<Camera> mInternal;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::_instantiate */
		void InstantiateInternal() override;

		/** @copydoc Component::onInitialized */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed */
		void OnDestroyed() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CCameraRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CCamera();// Serialization only
	 };

	 /** @} */
}
