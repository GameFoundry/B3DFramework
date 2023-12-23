//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Scene/BsComponent.h"
#include "Math/BsRect2I.h"
#include "GUI/BsGUIWidget.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Component wrapper for GUIWidget. */
	class B3D_EXPORT CGUIWidget : public Component
	{
	public:
		virtual ~CGUIWidget() = default;

		/** @copydoc GUIWidget::SetSkin */
		void SetSkin(const HGUISkin& skin);

		/** @copydoc GUIWidget::GetSkin */
		const GUISkin& GetSkin() const;

		/** @copydoc GUIWidget::GetSkinResource */
		const HGUISkin& GetSkinResource() const;

		/** @copydoc GUIWidget::GetStyleSheet */
		const HGUIStyleSheet& GetStyleSheet() const;

		/** @copydoc GUIWidget::GetPanel */
		GUIPanel* GetPanel() const;

		/** @copydoc GUIWidget::GetDepth */
		u8 GetDepth() const;

		/** @copydoc GUIWidget::SetDepth */
		void SetDepth(u8 depth);

		/** @copydoc GUIWidget::InBounds */
		bool InBounds(const Vector2I& position) const;

		/** @copydoc GUIWidget::GetBounds */
		const Rect2I& GetBounds() const;

		/** @copydoc GUIWidget::GetTarget */
		Viewport* GetTarget() const;

		/** @copydoc GUIWidget::GetCamera */
		SPtr<Camera> GetCamera() const;

		/** @copydoc GUIWidget::GetElements */
		const Vector<GUIRenderable*>& GetElements() const;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Returns the internal GUIWidget that is wrapped by this component. */
		GUIWidget* GetInternalInternal() const { return mInternal.get(); };

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

		void Update() override;
		void OnDestroyed() override;

		/**	Called when the viewport size changes and widget elements need to be updated. */
		virtual void OwnerTargetResized() {}

		/**	Called when the parent window gained or lost focus. */
		virtual void OwnerWindowFocusChanged() {}

	private:
		CGUIWidget(CGUIWidget&&) = delete;
		CGUIWidget(const CGUIWidget&) = delete;

		SPtr<GUIWidget> mInternal;
		HEvent mOwnerTargetResizedConn;
		HEvent mOwnerWindowFocusChangedConn;

		SPtr<Camera> mCamera;
		u32 mParentHash;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CGUIWidgetRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

		CGUIWidget(); // Serialization only
	};

	/** @} */
} // namespace bs
