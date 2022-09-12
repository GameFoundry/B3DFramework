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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:Renderable) CRenderable : public Component
	{
	public:
		/** @copydoc Renderable::setMesh */
		BS_SCRIPT_EXPORT(n:Mesh,pr:setter)
		void SetMesh(HMesh mesh);

		/** @copydoc Renderable::getMesh */
		BS_SCRIPT_EXPORT(n:Mesh,pr:getter)
		HMesh GetMesh() const { return mInternal->GetMesh(); }

		/** @copydoc Renderable::setMaterial */
		BS_SCRIPT_EXPORT(n:SetMaterial)
		void SetMaterial(UINT32 idx, HMaterial material) { mInternal->SetMaterial(idx, material); }

		/** @copydoc Renderable::setMaterial */
		BS_SCRIPT_EXPORT(n:SetMaterial)
		void SetMaterial(HMaterial material) { mInternal->SetMaterial(material); }

		/** @copydoc Renderable::getMaterial */
		BS_SCRIPT_EXPORT(n:GetMaterial)
		HMaterial GetMaterial(UINT32 idx) const { return mInternal->GetMaterial(idx); }

		/** @copydoc Renderable::setMaterials */
		BS_SCRIPT_EXPORT(n:Materials,pr:setter)
		void SetMaterials(const Vector<HMaterial>& materials) { mInternal->SetMaterials(materials); }

		/** @copydoc Renderable::getMaterials */
		BS_SCRIPT_EXPORT(n:Materials,pr:getter)
		const Vector<HMaterial>& GetMaterials() { return mInternal->GetMaterials(); }

		/** @copydoc Renderable::setCullDistanceFactor */
		BS_SCRIPT_EXPORT(n:CullDistance, pr:setter)
		void SetCullDistanceFactor(float factor) { mInternal->SetCullDistanceFactor(factor); }

		/** @copydoc Renderable::getCullDistanceFactor */
		BS_SCRIPT_EXPORT(n:CullDistance, pr:getter)
		float GetCullDistanceFactor() const { return mInternal->GetCullDistanceFactor(); }

		/** @copydoc Renderable::setWriteVelocity */
		BS_SCRIPT_EXPORT(n:WriteVelocity, pr:setter)
		void SetWriteVelocity(bool enable) { mInternal->SetWriteVelocity(enable); }

		/** @copydoc Renderable::getWriteVelocity */
		BS_SCRIPT_EXPORT(n:WriteVelocity, pr:getter)
		bool GetWriteVelocity() const { return mInternal->GetWriteVelocity(); }
		
		/** @copydoc Renderable::setLayer */
		BS_SCRIPT_EXPORT(n:Layers,pr:setter)
		void SetLayer(UINT64 layer) { mInternal->SetLayer(layer); }

		/** @copydoc Renderable::getLayer */
		BS_SCRIPT_EXPORT(n:Layers,pr:getter)
		UINT64 GetLayer() const { return mInternal->GetLayer(); }

		/**	Gets world bounds of the mesh rendered by this object. */
		BS_SCRIPT_EXPORT(n:Bounds,pr:getter)
		Bounds GetBounds() const;

		/** @copydoc Component::calculateBounds */
		bool CalculateBounds(Bounds& bounds) override;

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal renderable that is used for majority of operations by this component. */
		SPtr<Renderable> _getInternal() const { return mInternal; }

		/** Registers an Animation component that will be used for animating the renderable's mesh. */
		void _registerAnimation(const HAnimation& animation);

		/** Removes the Animation component, making the renderable rendered as a static object. */
		void _unregisterAnimation();

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

		/** @copydoc Component::onInitialized */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed */
		void OnDestroyed() override;

	public:
		/** @copydoc Component::update */
		void Update() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CRenderableRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CRenderable(); // Serialization only
	};

	/** @} */
}
