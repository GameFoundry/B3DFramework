//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIClickable.h"
#include "2D/B3DImageSprite.h"
#include "2D/B3DTextSprite.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Structure describing contents of a GUIListBox element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIListBoxContent
	{
		GUIListBoxContent() = default;
		GUIListBoxContent(const Vector<HString>& elements, bool allowMultiselect = false)
			: Elements(elements), AllowMultiselect(allowMultiselect)
		{ }

		Vector<HString> Elements; /**< Elements to display in the list box. */
		bool AllowMultiselect = false; /**< Determines should the listbox allow multiple elements to be selected or just one. */
	};

	/** List box GUI element which when active opens a drop down selection with provided elements. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIListBox : public GUIClickable, public TGUIConstructionMethods<GUIListBox, GUIListBoxContent>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

		/**	Checks whether the listbox supports multiple selected elements at once. */
		B3D_SCRIPT_EXPORT(Property(Getter))
		bool IsMultiselect() const { return mIsMultiselect; }

		/**	Changes the list box elements. */
		B3D_SCRIPT_EXPORT()
		void SetElements(const Vector<HString>& elements);

		/**	Makes the element with the specified index selected. */
		B3D_SCRIPT_EXPORT()
		void SelectElement(u32 index);

		/**	Deselect element the element with the specified index. Only relevant for multi-select list boxes. */
		B3D_SCRIPT_EXPORT()
		void DeselectElement(u32 index);

		/**
		 * Returns the index of the currently selected element. If the list box allows multi-select, returns the index of the first
		 * selected element, or ~0u if none is selected.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(SelectedElementIndex))
		u32 GetSelectedElementIndex() const;

		/**	Returns states of all element in the list box (enabled or disabled). */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ElementStates))
		const Vector<bool>& GetElementStates() const { return mElementStates; }

		/**
		 * Sets states for all list box elements. Only valid for multi-select list boxes. Number of states must match number
		 * of list box elements.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(ElementStates))
		void SetElementStates(const Vector<bool>& states);

		/**
		 * Triggered whenever user selects or deselects an element in the list box. Returned index maps to the element in
		 * the elements array that the list box was initialized with.
		 */
		B3D_SCRIPT_EXPORT()
		Event<void(u32, bool)> OnSelectionToggled;

		static constexpr const char* kElementType = "listbox";
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct { };
		GUIListBox(PrivatelyConstruct, const GUIListBoxContent& content, const String& styleName, const GUISizeConstraints& sizeConstraints);

		const char* GetStyleSheetElement() const override { return kElementType; }

		/** @} */
	protected:
		~GUIListBox();

	private:
		GUILogicalArea GetContentBounds() const override;
		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;

		/** Returns the area in which the arrow box will be placed, relative to the GUI element. */
		GUIPhysicalArea GetArrowCachedContentBoundsInElementSpace() const;

		/** Returns the width/height of the arrow box. */
		GUILogicalSize GetArrowCachedContentSize() const;

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

		ImageSprite* mArrowSprite = nullptr;
		ImageSpriteInformation mArrowSpriteInformation;
		const IGUIVectorPathBuilder* mArrowPathBuilder = nullptr;
		u32 mArrowPseudoElementIndex = ~0u;

		bool mIsMultiselect;
	};

	/** @} */
} // namespace b3d
