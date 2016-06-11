//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** GUI element representing an element with a draggable handle of a variable size. */
	class BS_EXPORT GUIScrollBar : public GUIElement
	{
	public:
		/**	Returns the position of the scroll handle in percent (ranging [0, 1]). */
		float getScrollPos() const;

		/**
		 * Moves the handle by some amount. Amount is specified in the percentage of the entire scrollable area. Values out
		 * of range will be clamped.
		 */
		void scroll(float amount);

		/**	Returns the maximum size of the scroll handle, in pixels. */
		UINT32 getMaxHandleSize() const;

		/**	Returns the maximum scrollable size the handle can move within (for example scroll bar length). */
		UINT32 getScrollableSize() const;

		/** @copydoc GUIElement::setTint */
		virtual void setTint(const Color& color) override;

		/**
		 * Triggered whenever the scrollbar handle is moved. Value provided is the handle position in percent 
		 * (ranging [0, 1]).
		 */
		Event<void(float newPosition)> onScrollPositionChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets the size of the handle in pixels.
		 *
		 * @note	Does not trigger layout update.
		 */
		void _setHandleSize(UINT32 size);

		/**
		 * Sets the position of the scroll handle in percent (ranging [0, 1]).
		 *
		 * @note	Does not trigger layout update.
		 */
		void _setScrollPos(float pct);

		/** @copydoc GUIElement::_getOptimalSize */
		virtual Vector2I _getOptimalSize() const override;

		/** @} */
	protected:
		/**
		 * Constructs a new scrollbar.
		 *
		 * @param[in]	horizontal	If true the scroll bar will have a horizontal moving handle, otherwise it will be a
		 *							vertical one.
		 * @param[in]	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 * @param[in]	dimensions	Determines valid dimensions (size) of the element.
		 */
		GUIScrollBar(bool horizontal, const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUIScrollBar();

		/** @copydoc GUIElement::_getNumRenderElements */
		UINT32 _getNumRenderElements() const override;

		/** @copydoc GUIElement::_getMaterial */
		const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const override;

		/** @copydoc GUIElement::_getMeshSize() */
		void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const override;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal */
		void updateRenderElementsInternal() override;

		/** @copydoc GUIElement::updateClippedBounds */
		void updateClippedBounds() override;

		/** @copydoc GUIElement::_getRenderElementDepth */
		UINT32 _getRenderElementDepth(UINT32 renderElementIdx) const override;

		/** @copydoc	GUIElement::_getRenderElementDepthRange */
		UINT32 _getRenderElementDepthRange() const override;
	private:
		/**
		 * Triggered whenever the scroll handle moves. Provided value represents the new position of the handle in percent
		 * (ranging [0, 1]).
		 */
		void handleMoved(float handlePct);

		/**	Triggered when scroll up button is clicked. */
		void upButtonClicked();

		/**	Triggered when scroll down button is clicked. */
		void downButtonClicked();

		GUILayout* mLayout;
		ImageSprite* mImageSprite;

		GUIButton* mUpBtn;
		GUIButton* mDownBtn;
		GUISliderHandle* mHandleBtn;
		bool mHorizontal;

		static const UINT32 ButtonScrollAmount;
	};

	/** @} */
}