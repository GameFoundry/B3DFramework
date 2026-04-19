//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIScrollBar.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Specialization of a GUIScrollBar for vertical scrolling. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIVerticalScrollBar : public GUIScrollBar, public TGUIConstructionMethodsWithoutContent<GUIVerticalScrollBar>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

	public:
		/**
		 * @name Internal
		 * @{
		 */

		struct PrivatelyConstruct {};
		GUIVerticalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints);

		/** @} */
	protected:
		~GUIVerticalScrollBar() = default;
	};

	/**	Specialization of a GUIScrollBar for vertical scrolling, with the ability to resize the scroll bar. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIResizableVerticalScrollBar : public GUIScrollBar, public TGUIConstructionMethodsWithoutContent<GUIResizableVerticalScrollBar>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

	public:
		/**
		 * @name Internal
		 * @{
		 */

		struct PrivatelyConstruct {};
		GUIResizableVerticalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints);

		/** @} */
	protected:
		~GUIResizableVerticalScrollBar() = default;
	};

	/** @} */
} // namespace b3d
