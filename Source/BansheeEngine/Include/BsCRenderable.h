//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRenderable.h"
#include "BsBounds.h"
#include "BsComponent.h"

namespace BansheeEngine
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Renderable
	 * 			
	 * Wraps a TRenderable as a Component.
	 */
	class BS_EXPORT CRenderable : public Component
	{
	public:
		/** @copydoc Renderable::setMesh */
		void setMesh(HMesh mesh) { mInternal->setMesh(mesh); }

		/** @copydoc Renderable::setMaterial */
		void setMaterial(UINT32 idx, HMaterial material) { mInternal->setMaterial(idx, material); }

		/** @copydoc Renderable::setMaterial */
		void setMaterial(HMaterial material) { mInternal->setMaterial(material); }

		/** @copydoc Renderable::setLayer */
		void setLayer(UINT64 layer) { mInternal->setLayer(layer); }

		/** @copydoc Renderable::getLayer */
		UINT64 getLayer() const { return mInternal->getLayer(); }

		/** @copydoc Renderable::getMesh */
		HMesh getMesh() const { return mInternal->getMesh(); }

		/** @copydoc Renderable::getMaterial */
		HMaterial getMaterial(UINT32 idx) const { return mInternal->getMaterial(idx); }

		/**	Gets world bounds of the mesh rendered by this object. */
		Bounds getBounds() const;

		/** @copydoc Component::calculateBounds */
		bool calculateBounds(Bounds& bounds) override;

		/** @name Internal 
		 *  @{
		 */

		/** Returns the internal renderable that is used for majority of operations by this component. */
		SPtr<Renderable> _getRenderable() const { return mInternal; }

		/** @} */

	private:
		/**	Updates the world transform if the SceneObject transform changed. */
		void updateTransform() const;

		mutable SPtr<Renderable> mInternal;

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
		CRenderable() {} // Serialization only
	};

	/** @} */
}