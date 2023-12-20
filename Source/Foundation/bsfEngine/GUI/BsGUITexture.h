//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUISpriteHelper.h"
#include "BsPrerequisites.h"
#include "GUI/BsGUIInteractable.h"
#include "GUI/BsGUIConstructionMethods.h"
#include "2D/BsImageSprite.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUITexture : public GUIInteractable, public TGUIConstructionMethods<GUITexture, GUITextureContents>
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
