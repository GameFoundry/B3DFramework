//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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

	/**	Specialization of a GUIScrollBar for horizontal scrolling. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIHorizontalScrollBar : public GUIScrollBar, public TGUIConstructionMethodsWithoutContent<GUIHorizontalScrollBar>
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
		GUIHorizontalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints);

		/** @} */
	protected:
		~GUIHorizontalScrollBar() = default;
	};

	/**	Specialization of a GUIScrollBar for horizontal scrolling, with the ability to resize the scroll bar. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIResizableHorizontalScrollBar : public GUIScrollBar, public TGUIConstructionMethodsWithoutContent<GUIResizableHorizontalScrollBar>
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
		GUIResizableHorizontalScrollBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints);

		/** @} */
	protected:
		~GUIResizableHorizontalScrollBar() = default;
	};

	/** @} */
} // namespace b3d
