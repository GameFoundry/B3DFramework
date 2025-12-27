//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIToggleable.h"
#include "B3DGUIConstructionMethods.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	GUI element representing a toggle (on/off) button. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIToggle : public GUIToggleable, public TGUIConstructionMethods<GUIToggle, GUIToggleContent>
	{
		using Super = GUIClickable;
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct {};
		GUIToggle(PrivatelyConstruct, const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& sizeConstraints);

		/** @} */
	protected:
		~GUIToggle() override = default;
	};

	/** @} */
} // namespace b3d
