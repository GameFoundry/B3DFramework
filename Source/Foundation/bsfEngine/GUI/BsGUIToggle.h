//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIToggleable.h"
#include "BsGUIConstructionMethods.h"

namespace bs
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
		GUIToggle(PrivatelyConstruct, const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& dimensions);

		ElementType GetElementType() const override { return ElementType::Toggle; }

		/** @} */
	protected:
		~GUIToggle() override = default;
	};

	/** @} */
} // namespace bs
