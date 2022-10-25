//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Resources/BsResource.h"
#include "GUI/BsGUIElementStyle.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Holds a set of styles that control how are GUI element types positioned and displayed in the GUI. Each element type
	 * can be assigned a specific style.
	 */
	class BS_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUISkin : public Resource
	{
	public:
		/**	Checks if the style with the specified name exists. */
		BS_SCRIPT_EXPORT()
		bool HasStyle(const String& name) const;

		/**
		 * Returns a style for the specified GUI element type.
		 *
		 * @see		GUIElement::getGUITypeName
		 */
		BS_SCRIPT_EXPORT()
		const GUIElementStyle* GetStyle(const String& guiElemType) const;

		/**
		 * Sets a style for the specified GUI element type.
		 *
		 * @see		GUIElement::getGUITypeName
		 */
		BS_SCRIPT_EXPORT()
		void SetStyle(const String& guiElemType, const GUIElementStyle& style);

		/**
		 * Removes a style for the specified GUI element type.
		 *
		 * @see		GUIElement::getGUITypeName
		 */
		BS_SCRIPT_EXPORT()
		void RemoveStyle(const String& guiElemType);

		/**	Returns names of all styles registered on this skin. */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(StyleNames))
		Vector<String> GetStyleNames() const;

		/**	Creates an empty GUI skin and returns a handle to it. */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(GUISkin))
		static HGUISkin Create();

		/**	Default style that may be used when no other is available. */
		static GUIElementStyle DefaultStyle;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates an empty GUI skin and returns a pointer to it.
		 *
		 * @note	Internal method. Use "create" returning handle for normal use.
		 */
		static SPtr<GUISkin> CreatePtrInternal();

		/** @} */
	private:
		GUISkin();
		GUISkin(const GUISkin& skin); // Disable copying

		UnorderedMap<String, GUIElementStyle> mStyles;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUISkinRTTI;
		static RTTITypeBase* GetRttiStatic();
		virtual RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
