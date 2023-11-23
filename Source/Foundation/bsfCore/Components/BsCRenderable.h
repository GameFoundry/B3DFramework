//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsRenderable.h"
#include "Math/BsBounds.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Renderable
	 *
	 * @note	Wraps a Renderable as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(Renderable)) CRenderable : public Component
	{
	public:
		/** @copydoc Renderable::SetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Setter))
		void SetMesh(HMesh mesh);

		/** @copydoc Renderable::GetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Getter))
		HMesh GetMesh() const { return mInternal->GetMesh(); }

		/** @copydoc Renderable::SetMaterial */
		B3D_SCRIPT_EXPORT(ExportName(SetMaterial))
		void SetMaterial(u32 idx, HMaterial material) { mInternal->SetMaterial(idx, material); }

		/** @copydoc Renderable::SetMaterial */
		B3D_SCRIPT_EXPORT(ExportName(SetMaterial))
		void SetMaterial(HMaterial material) { mInternal->SetMaterial(material); }

		/** @copydoc Renderable::GetMaterial */
		B3D_SCRIPT_EXPORT(ExportName(GetMaterial))
		HMaterial GetMaterial(u32 idx) const { return mInternal->GetMaterial(idx); }

		/** @copydoc Renderable::SetMaterials */
		B3D_SCRIPT_EXPORT(ExportName(Materials), Property(Setter))
		void SetMaterials(const Vector<HMaterial>& materials) { mInternal->SetMaterials(materials); }

		/** @copydoc Renderable::GetMaterials */
		B3D_SCRIPT_EXPORT(ExportName(Materials), Property(Getter))
		const Vector<HMaterial>& GetMaterials() { return mInternal->GetMaterials(); }

		/** @copydoc Renderable::SetCullDistanceFactor */
		B3D_SCRIPT_EXPORT(ExportName(CullDistance), Property(Setter))
		void SetCullDistanceFactor(float factor) { mInternal->SetCullDistanceFactor(factor); }

		/** @copydoc Renderable::GetCullDistanceFactor */
		B3D_SCRIPT_EXPORT(ExportName(CullDistance), Property(Getter))
		float GetCullDistanceFactor() const { return mInternal->GetCullDistanceFactor(); }

		/** @copydoc Renderable::SetWriteVelocity */
		B3D_SCRIPT_EXPORT(ExportName(WriteVelocity), Property(Setter))
		void SetWriteVelocity(bool enable) { mInternal->SetWriteVelocity(enable); }

		/** @copydoc Renderable::GetWriteVelocity */
		B3D_SCRIPT_EXPORT(ExportName(WriteVelocity), Property(Getter))
		bool GetWriteVelocity() const { return mInternal->GetWriteVelocity(); }

		/** @copydoc Renderable::SetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Setter))
		void SetLayer(u64 layer) { mInternal->SetLayer(layer); }

		/** @copydoc Renderable::GetLayer */
		B3D_SCRIPT_EXPORT(ExportName(Layers), Property(Getter))
		u64 GetLayer() const { return mInternal->GetLayer(); }

		/**	Gets world bounds of the mesh rendered by this object. */
		B3D_SCRIPT_EXPORT(ExportName(Bounds), Property(Getter))
		Bounds GetBounds() const;

		/** @copydoc Component::CalculateBounds */
		bool CalculateBounds(Bounds& bounds) override;

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal renderable that is used for majority of operations by this component. */
		SPtr<Renderable> GetInternalInternal() const { return mInternal; }

		/** Registers an Animation component that will be used for animating the renderable's mesh. */
		void RegisterAnimationInternal(const HAnimation& animation);

		/** Removes the Animation component, making the renderable rendered as a static object. */
		void UnregisterAnimationInternal();

		/** @} */

	private:
		mutable SPtr<Renderable> mInternal;
		HAnimation mAnimation;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		CRenderable(const HSceneObject& parent);

		void OnInitialized() override;
		void OnDestroyed() override;

	public:
		void Update() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CRenderableRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CRenderable(); // Serialization only
	};

	/** @} */
} // namespace bs
