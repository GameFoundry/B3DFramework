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
	struct GUIStyleSheetRule;
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
	B3D_FLAGS_OPERATORS(GUIElementOption)

	/** Flags that determine the state that a GUI element may be in. */
	enum class GUIElementStateFlag
	{
		Normal = 0,
		Hover = 1 << 0,
		Active = 1 << 1,
		Focused = 1 << 2,
		Disabled = 1 << 3,
		Checked = 1 << 4,
		Count = 5
	};

	using GUIElementStateFlags = Flags<GUIElementStateFlag>;
	B3D_FLAGS_OPERATORS(GUIElementStateFlag)

	/** Contains information about a single renderable element within a GUIElement. */
	struct GUIRenderElement : SpriteRenderElement
	{
		GUIMeshType Type = GUIMeshType::Triangle;
		u32 Depth = 0;
		Vector2 Offset = Vector2::kZero; /**< Offset to apply to every vertex in the render element, relative to parent GUI element. */
		Size2 ClipSize = Size2::kZero; /**< Size of the clip rectangle, relative to the offset. Any vertices outside of this area will be clipped. */
		bool UseNewFillBuffer = false;
	};

	/**
	 * Represents parent class for all visible GUI elements. Contains methods needed for positioning, rendering and
	 * handling input.
	 */
	class B3D_EXPORT GUIElement : public GUIElementBase
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
		GUIElement(String styleName, const GUISizeConstraints& dimensions, GUIElementOptions options = GUIElementOptions(0));
		GUIElement(const char* styleName, const GUISizeConstraints& dimensions, GUIElementOptions options = GUIElementOptions(0));
		~GUIElement() override = default;

		const String& GetStyleSheetClass() const override { return mStyleName; }

		/**
		 * Change the GUI element focus state.
		 *
		 * @param[in]	enabled		Give the element focus or take it away.
		 * @param[in]	clear		If true the focus will be cleared from any elements currently in focus. Otherwise
		 *							the element will just be appended to the in-focus list (if enabling focus).
		 */
		virtual void SetFocus(bool enabled, bool clear = false);

		/**	Sets the tint of the GUI element. */
		virtual void SetTint(const Color& color);

		void ResetDimensions() override;

		/**	Sets new style to be used by the element. */
		void SetStyle(const String& styleName);

		/**	Returns the name of the style used by this element. */
		const String& GetStyleName() const { return mStyleName; }

		/** A set of flags controlling various aspects of the GUIElement. See GUIElementOptions.  */
		void SetOptionFlags(GUIElementOptions options) { mOptionFlags = options; }

		/** @copydoc SetOptionFlags */
		GUIElementOptions GetOptionFlags() const { return mOptionFlags; }

		/**
		 * Assigns a new context menu that will be opened when the element is right clicked. Null is allowed in case no
		 * context menu is wanted.
		 */
		void SetContextMenu(const SPtr<GUIContextMenu>& menu) { mContextMenu = menu; }

		/**
		 * Sets a navigation group that determines in what order are GUI elements visited when using a keyboard or gamepad
		 * to switch between the elements. If you don't set a navigation group the elements will inherit the default
		 * navigation group from their parent GUIWidget. Also see setNavGroupIndex().
		 */
		void SetNavigationGroup(const SPtr<GUINavGroup>& navGroup);

		/**
		 * Sets the index that determines in what order is the element visited compared to all the other elements in the
		 * nav-group. Elements with lower index will be visited before elements with a higher index. Elements with index
		 * 0 (the default) are special and will have their visit order determines by their position compared to other
		 * elements. The applied index is tied to the nav-group, so if the nav-group changes the index will need to be
		 * re-applied.
		 */
		void SetNavigationGroupIndex(i32 index);

		/**
		 * Destroy the element. Removes it from parent and widget, and queues it for deletion. Element memory will be
		 * released delayed, next frame.
		 */
		static void Destroy(GUIElement* element);

		/**	Triggered when the element loses or gains focus. */
		Event<void(bool)> OnFocusChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns information about all renderable elements in this GUI element, including their mesh, material and
		 * general information.
		 */
		const TInlineArray<GUIRenderElement, 4>& GetRenderElements() const { return mRenderElements; }

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
		virtual void FillBuffer(
			u8* vertices,
			u32* indices,
			u32 vertexOffset,
			u32 indexOffset,
			const Vector2I& offset,
			u32 maxNumVerts,
			u32 maxNumIndices,
			u32 renderElementIdx) const = 0;

		/**
		 * Retrieves vertex and index data from GUI render element and outputs them to the provided buffers. GUI render
		 * elements must have been previously populated by calling UpdateRenderElements().
		 *
		 * @param	renderElementIndex	Zero-based index of the render element from which to retrieve the data.
		 * @param	vertexOffset		At which vertex should the method start writing to the output position/uv buffer.
		 * @param	indexOffset			At which index should the method start writing to the output index buffer.
		 * @param	outPositions		Previously allocated buffer where to store the vertex positions. Caller must ensure
		 *								size and data type match the mesh type and vertex count retrieved from
		 *								GetRenderElements() for the specified element.
		 * @param	outUVs				Previously allocated buffer where to store the vertex UVs. Caller must ensure
		 *								size and data type match the mesh type and vertex count retrieved from
		 *								GetRenderElements() for the specified element. Can be null if not needed.
		 * @param	outIndices			Previously allocated buffer where to store the indices. Caller must ensure
		 *								size and data type match the mesh type and index count retrieved from
		 *								GetRenderElements() for the specified element. 
		 */
		virtual void GetRenderElementVertexAndIndexData(
			u32 renderElementIndex,
			u32 vertexOffset,
			u32 indexOffset,
			DataRange& outPositions,
			DataRange& outUVs,
			DataRange& outIndices
		) const;

		/**
		 * Recreates the internal render elements. Must be called before fillBuffer if element is dirty. Marks the element
		 * as non dirty.
		 */
		virtual void UpdateRenderElements();

		/** Gets internal element style representing the exact type of GUI element in this object. */
		virtual ElementType GetElementType() const { return ElementType::Undefined; }

		/**
		 * Called when a mouse event is received on any GUI element the mouse is interacting with. Return true if you have
		 * processed the event and don't want other elements to process it.
		 */
		virtual bool DoOnMouseEvent(const GUIMouseEvent& event);

		/**
		 * Called when some text is input and the GUI element has input focus. Return true if you have processed the event
		 * and don't want other elements to process it.
		 */
		virtual bool DoOnTextInputEvent(const GUITextInputEvent& event);

		/**
		 * Called when a command event is triggered. Return true if you have processed the event and don't want other
		 * elements to process it.
		 */
		virtual bool DoOnCommandEvent(const GUICommandEvent& event);

		/**
		 * Called when a virtual button is pressed/released and the GUI element has input focus. Return true if you have
		 * processed the event and don't want other elements to process it.
		 */
		virtual bool DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& event);

		/** Set element part of element depth. Less significant than both widget and area depth. */
		void SetElementDepth(u8 depth);

		/** Retrieve element part of element depth. Less significant than both widget and area depth. */
		u8 GetElementDepth() const;

		void SetLayoutData(const GUILayoutData& data) override;
		void ChangeParentWidget(GUIWidget* widget) override;

		/**
		 * Returns the range of depths that the child elements can be rendered it.
		 *
		 * @note
		 * For example if you are rendering a button with an image and a text you will want the text to be rendered in front
		 * of the image at a different depth, which means the depth range is 2 (0 for text, 1 for background image).
		 */
		virtual u32 GetRenderElementDepthRange() const { return 1; }

		/** Gets internal element style representing the exact type of GUI element in this object. */
		Type GetType() const override { return GUIElementBase::Type::Element; }

		/** Checks if element has been destroyed and is queued for deletion. */
		bool IsDestroyed() const override { return mIsDestroyed; }

		/** Update element style based on active GUI skin and style name. */
		void NotifyStyleSheetChanged();

		/**	Gets the currently active element style. */
		const GUIElementStyle* GetStyle() const { return mStyle; }

		/** Similar to GetCachedBounds(), except the bounds are clipped against the current clip rectangle. */
		const Rect2I& GetCachedClippedBounds() const { return mClippedBounds; }

		const RectOffset& GetMargins() const override;
		const RectOffset& GetPadding() const override;

		/**
		 * Returns GUI element depth. This includes widget and area depth, but does not include specific per-render-element
		 * depth.
		 */
		u32 GetDepth() const { return mLayoutData.Depth; }

		/** Returns the navigation group this element belongs to. See setNavGroup(). */
		SPtr<GUINavGroup> GetNavigationGroup() const;

		/** Checks is the specified position within GUI element bounds. Position is relative to parent GUI widget. */
		virtual bool IsInBounds(const Vector2I position) const;

		/**	Checks if the GUI element has a custom cursor and outputs the cursor type if it does. */
		virtual bool HasCustomCursor(const Vector2I position, CursorType& type) const { return false; }

		/**	Checks if the GUI element accepts a drag and drop operation of the specified type. */
		virtual bool AcceptDragAndDrop(const Vector2I position, u32 typeId) const { return false; }

		/**	Returns a context menu if a GUI element has one. Otherwise returns nullptr. */
		virtual SPtr<GUIContextMenu> GetContextMenu() const;

		/**	Returns text to display when hovering over the element. Returns empty string if no tooltip. */
		virtual String GetTooltip() const { return StringUtil::kBlank; }

		/**	Returns a clip rectangle relative to the element, used for offsetting the input text. */
		virtual Vector2I GetTextInputOffset() const { return Vector2I(); }

		/**	Returns a clip rectangle relative to the element, used for clipping	the input text. */
		virtual Rect2I GetTextInputRect() const { return Rect2I(); }

		/** @} */

	protected:
		/**
		 * Called whenever element clipped bounds need to be recalculated. (for example when width, height or clip
		 * rectangles changes).
		 */
		virtual void UpdateClippedBounds();

		/**
		 * Helper method that returns style name used by an element of a certain type. If override style is empty, default
		 * style for that type is returned.
		 */
		template <class T>
		static const String& GetStyleName(const String& overrideStyle)
		{
			if(overrideStyle == StringUtil::kBlank)
				return T::GetGuiTypeName();

			return overrideStyle;
		}

		/**
		 * Attempts to find a sub-style for the specified type in the currently set GUI element style. If one cannot be
		 * found empty string is returned.
		 */
		const String& GetSubStyleName(const String& subStyleTypeName) const;

		/**	Method that gets triggered whenever element style changes. */
		virtual void NotifyStyleChanged() {}

		/**
		 * Returns bounds of the content contained within the GUI element. This will be the bounds returned by GetCachedBounds(),
		 * minus the border and the padding. Relative to parent widget.
		 */
		Rect2I GetCachedContentBounds() const;

		/**
		 * Similar to GetCachedContentBounds(), except the bounds will be clipped by the current clip rectangle, and the
		 * bounds will be relative to the content area of the GUI element rather than relative to the parent widget.
		 */
		Rect2I GetCachedClippedContentBoundsInContentSpace() const;

		/** Calculates the offset from the origin of the GUI element to the area containing content (combined border + padding offsets). */
		Vector2I GetContentOffsetInElementSpace() const;

		/**	Returns the tint that is applied to the GUI element. */
		Color GetTint() const;

		/** Transitions the GUI element into a new state by adding state flags. */
		void AddStateFlags(GUIElementStateFlags flags);

		/** Transitions the GUI element into a new state by removing state flags. */
		void RemoveStateFlags(GUIElementStateFlags flags);

		bool mIsDestroyed = false;
		GUIElementOptions mOptionFlags;
		GUIElementStateFlags mStateFlags = GUIElementStateFlag::Normal;
		Rect2I mClippedBounds;
		TInlineArray<GUIRenderElement, 4> mRenderElements;

		SPtr<const GUIStyleSheetRule> mStyleSheetStyle;
		SPtr<const GUIStyleSheetStateRule> mStyleSheetStateStyle;
	private:
		static const Color kDisabledColor;

		const GUIElementStyle* mStyle;

		String mStyleName;

		SPtr<GUIContextMenu> mContextMenu;
		SPtr<GUINavGroup> mNavigationGroup;
		Color mColor;
	};

	/** @} */

	/** @cond IMPLEMENTATION */
	/** Helper class used for populating GUIRenderElement information from Sprite objects. */
	struct GUIRenderElementHelper
	{
		/**
		 * Contains the sprite to generate render element data for, as well as additional data not provided in the
		 * sprite itself.
		 */
		struct SpriteInfo
		{
			SpriteInfo(Sprite* sprite, u32 depth = 0, GUIMeshType meshType = GUIMeshType::Triangle)
				: Sprite(sprite), Depth(depth), MeshType(meshType)
			{}

			SpriteInfo(Sprite* sprite, u32 depth, const Rect2& bounds, GUIMeshType meshType = GUIMeshType::Triangle)
				: Sprite(sprite), Depth(depth), MeshType(meshType), Bounds(bounds), UseNewFillBuffer(true)
			{}

			Sprite* Sprite;
			u32 Depth = 0;
			GUIMeshType MeshType = GUIMeshType::Triangle;
			Rect2 Bounds = Rect2::kEmpty;
			bool UseNewFillBuffer = false;
		};

		/**
		 * Determines the total number of requires render elements from the provided set of sprites, and initializes that
		 * many render elements from the sprite render elements and the extra information provided in SpriteInfo.
		 */
		template <u32 N>
		static void Populate(const SpriteInfo (&spriteInfos)[N], TInlineArray<GUIRenderElement, 4>& output)
		{
			u32 totalCount = 0;
			for(u32 i = 0; i < N; i++)
				totalCount += spriteInfos[i].Sprite ? spriteInfos[i].Sprite->GetRenderElementCount() : 0;

			output.Resize(totalCount);

			u32 globalIdx = 0;
			for(u32 i = 0; i < N; i++)
			{
				const SpriteInfo& spriteInfo = spriteInfos[i];

				u32 count = spriteInfo.Sprite ? spriteInfo.Sprite->GetRenderElementCount() : 0;
				for(u32 j = 0; j < count; j++)
				{
					GUIRenderElement& renderElem = output[globalIdx];
					spriteInfo.Sprite->GetRenderElement(j, renderElem);

					renderElem.Depth = spriteInfo.Depth;
					renderElem.Type = spriteInfo.MeshType;
					renderElem.Offset = Vector2(spriteInfo.Bounds.X, spriteInfo.Bounds.Y);
					renderElem.ClipSize = Size2(spriteInfo.Bounds.Width, spriteInfo.Bounds.Height);
					renderElem.UseNewFillBuffer = spriteInfo.UseNewFillBuffer;

					globalIdx++;
				}
			}
		}
	};

	/** @endcond */
} // namespace bs
