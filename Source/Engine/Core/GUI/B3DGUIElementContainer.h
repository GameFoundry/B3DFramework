//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Base for GUI elements that combine multiple GUI elements into one more complex element. */
	class B3D_EXPORT GUIElementContainer : public GUIInteractable
	{
	public:
		virtual ~GUIElementContainer() = default;

		/** @copydoc GUIElement::SetFocus */
		void SetFocus(bool enabled, bool clear = false);

		const char* GetStyleSheetElement() const override { return "container"; }

	protected:
		GUIElementContainer(const GUISizeConstraints& dimensions, const char* style, GUIElementOptions options = GUIElementOptions(0));
		GUIElementContainer(const GUISizeConstraints& dimensions, const String& style = StringUtil::kBlank, GUIElementOptions options = GUIElementOptions(0));

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;

		GUIInteractable* mFocusElement = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIElementContainerRTTI;

		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
