//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIRenderable.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElement.h"
#include "GUI/B3DGUIOptions.h"
#include "2D/B3DSprite.h"
#include "Math/B3DArea2.h"
#include "Image/B3DColor.h"

namespace b3d
{
	class GUINavGroup;

	/** @addtogroup GUI
	 *  @{
	 */

	/** Contains options that change GUIElement behaviour. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIElementOption
	{
		/**
		 * Enable this option if you want pointer events to pass through this element by default. This will allow elements
		 * underneath this element to receive pointer events.
		 */
		ClickThrough = 1 << 0,

		/**
		 * Enable this option if the element accepts keyboard/gamepad input focus. This will allow the element to be
		 * navigated to using keys/buttons.
		 */
		AcceptsKeyFocus = 1 << 1,

		/** Pointer events on the GUI element will be ignored. */
		IgnorePointerEvents = 1 << 2,
	};

	typedef Flags<GUIElementOption> GUIElementOptions;
	B3D_FLAGS_OPERATORS(GUIElementOption)

	/** Represents a GUI element that can be interacted with. All interactable elements are also renderable (i.e. have a visual component). */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIInteractable : public GUIRenderable
	{
	public:
		GUIInteractable(String styleClass, const GUISizeConstraints& dimensions, GUIElementOptions options = GUIElementOptions(0));
		GUIInteractable(const char* styleClass, const GUISizeConstraints& dimensions, GUIElementOptions options = GUIElementOptions(0));
		~GUIInteractable() override = default;

		/**
		 * Change the GUI element focus state.
		 *
		 * @param	enabled		Give the element focus or take it away.
		 * @param	clear		If true the focus will be cleared from any elements currently in focus. Otherwise
		 *						the element will just be appended to the in-focus list (if enabling focus).
		 */
		B3D_SCRIPT_EXPORT()
		virtual void SetFocus(bool enabled, bool clear = false);

		/** A set of flags controlling various aspects of the GUIElement. See GUIElementOptions.  */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(OptionFlags))
		void SetOptionFlags(GUIElementOptions options) { mOptionFlags = options; }

		/** @copydoc SetOptionFlags */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(OptionFlags))
		GUIElementOptions GetOptionFlags() const { return mOptionFlags; }

		/**
		 * Assigns a new context menu that will be opened when the element is right clicked. Null is allowed in case no
		 * context menu is wanted.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(ContextMenu))
		void SetContextMenu(const SPtr<GUIContextMenu>& menu) { mContextMenu = menu; }

		/**
		 * Sets a navigation group that determines in what order are GUI elements visited when using a keyboard or gamepad
		 * to switch between the elements. If you don't set a navigation group the elements will inherit the default
		 * navigation group from their parent GUIWidget. Also see setNavGroupIndex().
		 */
		void SetNavigationGroup(const SPtr<GUINavGroup>& navGroup);

		/**
		 * Sets the index that determines in what order is the element visited compared to all the other elements in the
		 * nav-group. Elements with lower index will be visited before elements with a higher index. Elements with index
		 * 0 (the default) are special and will have their visit order determines by their position compared to other
		 * elements. The applied index is tied to the nav-group, so if the nav-group changes the index will need to be
		 * re-applied.
		 */
		void SetNavigationGroupIndex(i32 index);

		/**
		 * Checks is the specified position within interactable bounds of a GUI element. These are the bounds that will be used for hit tests for e.g. mouse cursor.
		 * By default this is the same as the absolute clipped bounds of the GUI element. Position is relative to parent GUI widget.
		 */
		virtual bool IsInInteractionBounds(const GUIPhysicalPoint& position) const;

		void Destroy() override;

		/**	Triggered when the element gains focus. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnFocusGained;

		/**	Triggered when the element loses focus. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnFocusLost;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Called when a mouse event is received on any GUI element the mouse is interacting with. Return true if you have
		 * processed the event and don't want other elements to process it.
		 */
		virtual bool DoOnMouseEvent(const GUIMouseEvent& event);

		/**
		 * Called when some text is input and the GUI element has input focus. Return true if you have processed the event
		 * and don't want other elements to process it.
		 */
		virtual bool DoOnTextInputEvent(const GUITextInputEvent& event);

		/**
		 * Called when a command event is triggered. Return true if you have processed the event and don't want other
		 * elements to process it.
		 */
		virtual bool DoOnCommandEvent(const GUICommandEvent& event);

		/**
		 * Called when a virtual button is pressed/released and the GUI element has input focus. Return true if you have
		 * processed the event and don't want other elements to process it.
		 */
		virtual bool DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& event);

		void ChangeParentWidget(GUIWidget* widget) override;

		/** Notifies the system the state flag was added or removed. */
		virtual void NotifyStateFlagsChanged();

		/** Returns the navigation group this element belongs to. See setNavGroup(). */
		SPtr<GUINavGroup> GetNavigationGroup() const;

		/** Transitions the GUI element into a new state by adding state flags. */
		void AddStateFlags(GUIElementStateFlags flags);

		/** Transitions the GUI element into a new state by removing state flags. */
		void RemoveStateFlags(GUIElementStateFlags flags);

		/**	Checks if the GUI element has a custom cursor and outputs the cursor type if it does. */
		virtual bool HasCustomCursor(const GUIPhysicalPoint& position, CursorType& type) const { return false; }

		/**	Checks if the GUI element accepts a drag and drop operation of the specified type. */
		virtual bool AcceptDragAndDrop(const GUIPhysicalPoint& position, u32 typeId) const { return false; }

		/**	Returns a context menu if a GUI element has one. Otherwise returns nullptr. */
		virtual SPtr<GUIContextMenu> GetContextMenu() const;

		/**	Returns text to display when hovering over the element. Returns empty string if no tooltip. */
		virtual String GetTooltip() const { return StringUtility::kBlank; }

		/** @} */

	protected:
		GUIElementOptions mOptionFlags;

	private:
		SPtr<GUIContextMenu> mContextMenu;
		SPtr<GUINavGroup> mNavigationGroup;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIInteractableRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
