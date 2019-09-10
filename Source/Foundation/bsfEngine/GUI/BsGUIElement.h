//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementBase.h"
#include "GUI/BsGUIOptions.h"
#include "2D/BsSprite.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"
#include "Image/BsColor.h"

namespace bs
{
	class GUINavGroup;

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Contains options that change GUIElement behaviour. */
	enum class GUIElementOption
	{
		/** 
		 * Enable this option if you want pointer events to pass through this element by default. This will allow elements
		 * underneath this element to receive pointer events.
		 */
		ClickThrough = 0x01,

		/** 
		 * Enable this option if the element accepts keyboard/gamepad input focus. This will allow the element to be
		 * navigated to using keys/buttons. 
		 */
		AcceptsKeyFocus = 0x02
	};

	typedef Flags<GUIElementOption> GUIElementOptions;
	BS_FLAGS_OPERATORS(GUIElementOption)

	/** Contains information about a single renderable element within a GUIElement. */
	struct GUIRenderElement : SpriteRenderElement
	{
		GUIMeshType type = GUIMeshType::Triangle;
		UINT32 depth = 0;
	};

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
		GUIElement(String styleName, const GUIDimensions& dimensions, GUIElementOptions options = GUIElementOptions(0));
		GUIElement(const char* styleName, const GUIDimensions& dimensions, GUIElementOptions options = GUIElementOptions(0));
		virtual ~GUIElement() = default;

		/**	
		 * Change the GUI element focus state. 
		 * 
		 * @param[in]	enabled		Give the element focus or take it away.
		 * @param[in]	clear		If true the focus will be cleared from any elements currently in focus. Otherwise
		 *							the element will just be appended to the in-focus list (if enabling focus).
		 */
		virtual void setFocus(bool enabled, bool clear = false);

		/**	Sets the tint of the GUI element. */
		virtual void setTint(const Color& color);

		/** @copydoc GUIElementBase::resetDimensions */
		void resetDimensions() override;

		/**	Sets new style to be used by the element. */
		void setStyle(const String& styleName);

		/**	Returns the name of the style used by this element. */
		const String& getStyleName() const { return mStyleName; }

		/** A set of flags controlling various aspects of the GUIElement. See GUIElementOptions.  */
		void setOptionFlags(GUIElementOptions options) { mOptionFlags = options; }

		/** @copydoc setOptionFlags */
		GUIElementOptions getOptionFlags() const { return mOptionFlags; }

		/**
		 * Assigns a new context menu that will be opened when the element is right clicked. Null is allowed in case no
		 * context menu is wanted.
		 */
		void setContextMenu(const SPtr<GUIContextMenu>& menu) { mContextMenu = menu; }

		/** 
		 * Sets a navigation group that determines in what order are GUI elements visited when using a keyboard or gamepad
		 * to switch between the elements. If you don't set a navigation group the elements will inherit the default
		 * navigation group from their parent GUIWidget. Also see setNavGroupIndex().
		 */
		void setNavGroup(const SPtr<GUINavGroup>& navGroup);

		/**
		 * Sets the index that determines in what order is the element visited compared to all the other elements in the
		 * nav-group. Elements with lower index will be visited before elements with a higher index. Elements with index
		 * 0 (the default) are special and will have their visit order determines by their position compared to other
		 * elements. The applied index is tied to the nav-group, so if the nav-group changes the index will need to be
		 * re-applied.
		 */
		void setNavGroupIndex(INT32 index);

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
		 * Returns information about all renderable elements in this GUI element, including their mesh, material and
		 * general information.
		 */
		const SmallVector<GUIRenderElement, 4>& _getRenderElements() const { return mRenderElements; }

		/**
		 * Fill the pre-allocated vertex, uv and index buffers with the mesh data for the specified render element.
		 * 			
		 * @param[out]	vertices			Previously allocated buffer where to store the vertices. Output is expected
		 *									to match the GUIMeshType as returned by getRenderElements() for the specified
		 *									element.
		 * @param[out]	indices				Previously allocated buffer where to store the indices.
		 * @param[in]	vertexOffset		At which vertex should the method start filling the buffer.
		 * @param[in]	offset				Offset that should be applied to all output vertex positions.
		 * @param[in]	indexOffset			At which index should the method start filling the buffer.
		 * @param[in]	maxNumVerts			Total number of vertices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param[in]	maxNumIndices		Total number of indices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param[in]	renderElementIdx	Zero-based index of the render element.
		 *
		 */
		virtual void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const = 0;

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

		/** Returns the navigation group this element belongs to. See setNavGroup(). */
		SPtr<GUINavGroup> _getNavGroup() const;

		/** Checks is the specified position within GUI element bounds. Position is relative to parent GUI widget. */
		virtual bool _isInBounds(const Vector2I position) const;

		/**	Checks if the GUI element has a custom cursor and outputs the cursor type if it does. */
		virtual bool _hasCustomCursor(const Vector2I position, CursorType& type) const { return false; }

		/**	Checks if the GUI element accepts a drag and drop operation of the specified type. */
		virtual bool _acceptDragAndDrop(const Vector2I position, UINT32 typeId) const { return false; }

		/**	Returns a context menu if a GUI element has one. Otherwise returns nullptr. */
		virtual SPtr<GUIContextMenu> _getContextMenu() const;

		/**	Returns text to display when hovering over the element. Returns empty string if no tooltip. */
		virtual String _getTooltip() const { return StringUtil::BLANK; }

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

		bool mIsDestroyed = false;
		GUIElementOptions mOptionFlags;
		Rect2I mClippedBounds;
		SmallVector<GUIRenderElement, 4> mRenderElements;
		
	private:
		static const Color DISABLED_COLOR;

		const GUIElementStyle* mStyle;
		String mStyleName;

		SPtr<GUIContextMenu> mContextMenu;
		SPtr<GUINavGroup> mNavGroup;
		Color mColor;
	};

	/** @} */

	/** @cond IMPLEMENTATION */
	namespace impl
	{
		/** Helper class used for populating GUIRenderElement information from Sprite objects. */
		struct GUIRenderElementHelper
		{
			/**
			 * Contains the sprite to generate render element data for, as well as additional data not provided in the
			 * sprite itself.
			 */
			struct SpriteInfo
			{
				SpriteInfo(Sprite* sprite, UINT32 depth = 0, GUIMeshType meshType = GUIMeshType::Triangle)
					: sprite(sprite), depth(depth), meshType(meshType)
				{ }
				
				Sprite* sprite;
				UINT32 depth = 0;
				GUIMeshType meshType = GUIMeshType::Triangle;
			};

			/**
			 * Determines the total number of requires render elements from the provided set of sprites, and initializes that
			 * many render elements from the sprite render elements and the extra information provided in SpriteInfo.
			 */
			template<UINT32 N>
			static void populate(const SpriteInfo (&spriteInfos)[N], SmallVector<GUIRenderElement, 4>& output)
			{
				UINT32 totalCount = 0;
				for (UINT32 i = 0; i < N; i++)
					totalCount += spriteInfos[i].sprite ? spriteInfos[i].sprite->getNumRenderElements() : 0;

				output.resize(totalCount);

				UINT32 globalIdx = 0;
				for (UINT32 i = 0; i < N; i++)
				{
					const SpriteInfo& spriteInfo = spriteInfos[i];
					
					UINT32 count = spriteInfo.sprite ? spriteInfo.sprite->getNumRenderElements() : 0;
					for(UINT32 j = 0; j < count; j++)
					{
						GUIRenderElement& renderElem = output[globalIdx];
						spriteInfo.sprite->getRenderElementInfo(j, renderElem);

						renderElem.depth = spriteInfo.depth;
						renderElem.type = spriteInfo.meshType;
						
						globalIdx++;
					}
				}
			}
		};
	}

	/** @endcond */
}
