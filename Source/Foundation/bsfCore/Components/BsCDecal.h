//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsDecal.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/** Wraps Decal as a Component. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:Decal) CDecal : public Component
	{
	public:
		CDecal(const HSceneObject& parent);
		virtual ~CDecal();

		/** @copydoc Decal::setMaterial */
		BS_SCRIPT_EXPORT(n:Material,pr:setter)
		void setMaterial(const HMaterial& material) { mInternal->setMaterial(material); }

		/** @copydoc setMaterial */
		BS_SCRIPT_EXPORT(n:Material,pr:getter)
		const HMaterial& getMaterial() const { return mInternal->getMaterial(); }

		/** @copydoc Decal::setSize */
		BS_SCRIPT_EXPORT(n:Size,pr:setter)
		void setSize(const Vector2& size) { mInternal->setSize(size); }

		/** @copydoc setSize */
		BS_SCRIPT_EXPORT(n:Size,pr:getter)
		Vector2 getSize() const { return mInternal->getSize(); }

		/** @copydoc Decal::setMaxDistance */
		BS_SCRIPT_EXPORT(n:MaxDistance,pr:setter)
		void setMaxDistance(float distance) { mInternal->setMaxDistance(distance); }

		/** @copydoc getSize */
		BS_SCRIPT_EXPORT(n:MaxDistance,pr:getter)
		float getMaxDistance() const { return mInternal->getMaxDistance(); }

		/** @copydoc Decal::setLayer */
		BS_SCRIPT_EXPORT(n:Layer,pr:setter)
		void setLayer(UINT64 layer) { mInternal->setLayer(layer); }

		/** @copydoc setLayer() */
		BS_SCRIPT_EXPORT(n:Layer,pr:getter)
		UINT64 getLayer() const { return mInternal->getLayer(); }

		BS_SCRIPT_EXPORT(n:LayerMask,pr:setter)
		/** @copydoc Decal::setLayerMask */
		void setLayerMask(UINT32 mask) { mInternal->setLayerMask(mask); }

		BS_SCRIPT_EXPORT(n:LayerMask,pr:getter)
		/** @copydoc setLayerMask */
		UINT32 getLayerMask() const { return mInternal->getLayerMask(); }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the internal decal object that this component wraps. */
		SPtr<Decal> _getInternal() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<Decal> mInternal;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

		/** @copydoc Component::update */
		void update() override { }

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CDecalRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CDecal(); // Serialization only
	};

	/** @} */
}