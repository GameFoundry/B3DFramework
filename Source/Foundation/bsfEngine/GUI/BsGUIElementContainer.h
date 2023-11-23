//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Base for GUI elements that combine multiple GUI elements into one more complex element. */
	class B3D_EXPORT GUIElementContainer : public GUIElement
	{
	public:
		/** @copydoc GUIElement::SetFocus */
		void SetFocus(bool enabled, bool clear = false);

		const char* GetStyleSheetElement() const override { return "container"; }

	protected:
		GUIElementContainer(const GUISizeConstraints& dimensions, const char* style, GUIElementOptions options = GUIElementOptions(0));
		GUIElementContainer(const GUISizeConstraints& dimensions, const String& style = StringUtil::kBlank, GUIElementOptions options = GUIElementOptions(0));
		virtual ~GUIElementContainer() = default;

		void FillBuffer(u8* vertices, u32* indices, u32 vertexOffset, u32 indexOffset, const Vector2I& offset, u32 maxNumVerts, u32 maxNumIndices, u32 renderElementIdx) const override;
		Vector2I CalculateUnconstrainedOptimalSize() const override;
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;

		GUIElement* mFocusElement = nullptr;
	};

	/** @} */
} // namespace bs
