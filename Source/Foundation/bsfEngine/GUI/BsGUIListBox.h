//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIButtonBase.h"
#include "2D/BsImageSprite.h"
#include "2D/BsTextSprite.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** List box GUI element which when active opens a drop down selection with provided elements. */
	class BS_EXPORT GUIListBox : public GUIButtonBase
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new listbox with the provided elements.
		 *
		 * @param[in]	elements		Elements to display in the list box.
		 * @param[in]	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIListBox* Create(const Vector<HString>& elements, bool multiselect = false, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new listbox with the provided elements.
		 *
		 * @param[in]	elements		Elements to display in the list box.
		 * @param[in]	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIListBox* Create(const Vector<HString>& elements, bool multiselect, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new single-select listbox with the provided elements.
		 *
		 * @param[in]	elements		Elements to display in the list box.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIListBox* Create(const Vector<HString>& elements, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**	Checks whether the listbox supports multiple selected elements at once. */
		bool IsMultiselect() const { return mIsMultiselect; }

		/**	Changes the list box elements. */
		void SetElements(const Vector<HString>& elements);

		/**	Makes the element with the specified index selected. */
		void SelectElement(u32 idx);

		/**	Deselect element the element with the specified index. Only relevant for multi-select list boxes. */
		void DeselectElement(u32 idx);

		/**	Returns states of all element in the list box (enabled or disabled). */
		const Vector<bool>& GetElementStates() const { return mElementStates; }

		/**
		 * Sets states for all list box elements. Only valid for multi-select list boxes. Number of states must match number
		 * of list box elements.
		 */
		void SetElementStates(const Vector<bool>& states);

		/**
		 * Triggered whenever user selects or deselects an element in the list box. Returned index maps to the element in
		 * the elements array that the list box was initialized with.
		 */
		Event<void(u32, bool)> OnSelectionToggled;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIButtonBase::_getElementType */
		ElementType GetElementTypeInternal() const override { return ElementType::ListBox; }

		/** @} */
	protected:
		~GUIListBox();

	private:
		GUIListBox(const String& styleName, const Vector<HString>& elements, bool isMultiselect, const GUIDimensions& dimensions);

		/** @copydoc GUIButtonBase::_mouseEvent */
		bool MouseEventInternal(const GUIMouseEvent& ev) override;

		/** @copydoc GUIButtonBase::_commandEvent */
		bool CommandEventInternal(const GUICommandEvent& ev) override;

		/**	Triggered when user clicks on an element. */
		void ElementSelected(u32 idx);

		/**	Opens the list box drop down menu. */
		void OpenListBox();

		/**	Closes the list box drop down menu. */
		void CloseListBox();

		/** Called when the list box drop down menu is closed by external influence. */
		void OnListBoxClosed();

		/**	Updates visible contents depending on selected element(s). */
		void UpdateContents();

	private:
		Vector<HString> mElements;
		Vector<bool> mElementStates;
		GameObjectHandle<GUIDropDownMenu> mDropDownBox;

		bool mIsMultiselect;
	};

	/** @} */
} // namespace bs
