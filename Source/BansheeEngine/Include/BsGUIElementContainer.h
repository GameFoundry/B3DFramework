//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"

namespace BansheeEngine
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Base for GUI elements that combine multiple GUI elements into one more complex element. */
	class BS_EXPORT GUIElementContainer : public GUIElement
	{
	protected:
		GUIElementContainer(const GUIDimensions& dimensions, const String& style = StringUtil::BLANK);
		virtual ~GUIElementContainer();

		/** @copydoc GUIElement::_getNumRenderElements */
		UINT32 _getNumRenderElements() const override;

		/** @copydoc GUIElement::_getMaterial */
		const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const override;

		/** @copydoc GUIElement::_getMeshSize() */
		void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const override;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @copydoc GUIElement::_commandEvent */
		bool _commandEvent(const GUICommandEvent& event) override;
	};

	/** @} */
}