//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIContent.h"
#include "GUI/B3DGUIElement.h"
#include "2D/B3DSprite.h"
#include "Math/B3DArea2.h"
#include "Image/B3DColor.h"

namespace b3d
{
	class GUINavGroup;
	class GUIStyleSheetCascade;

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Contains information about a single renderable element within a GUIElement. */
	struct GUIRenderElement : SpriteRenderElement
	{
		GUIMeshType Type = GUIMeshType::Triangle;
		u32 Depth = 0;
		Vector2 Offset = Vector2::kZero; /**< Offset to apply to every vertex in the render element, relative to parent GUI element. */
		Area2 ClipRectangle = Area2::kEmpty; /**< Area of the clip rectangle, relative to the parent GUI element. Any vertices outside of this area will be clipped. Clipping is done before @p Offset is applied. */
		bool UseNewFillBuffer = false;
	};

	/** @} */

	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Represents a GUI element that can be rendered (i.e. has a visual representation). Renderable element can have a particular style, and provides
	 * one or multiple render elements to be drawn.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIRenderable : public GUIElement
	{
		using Super = GUIElement;
	public:
		GUIRenderable(String styleClass, const GUISizeConstraints& sizeConstraints);
		GUIRenderable(const char* styleClass, const GUISizeConstraints& sizeConstraints);
		~GUIRenderable() override = default;

		/** Returns the name of the GUI element type to be used for style lookup in the style sheet. */
		virtual StringView GetStyleSheetElement() const = 0;

		/**
		 * Returns the first of the user-specified classes that are used for style lookup in the style sheet, or an empty
		 * string if the element has no classes. See GetStyleSheetClasses() for the full set.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(StyleSheetClass))
		const String& GetStyleSheetClass() const;

		/** Returns all user-specified classes that are used for style lookup in the style sheet. */
		const TInlineArray<String, 1>& GetStyleSheetClasses() const { return mStyleClasses; }

		/** Returns an user-specific ID will be used for style lookup in the style sheet. */
		virtual const String& GetStyleSheetId() const { return StringUtility::kBlank; }

		/**	Replaces all style classes of the element with a single new one. An empty string clears all classes. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(StyleSheetClass))
		void SetStyleSheetClass(const String& styleClass);

		/** Checks if the element carries the provided style class. */
		B3D_SCRIPT_EXPORT()
		bool HasStyleClass(const String& styleClass) const;

		/** Adds a style class to the element, restyling it. Does nothing if the class is already present. */
		B3D_SCRIPT_EXPORT()
		void AddStyleClass(const String& styleClass);

		/** Removes a style class from the element, restyling it. Does nothing if the class is not present. */
		B3D_SCRIPT_EXPORT()
		void RemoveStyleClass(const String& styleClass);

		/** Adds or removes a style class depending on @p enabled. */
		B3D_SCRIPT_EXPORT()
		void ToggleStyleClass(const String& styleClass, bool enabled);

		/** Replaces all style classes of the element with the provided set. */
		void SetStyleClasses(TInlineArray<String, 1> styleClasses);

		/** Removes all style classes from the element. */
		B3D_SCRIPT_EXPORT()
		void ClearStyleClasses();

		/** Returns true if the GUI elements wants to use the new style sheet approach for styling. */
		bool IsUsingStyleSheets() const; // TODO - Temporary only while we transition styles out

		/** Returns the set of states the GUI element is currently in. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(StateFlags))
		GUIElementStates GetStateFlags() const { return mStateFlags; }

		/**	Sets the tint of the GUI element. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Tint))
		virtual void SetTint(const Color& color);

		/**	Returns the tint that is applied to the GUI element. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Tint))
		Color GetTint() const;

		void ResetSizeConstraints() override;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Determines if an element type falls back to its type name when no style class was provided at construction.
		 * Types that leave this false stay class-less until a class is assigned, which is what lets them opt out of style
		 * sheet lookup entirely (see GUILayout).
		 */
		static constexpr bool kUseTypeNameAsDefaultStyleClass = true;

		/**
		 * Helper method that returns style class used by an element of a certain type. If override style class is empty, default
		 * class for that type is returned, unless the type opted out through kUseTypeNameAsDefaultStyleClass.
		 */
		template <class T>
		static const String& GetStyleClass(const String& overrideStyle)
		{
			if constexpr(T::kUseTypeNameAsDefaultStyleClass)
			{
				if(overrideStyle == StringUtility::kBlank)
					return T::GetGuiTypeName();
			}

			return overrideStyle;
		}

		/**
		 * Returns information about all renderable elements in this GUI element, including their mesh, material and
		 * general information.
		 */
		const TInlineArray<GUIRenderElement, 4>& GetRenderElements() const { return mRenderElements; }

		/**
		 * Fill the pre-allocated vertex, uv and index buffers with the mesh data for the specified render element.
		 *
		 * @param	vertices				Previously allocated buffer where to store the vertices. Output is expected
		 *										to match the GUIMeshType as returned by getRenderElements() for the specified
		 *										element.
		 * @param	indices					Previously allocated buffer where to store the indices.
		 * @param	vertexOffset			At which vertex should the method start filling the buffer.
		 * @param	offset					Offset that should be applied to all output vertex positions.
		 * @param	indexOffset				At which index should the method start filling the buffer.
		 * @param	maxVertexCount			Total number of vertices the buffers were allocated for. Used only for memory
		 *										safety.
		 * @param	maxIndexCount			Total number of indices the buffers were allocated for. Used only for memory
		 *										safety.
		 * @param	renderElementIndex		Zero-based index of the render element.
		 *
		 */
		virtual void FillBuffer(
			u8* vertices,
			u32* indices,
			u32 vertexOffset,
			u32 indexOffset,
			const Vector2I& offset,
			u32 maxVertexCount,
			u32 maxIndexCount,
			u32 renderElementIndex) const { }

		/**
		 * Retrieves vertex and index data from GUI render element and outputs them to the provided buffers. GUI render
		 * elements must have been previously populated by calling UpdateRenderElements().
		 *
		 * @param	renderElementIndex		Zero-based index of the render element from which to retrieve the data.
		 * @param	vertexOffset			At which vertex should the method start writing to the output position/uv buffer.
		 * @param	indexOffset				At which index should the method start writing to the output index buffer.
		 * @param	outPositions			Previously allocated buffer where to store the vertex positions. Caller must ensure
		 *										size and data type match the mesh type and vertex count retrieved from
		 *										GetRenderElements() for the specified element.
		 * @param	outUVs					Previously allocated buffer where to store the vertex UVs. Caller must ensure
		 *										size and data type match the mesh type and vertex count retrieved from
		 *										GetRenderElements() for the specified element. Can be null if not needed.
		 * @param	outIndices				Previously allocated buffer where to store the indices. Caller must ensure
		 *										size and data type match the mesh type and index count retrieved from
		 *										GetRenderElements() for the specified element.
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
		 * Recreates the internal render elements. Must be called before GetRenderElementVertexAndIndexData/FillBuffer if element is dirty.
		 * Marks the element as non dirty.
		 */
		virtual void UpdateRenderElements() { }

		/** Set element part of element depth. Less significant than both widget and area depth. */
		void SetElementDepth(u8 depth);

		/** Retrieve element part of element depth. Less significant than both widget and area depth. */
		u8 GetElementDepth() const;

		/**
		 * Returns the range of depths that the child elements can be rendered it.
		 *
		 * @note
		 * For example if you are rendering a button with an image and a text you will want the text to be rendered in front
		 * of the image at a different depth, which means the depth range is 2 (0 for text, 1 for background image).
		 */
		virtual u32 GetRenderElementDepthRange() const { return 1; }

		/** Updates element style based on active GUI style sheet. Call this after active style sheet changes, or element class/id changes. */
		void RefreshStyle();

		/**
		 * Recalculates the style keys and restyles the element, then continues into the elements below it if the key they
		 * build their own keys on changed.
		 */
		void RefreshStyleRecursive() override;

		/**
		 * Returns a value that identifies everything the style sheet selectors can match on for this element: its type,
		 * classes and id, plus whatever the ancestor parts of the selectors can observe about its ancestors (see
		 * GUIStyleSheetCascade::MatchAncestorSelectors()). The element's own state is not part of the key. Two elements
		 * with an equal style key always resolve to the same set of style sheet rulesets.
		 */
		u64 GetStyleKey() const;

		/**
		 * Returns the states of this element that the style sheet rules of its descendants depend on. A change to any other
		 * state never needs to restyle the descendants.
		 */
		GUIElementStates GetStatesAffectingDescendants() const;

		/**
		 * Returns true if the element only arranges its children. Such elements are not registered with the parent widget,
		 * so they are never drawn and never receive input, and cost nothing beyond their layout contribution. They are
		 * also invisible to the style sheet: they are never styled, and selectors match through them as if their children
		 * were children of their parent.
		 */
		virtual bool IsLayoutOnly() const { return false; }

		/** Returns the closest ancestor that takes part in style sheet matching, skipping elements that are layout only. */
		const GUIRenderable* GetStyleParent() const;

		/**
		 * Returns GUI element depth. This includes widget and area depth, but does not include specific per-render-element
		 * depth.
		 */
		u32 GetDepth() const { return mLayoutData.Depth; }

		const RectOffset& GetMargins() const override;
		const RectOffset& GetPadding() const override;

		void SetLayoutData(const GUILayoutData& data) override;
		void ChangeParentWidget(GUIWidget* widget) override;

		/** @} */
	protected:
		friend class GUISpriteHelper;

		/**	Method that gets triggered whenever element style changes. */
		virtual void NotifyStyleChanged() {}

		/**
		 * Restyles the element and everything below it, and re-registers it with the parent widget if the result of
		 * IsLayoutOnly() changed. Call after changing the style classes, or anything else IsLayoutOnly() depends on.
		 *
		 * @param	wasLayoutOnly	Result of IsLayoutOnly() before the change.
		 */
		void RefreshStyleAndRegistration(bool wasLayoutOnly);

		/**
		 * Re-registers the element with its parent widget after the result of IsLayoutOnly() changed. Only called while the
		 * element has a parent widget and is not pending destroy.
		 */
		virtual void RefreshWidgetRegistration();

		/** Returns the style sheet cascade of the parent widget, or an empty cascade if the element is not in a widget. */
		const GUIStyleSheetCascade& GetActiveStyleSheetCascade() const;

		/**
		 * Recalculates the style key, the descendant style key and the states affecting descendants if they were never
		 * calculated, or were calculated against a different revision of the style sheet cascade.
		 */
		void EnsureStyleKeys() const;

		/** Recalculates the style key, the descendant style key and the states affecting descendants. */
		void UpdateStyleKeys() const;

		/**
		 * Similar to GetAbsoluteContentBounds(), except the bounds are relative to the parent GUI element rather than the
		 * parent widget.
		 */
		virtual GUILogicalArea GetContentBounds() const;

		/** Similar to GetContentBounds(), but scaling has been applied to the position/size. */
		GUIPhysicalArea GetScaledContentBounds() const;

		/**
		 * Returns bounds of the content contained within the GUI element. This will be the bounds returned by GetAbsoluteBounds(),
		 * minus the border and the padding. Relative to parent widget.
		 */
		GUIPhysicalArea GetAbsoluteContentBounds() const;

		/** Calculates the offset from the origin of the GUI element to the area containing content (combined border + padding offsets). */
		GUILogicalPoint GetContentOffset() const;

		/** Similar to GetContentOffset(), but scaling has been applied. */
		GUIPhysicalPoint GetScaledContentOffset() const;

		/**
		 * Registers a new pseudo-element for the GUI element. Pseudo-element can be used for providing additional style sheet rules for a GUI element.
		 *
		 * @param	name		Name of the pseudo-element. This will correspond to the pseudo-element in the style sheet (e.g. `toggle::checkmark` will
		 *						provide a `checkmark` pseudo-element for the `toggle` GUI element).
		 * @return				Index you can use to retrieve pseudo-element style information from GetPseudoElementStyleSheetRuleInformation().
		 *
		 * @note	Pseudo-elements cannot be removed. They are intended to be registered once on GUI element construction.
		 */
		u32 RegisterPseudoElement(const char* name);

		/** Returns style information for a pseudo-element at the specified index. */
		const GUIStyleSheetRuleInformation& GetPseudoElementStyleSheetRuleInformation(u32 pseudoElementIndex) const;

	protected:
		static const Color kDisabledColor;

		TInlineArray<GUIRenderElement, 4> mRenderElements;
		GUIElementStates mStateFlags = GUIElementState::Normal;

		Color mColor;

		// Style sheet
		TInlineArray<GUIStyleSheetRuleInformation, 2> mPseudoElementStyleSheetRules;
		GUIStyleSheetRuleInformation mStyleSheetRuleInformation;
		TInlineArray<String, 1> mStyleClasses;

		mutable u64 mStyleKey = 0;
		mutable u64 mDescendantStyleKey = 0; /**< Key descendants build their own style keys on. Covers the ancestors and whatever the ancestor parts of the selectors can observe about this element, including its state. */
		mutable GUIElementStates mStatesAffectingDescendants = GUIElementState::Normal;
		mutable bool mHasParentOfMatch = false; /**< See GUIStyleSheetAncestorMatch::HasParentOfMatch. */
		mutable u64 mStyleKeyRevision = 0; /**< Revision of the style sheet cascade the style keys were calculated against. Zero if they need recalculating. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIRenderableRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
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

			SpriteInfo(Sprite* sprite, u32 depth, const Area2& bounds, GUIMeshType meshType = GUIMeshType::Triangle)
				: Sprite(sprite), Depth(depth), MeshType(meshType), Offset(bounds.X, bounds.Y), ClipRectangle(bounds), UseNewFillBuffer(true)
			{}

			SpriteInfo(Sprite* sprite, u32 depth, const Vector2& offset, const Area2& clipRectangle, GUIMeshType meshType = GUIMeshType::Triangle)
				: Sprite(sprite), Depth(depth), MeshType(meshType), Offset(offset.X, offset.Y), ClipRectangle(clipRectangle), UseNewFillBuffer(true)
			{}

			Sprite* Sprite;
			u32 Depth = 0;
			GUIMeshType MeshType = GUIMeshType::Triangle;
			Vector2 Offset = Vector2::kZero;
			Area2 ClipRectangle;
			bool UseNewFillBuffer = false;
		};

		/**
		 * Determines the total number of requires render elements from the provided set of sprites, and initializes that
		 * many render elements from the sprite render elements and the extra information provided in SpriteInfo.
		 */
		template <u32 N>
		static void Populate(const SpriteInfo (&spriteInfos)[N], TInlineArray<GUIRenderElement, 4>& output)
		{
			output.Clear();

			Append(spriteInfos, output);
		}

		/** Appends render elements from one or multiple sprites into @p output. */
		template <u32 N>
		static void Append(const SpriteInfo (&spriteInfos)[N], TInlineArray<GUIRenderElement, 4>& output)
		{
			u32 totalCount = 0;
			for(u32 spriteInfoIndex = 0; spriteInfoIndex < N; spriteInfoIndex++)
				totalCount += spriteInfos[spriteInfoIndex].Sprite ? spriteInfos[spriteInfoIndex].Sprite->GetRenderElementCount() : 0;

			u32 outputIndex = (u32)output.Size();
			output.Resize(output.Size() + totalCount);

			for(u32 spriteInfoIndex = 0; spriteInfoIndex < N; spriteInfoIndex++)
			{
				const SpriteInfo& spriteInfo = spriteInfos[spriteInfoIndex];

				const u32 renderElementCount = spriteInfo.Sprite ? spriteInfo.Sprite->GetRenderElementCount() : 0;
				for(u32 renderElementIndex = 0; renderElementIndex < renderElementCount; renderElementIndex++)
				{
					GUIRenderElement& renderElement = output[outputIndex];
					spriteInfo.Sprite->GetRenderElement(renderElementIndex, renderElement);

					renderElement.Depth = spriteInfo.Depth;
					renderElement.Type = spriteInfo.MeshType;
					renderElement.Offset = spriteInfo.Offset;
					renderElement.ClipRectangle = spriteInfo.ClipRectangle;
					renderElement.UseNewFillBuffer = spriteInfo.UseNewFillBuffer;

					outputIndex++;
				}
			}
		}
	};

	/** @endcond */
} // namespace b3d
