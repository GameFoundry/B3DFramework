//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUISpriteHelper.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIConstructionMethods.h"
#include "2D/B3DImageSprite.h"

namespace b3d
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

		static constexpr const char* kStyleSheetElementType = "texture";
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct {};
		GUITexture(PrivatelyConstruct, const GUITextureContents& contents, const String& styleName, const GUISizeConstraints& dimensions);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;
		const char* GetStyleSheetElement() const override { return kStyleSheetElementType; }

		/** @} */
	protected:
		virtual ~GUITexture();

		void UpdateRenderElements() override;
		void NotifyStyleChanged() override;

		GUIBackgroundSprite mBackgroundSprite;
		ImageSprite* mImageSprite;
		HSpriteImage mActiveImage;
		GUILogicalSize mActiveImageSize{kZeroTag};
		ImageSpriteInformation mImageSpriteInformation;
		TextureScaleMode mScaleMode;
		bool mTransparent;
		bool mUsingStyleTexture;
	};

	/** @} */
} // namespace b3d
