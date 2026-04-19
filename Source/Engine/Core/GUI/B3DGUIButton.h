//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIClickable.h"
#include "GUI/B3DGUIContent.h"
#include "B3DGUIConstructionMethods.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** GUI button that can be clicked. Has normal, hover and active states with an optional label. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIButton : public GUIClickable, public TGUIConstructionMethods<GUIButton, GUIContent>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct { };
		GUIButton(PrivatelyConstruct, const GUIContent& content, const String& styleClass, const GUISizeConstraints& dimensions);

		/** @} */
	private:
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;
	};

	/** @} */
} // namespace b3d
