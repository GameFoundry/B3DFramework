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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(GUIWidget)) CGUIWidget : public Component
	{
	public:
		virtual ~CGUIWidget() = default;

		/** @copydoc GUIWidget::GetStyleSheetCascade */
		const GUIStyleSheetCascade& GetStyleSheetCascade() const;

		/** @copydoc GUIWidget::SetStyleSheetCascade */
		void SetStyleSheetCascade(const SPtr<const GUIStyleSheetCascade>& styleSheetCascade);

		/** @copydoc GUIWidget::GetPanel */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Panel))
		GUIPanel* GetPanel() const;

		/** @copydoc GUIWidget::GetDepth */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Depth))
		u8 GetDepth() const;

		/** @copydoc GUIWidget::SetDepth */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Depth))
		void SetDepth(u8 depth);

		/** @copydoc GUIWidget::InBounds */
		B3D_SCRIPT_EXPORT()
		bool InBounds(const GUIPhysicalPoint& position) const;

		/** @copydoc GUIWidget::GetBounds */
		B3D_SCRIPT_EXPORT()
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
		GUIWidget* GetInternalInternal() const { return mInternal.get(); }

		/** @} */

	protected:
		friend class SceneObject;
		friend class GUIElement;
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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		CGUIWidget(); // Serialization only
	};

	/** @} */
} // namespace bs
