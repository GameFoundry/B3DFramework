//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUISpriteHelper.h"
#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsImageSprite.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Provides all needed permutations of Create methods used for creating a GUI element. */
	template <class GUIElementType, class ContentType>
	class TGUIConstructionMethods // TODO - Move this to some common header
	{
	public:
		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const ContentType& contents, const String& styleClass, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), contents, GUIElement::GetStyleName<GUIElementType>(styleClass), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 */
		static GUIElementType* Create(const ContentType& contents, const String& styleClass)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), contents, GUIElement::GetStyleName<GUIElementType>(styleClass), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const ContentType& contents, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), contents, GUIElement::GetStyleName<GUIElementType>(StringUtil::kBlank), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 */
		static GUIElementType* Create(const ContentType& contents)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), contents, GUIElement::GetStyleName<GUIElementType>(StringUtil::kBlank), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const String& styleClass, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), ContentType(), GUIElement::GetStyleName<GUIElementType>(styleClass), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 */
		static GUIElementType* Create(const String& styleClass)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), ContentType(), GUIElement::GetStyleName<GUIElementType>(styleClass), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), ContentType(), GUIElement::GetStyleName<GUIElementType>(StringUtil::kBlank), GUISizeConstraints::Create(options));
		}

		/** Creates a new GUI element. */
		static GUIElementType* Create()
		{
			return B3DNew<GUIElementType>(GUIElementType::PrivatelyConstruct(), ContentType(), GUIElement::GetStyleName<GUIElementType>(StringUtil::kBlank), GUISizeConstraints::Create());
		}
	};

	/** Structure describing contents of a GUITexture element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUITextureContents
	{
		GUITextureContents() = default;
		GUITextureContents(const HSpriteImage& B3D_NO_RREF image, TextureScaleMode scaleMode = TextureScaleMode::StretchToFit, bool isTransparent = true)
			: Image(image), ScaleMode(scaleMode), IsTransparent(isTransparent)
		{ }

		B3D_NO_RREF HSpriteImage Image; /**< Image to display. If this is null then the image specified by the style will be used. */
		TextureScaleMode ScaleMode = TextureScaleMode::StretchToFit; /**< Scale mode to use when sizing the texture. */
		bool IsTransparent = true; /**< Determines should the texture be rendered with transparency active. */ 
	};

	/**	A GUI element that displays a texture. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUITexture : public GUIElement, public TGUIConstructionMethods<GUITexture, GUITextureContents>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Changes the active image. If the provided image is null then the image specified by the style will be used.
		 */
		B3D_SCRIPT_EXPORT()
		void SetImage(const HSpriteImage& image);

		static constexpr const char* kElementType = "texture";
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct {};
		GUITexture(PrivatelyConstruct, const GUITextureContents& contents, const String& styleName, const GUISizeConstraints& dimensions);

		ElementType GetElementType() const override { return ElementType::Texture; }
		Vector2I CalculateUnconstrainedOptimalSize() const override;
		const char* GetStyleSheetElement() const override { return kElementType; }

		/** @} */
	protected:
		virtual ~GUITexture();

		void UpdateRenderElements() override;
		void NotifyStyleChanged() override;

		GUIBackgroundSprite mBackgroundSprite;
		ImageSprite* mImageSprite;
		HSpriteImage mActiveImage;
		u32 mActiveImageWidth = 0;
		u32 mActiveImageHeight = 0;
		ImageSpriteInformation mDesc;
		TextureScaleMode mScaleMode;
		bool mTransparent;
		bool mUsingStyleTexture;
	};

	/** @} */
} // namespace bs
