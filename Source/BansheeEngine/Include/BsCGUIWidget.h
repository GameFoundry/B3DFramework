//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsComponent.h"
#include "BsRect2I.h"
#include "BsGUIWidget.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Component wrapper for GUIWidget. */
	class BS_EXPORT CGUIWidget : public Component
	{
	public:
		virtual ~CGUIWidget();

		/** @copydoc GUIWidget::setSkin */
		void setSkin(const HGUISkin& skin);

		/** @copydoc GUIWidget::getSkin */
		const GUISkin& getSkin() const;

		/** @copydoc GUIWidget::getSkinResource */
		const HGUISkin& getSkinResource() const;

		/** @copydoc GUIWidget::getPanel */
		GUIPanel* getPanel() const;

		/** @copydoc GUIWidget::getDepth */
		UINT8 getDepth() const;

		/** @copydoc GUIWidget::setDepth */
		void setDepth(UINT8 depth);

		/** @copydoc GUIWidget::inBounds */
		bool inBounds(const Vector2I& position) const;

		/** @copydoc GUIWidget::getBounds */
		const Rect2I& getBounds() const;

		/** @copydoc GUIWidget::isDirty */
		bool isDirty(bool cleanIfDirty);

		/** @copydoc GUIWidget::getTarget */
		Viewport* getTarget() const;

		/** @copydoc GUIWidget::getCamera */
		SPtr<Camera> getCamera() const;

		/** @copydoc GUIWidget::getElements */
		const Vector<GUIElement*>& getElements() const;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Returns the internal GUIWidget that is wrapped by this component. */
		GUIWidget* _getInternal() const { return mInternal.get(); };

		/** @} */

	protected:
		friend class SceneObject;
		friend class GUIElementBase;
		friend class GUIManager;

		/**
		 * Constructs a new GUI widget attached to the specified parent scene object. Widget elements will be rendered on
		 * the provided camera.
		 */
		CGUIWidget(const HSceneObject& parent, const SPtr<Camera>& camera);

		/**
		 * Constructs a new GUI widget attached to the specified parent scene object. Widget elements will be rendered on
		 * the provided camera.
		 */
		CGUIWidget(const HSceneObject& parent, const HCamera& camera);

		/** @copydoc Component::update */
		void update() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

		/**	Called when the viewport size changes and widget elements need to be updated. */
		virtual void ownerTargetResized() { }

		/**	Called when the parent window gained or lost focus. */
		virtual void ownerWindowFocusChanged() { }

	private:
		CGUIWidget(const CGUIWidget& other) { }

		SPtr<GUIWidget> mInternal;
		HEvent mOwnerTargetResizedConn;
		HEvent mOwnerWindowFocusChangedConn;

		SPtr<Camera> mCamera;
		UINT32 mParentHash;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CGUIWidgetRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;

		CGUIWidget() { } // Serialization only
	};

	/** @} */
}