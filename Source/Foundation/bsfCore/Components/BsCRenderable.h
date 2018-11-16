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
	 * @note Wraps a TRenderable as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:Renderable) CRenderable : public Component
	{
	public:
		/** @copydoc Renderable::setMesh */
		BS_SCRIPT_EXPORT(n:Mesh,pr:setter)
		void setMesh(HMesh mesh);

		/** @copydoc Renderable::getMesh */
		BS_SCRIPT_EXPORT(n:Mesh,pr:getter)
		HMesh getMesh() const { return mInternal->getMesh(); }

		/** @copydoc Renderable::setMaterial */
		BS_SCRIPT_EXPORT(n:SetMaterial)
		void setMaterial(UINT32 idx, HMaterial material) { mInternal->setMaterial(idx, material); }

		/** @copydoc Renderable::setMaterial */
		BS_SCRIPT_EXPORT(n:SetMaterial)
		void setMaterial(HMaterial material) { mInternal->setMaterial(material); }

		/** @copydoc Renderable::getMaterial */
		BS_SCRIPT_EXPORT(n:GetMaterial)
		HMaterial getMaterial(UINT32 idx) const { return mInternal->getMaterial(idx); }

		/** @copydoc Renderable::setMaterials */
		BS_SCRIPT_EXPORT(n:Materials,pr:setter)
		void setMaterials(const Vector<HMaterial>& materials) { mInternal->setMaterials(materials); }

		/** @copydoc Renderable::getMaterials */
		BS_SCRIPT_EXPORT(n:Materials,pr:getter)
		const Vector<HMaterial>& getMaterials() { return mInternal->getMaterials(); }

		/** @copydoc Renderable::setLayer */
		BS_SCRIPT_EXPORT(n:Layers,pr:setter)
		void setLayer(UINT64 layer) { mInternal->setLayer(layer); }

		/** @copydoc Renderable::getLayer */
		BS_SCRIPT_EXPORT(n:Layers,pr:getter)
		UINT64 getLayer() const { return mInternal->getLayer(); }

		/**	Gets world bounds of the mesh rendered by this object. */
		BS_SCRIPT_EXPORT(n:Bounds,pr:getter)
		Bounds getBounds() const;

		/** @copydoc Component::calculateBounds */
		bool calculateBounds(Bounds& bounds) override;

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
		void onInitialized() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

	public:
		/** @copydoc Component::update */
		void update() override;

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