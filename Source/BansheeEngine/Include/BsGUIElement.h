//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElementBase.h"
#include "BsGUIOptions.h"
#include "BsRect2I.h"
#include "BsVector2I.h"
#include "BsColor.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * Represents parent class for all visible GUI elements. Contains methods needed for positioning, rendering and
	 * handling input.
	 */
	class BS_EXPORT GUIElement : public GUIElementBase
	{
	public:
		/**	Different sub-types of GUI elements. */
		enum class ElementType
		{
			Label,
			Button,
			Toggle,
			Texture,
			InputBox,
			ListBox,
			ScrollArea,
			Layout,
			Undefined
		};

	public:
		GUIElement(const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUIElement();

		/**	Sets or removes focus from an element. Will change element style. */
		void setFocus(bool enabled);

		/**	Sets the tint of the GUI element. */
		virtual void setTint(const Color& color);

		/** @copydoc GUIElementBase::resetDimensions */
		void resetDimensions() override;

		/**	Sets new style to be used by the element. */
		void setStyle(const String& styleName);

		/**	Returns the name of the style used by this element. */
		const String& getStyleName() const { return mStyleName; }

		/** 
		 * Determines will this element block elements underneath it from receiving pointer events (clicks, focus 
		 * gain/lost, hover on/off, etc.). Enabled by default.
		 */
		void setBlockPointerEvents(bool block) { mBlockPointerEvents = block; }

		/** @copydoc setBlockPointerEvents */
		bool getBlockPointerEvents() const { return mBlockPointerEvents; }

		/**
		 * Assigns a new context menu that will be opened when the element is right clicked. Null is allowed in case no
		 * context menu is wanted.
		 */
		void setContextMenu(const SPtr<GUIContextMenu>& menu) { mContextMenu = menu; }

		/** @copydoc GUIElementBase::getVisibleBounds */
		Rect2I getVisibleBounds() override;

		/**
		 * Destroy the element. Removes it from parent and widget, and queues it for deletion. Element memory will be 
		 * released delayed, next frame.
		 */	
		static void destroy(GUIElement* element);

		/**	Triggered when the element loses or gains focus. */
		Event<void(bool)> onFocusChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns the number of separate render elements in the GUI element.
		 * 			
		 * @return	The number render elements.
		 *
		 * @note	
		 * GUI system attempts to reduce the number of GUI meshes so it will group sprites based on their material and 
		 * textures. One render elements represents a group of such sprites that share a material/texture.
		 */
		virtual UINT32 _getNumRenderElements() const = 0;

		/**
		 * Gets a material for the specified render element index.
		 * 		
		 * @return	Handle to the material.
		 *
		 * @see		_getNumRenderElements()
		 */
		virtual const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const = 0;

		/**
		 * Returns the type of mesh and number of vertices and indices that the specified render element will use. You will
		 * need this value when creating the buffers before calling _fillBuffer().
		 *
		 * @see		_getNumRenderElements()
		 * @see		_fillBuffer()
		 */
		virtual void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const = 0;

		/**
		 * Fill the pre-allocated vertex, uv and index buffers with the mesh data for the specified render element.
		 * 			
		 * @param[out]	vertices			Previously allocated buffer where to store the vertices. Output is expected
		 *									to match the GUIMeshType as returned by _getMeshInfo.
		 * @param[out]	indices				Previously allocated buffer where to store the indices.
		 * @param[in]	vertexOffset		At which vertex should the method start filling the buffer.
		 * @param[in]	indexOffset			At which index should the method start filling the buffer.
		 * @param[in]	maxNumVerts			Total number of vertices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param[in]	maxNumIndices		Total number of indices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param[in]	renderElementIdx	Zero-based index of the render element.
		 *
		 * @see		_getNumRenderElements()
		 * @see		_getMeshSize()
		 */
		virtual void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const = 0;

		/**
		 * Recreates the internal render elements. Must be called before fillBuffer if element is dirty. Marks the element
		 * as non dirty.
		 */
		void _updateRenderElements();

		/** Gets internal element style representing the exact type of GUI element in this object. */
		virtual ElementType _getElementType() const { return ElementType::Undefined; }

		/**
		 * Called when a mouse event is received on any GUI element the mouse is interacting with. Return true if you have
		 * processed the event and don't want other elements to process it.
		 */
		virtual bool _mouseEvent(const GUIMouseEvent& ev);

		/**
		 * Called when some text is input and the GUI element has input focus. Return true if you have processed the event
		 * and don't want other elements to process it.
		 */	
		virtual bool _textInputEvent(const GUITextInputEvent& ev);

		/**
		 * Called when a command event is triggered. Return true if you have processed the event and don't want other 
		 * elements to process it.
		 */
		virtual bool _commandEvent(const GUICommandEvent& ev);

		/**
		 * Called when a virtual button is pressed/released and the GUI element has input focus. Return true if you have
		 * processed the event and don't want other elements to process it.
		 */
		virtual bool _virtualButtonEvent(const GUIVirtualButtonEvent& ev);

		/** Set element part of element depth. Less significant than both widget and area depth. */
		void _setElementDepth(UINT8 depth);

		/** Retrieve element part of element depth. Less significant than both widget and area depth. */
		UINT8 _getElementDepth() const;

		/** @copydoc GUIElementBase::_setLayoutData */
		void _setLayoutData(const GUILayoutData& data) override;

		/** @copydoc GUIElementBase::_changeParentWidget */
		void _changeParentWidget(GUIWidget* widget) override;

		/**
		 * Returns depth for a specific render element. This contains a combination of widget depth (8 bit(, area depth
		 * (16 bit) and render element depth (8 bit).
		 *
		 * @see		_getNumRenderElements
		 */
		virtual UINT32 _getRenderElementDepth(UINT32 renderElementIdx) const { return _getDepth(); }

		/**
		 * Returns the range of depths that the child elements can be rendered it.
		 *
		 * @note	
		 * For example if you are rendering a button with an image and a text you will want the text to be rendered in front
		 * of the image at a different depth, which means the depth range is 2 (0 for text, 1 for background image).
		 */
		virtual UINT32 _getRenderElementDepthRange() const { return 1; }

		/** Gets internal element style representing the exact type of GUI element in this object. */
		Type _getType() const override { return GUIElementBase::Type::Element; }

		/** Checks if element has been destroyed and is queued for deletion. */
		bool _isDestroyed() const override { return mIsDestroyed; }

		/** Update element style based on active GUI skin and style name. */
		void _refreshStyle();

		/**	Gets the currently active element style. */
		const GUIElementStyle* _getStyle() const { return mStyle; }

		/**	Gets GUI element bounds relative to parent widget, clipped by specified clip rect. */
		const Rect2I& _getClippedBounds() const { return mClippedBounds; }

		/** 
		 * Returns GUI element padding. Padding is modified by changing element style and determines minimum distance 
		 * between different GUI elements. 
		 */
		const RectOffset& _getPadding() const override;

		/**
		 * Returns GUI element depth. This includes widget and area depth, but does not include specific per-render-element
		 * depth.
		 */
		UINT32 _getDepth() const { return mLayoutData.depth; }

		/** Checks is the specified position within GUI element bounds. Position is relative to parent GUI widget. */
		virtual bool _isInBounds(const Vector2I position) const;

		/**	Checks if the GUI element has a custom cursor and outputs the cursor type if it does. */
		virtual bool _hasCustomCursor(const Vector2I position, CursorType& type) const { return false; }

		/**	Checks if the GUI element accepts a drag and drop operation of the specified type. */
		virtual bool _acceptDragAndDrop(const Vector2I position, UINT32 typeId) const { return false; }

		/**	Returns a context menu if a GUI element has one. Otherwise returns nullptr. */
		virtual SPtr<GUIContextMenu> _getContextMenu() const;

		/**	Returns text to display when hovering over the element. Returns empty string if no tooltip. */
		virtual WString _getTooltip() const { return StringUtil::WBLANK; }

		/**	Returns a clip rectangle relative to the element, used for offsetting the input text. */
		virtual Vector2I _getTextInputOffset() const { return Vector2I(); }

		/**	Returns a clip rectangle relative to the element, used for clipping	the input text. */
		virtual Rect2I _getTextInputRect() const { return Rect2I(); }

		/** @} */

	protected:
		/**	Called whenever render elements are dirty and need to be rebuilt. */
		virtual void updateRenderElementsInternal();

		/**
		 * Called whenever element clipped bounds need to be recalculated. (for example when width, height or clip 
		 * rectangles changes).
		 */
		virtual void updateClippedBounds();

		/**
		 * Helper method that returns style name used by an element of a certain type. If override style is empty, default
		 * style for that type is returned.
		 */
		template<class T>
		static const String& getStyleName(const String& overrideStyle)
		{
			if(overrideStyle == StringUtil::BLANK)
				return T::getGUITypeName();

			return overrideStyle;
		}

		/**
		 * Attempts to find a sub-style for the specified type in the currently set GUI element style. If one cannot be
		 * found empty string is returned.
		 */
		const String& getSubStyleName(const String& subStyleTypeName) const;

		/**	Method that gets triggered whenever element style changes. */
		virtual void styleUpdated() { }

		/**	Returns clipped bounds excluding the margins. Relative to parent widget. */
		Rect2I getCachedVisibleBounds() const;

		/**	Returns bounds of the content contained within the GUI element. Relative to parent widget. */
		Rect2I getCachedContentBounds() const;

		/**
		 * Returns a clip rectangle that can be used for clipping the contents of this GUI element. Clip rect is relative
		 * to GUI element origin.
		 */
		Rect2I getCachedContentClipRect() const;

		/**	Returns the tint that is applied to the GUI element. */
		Color getTint() const;

		bool mIsDestroyed;
		bool mBlockPointerEvents;
		Rect2I mClippedBounds;
		
	private:
		static const Color DISABLED_COLOR;

		const GUIElementStyle* mStyle;
		String mStyleName;

		SPtr<GUIContextMenu> mContextMenu;
		Color mColor;
	};

	/** @} */
}