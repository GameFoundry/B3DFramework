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
	class BS_EXPORT GUIElementContainer : public GUIElement
	{
	public:
		/** @copydoc GUIElement::setFocus */
		void setFocus(bool enabled, bool clear = false) override;

	protected:
		GUIElementContainer(const GUIDimensions& dimensions, const char* style,
			GUIElementOptions options = GUIElementOptions(0));
		GUIElementContainer(const GUIDimensions& dimensions, const String& style = StringUtil::BLANK, 
			GUIElementOptions options = GUIElementOptions(0));
		virtual ~GUIElementContainer() = default;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @copydoc GUIElement::_commandEvent */
		bool _commandEvent(const GUICommandEvent& ev) override;

		GUIElement* mFocusElement = nullptr;
	};

	/** @} */
}
