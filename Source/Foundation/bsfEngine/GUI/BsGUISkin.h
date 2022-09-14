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
	class BS_EXPORT BS_SCRIPT_EXPORT(m:GUI) GUISkin : public Resource
	{
	public:
		/**	Checks if the style with the specified name exists. */
		BS_SCRIPT_EXPORT()
		bool hasStyle(const String& name) const;

		/**
		 * Returns a style for the specified GUI element type.
		 *
		 * @see		GUIElement::getGUITypeName
		 */
		BS_SCRIPT_EXPORT()
		const GUIElementStyle* getStyle(const String& guiElemType) const;

		/**
		 * Sets a style for the specified GUI element type.
		 *
		 * @see		GUIElement::getGUITypeName
		 */
		BS_SCRIPT_EXPORT()
		void setStyle(const String& guiElemType, const GUIElementStyle& style);

		/**
		 * Removes a style for the specified GUI element type.
		 *
		 * @see		GUIElement::getGUITypeName
		 */
		BS_SCRIPT_EXPORT()
		void removeStyle(const String& guiElemType);

		/**	Returns names of all styles registered on this skin. */
		BS_SCRIPT_EXPORT(pr:getter,n:StyleNames)
		Vector<String> getStyleNames() const;

		/**	Creates an empty GUI skin and returns a handle to it. */
		BS_SCRIPT_EXPORT(ec:GUISkin)
		static HGUISkin create();

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
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
