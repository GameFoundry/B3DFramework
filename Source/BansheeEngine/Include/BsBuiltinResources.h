//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUISkin.h"
#include "BsModule.h"
#include "BsVector2I.h"
#include "BsApplication.h"

namespace bs
{
	/** @addtogroup Resources-Engine
	 *  @{
	 */

	/**	Types of builtin meshes that are always available. */
	enum class BuiltinMesh
	{
		Box, Sphere, Cone, Quad, Disc
	};

	/**	Types of builtin textures that are always available. */
	enum class BuiltinTexture
	{
		White, Black, Normal
	};

	/** Types of builtin shaders that are always available. */
	enum class BuiltinShader
	{
		Standard, Custom
	};

	/**	Holds references to built-in resources used by the core engine. */
	class BS_EXPORT BuiltinResources : public bs::Module<BuiltinResources>
	{
	public:
		BuiltinResources();
		~BuiltinResources();

		/**	Returns the default skin used by engine GUI elements. */
		const HGUISkin& getGUISkin() const { return mSkin; }

		/**	Returns an empty skin used to be used when no other is available. */
		const HGUISkin& getEmptyGUISkin() const { return mEmptySkin; }

		/**	Returns a small entirely white texture. */
		const HSpriteTexture& getWhiteSpriteTexture() const { return mWhiteSpriteTexture; }

		/**	Returns a 2x2 sprite texture that can be used when no other is available. */
		const HSpriteTexture& getDummySpriteTexture() const { return mDummySpriteTexture; }

		/**	Returns a dummy 2x2 texture that may be used when no other is available. Don't modify the returned texture. */
		const HTexture& getDummyTexture() const { return mDummyTexture; }

		/**	Returns image data for an arrow cursor, along with its hotspot. */
		const PixelData& getCursorArrow(Vector2I& hotSpot);

		/**	Returns image data for an arrow with dragged object cursor, along with its hotspot. */
		const PixelData& getCursorArrowDrag(Vector2I& hotSpot);
		
		/**	Returns image data for a wait cursor, along with its hotspot. */
		const PixelData& getCursorWait(Vector2I& hotSpot);
		
		/**	Returns image data for an "I" beam cursor, along with its hotspot. */
		const PixelData& getCursorIBeam(Vector2I& hotSpot);
		
		/**	Returns image data for a NESW resize cursor, along with its hotspot. */
		const PixelData& getCursorSizeNESW(Vector2I& hotSpot);
		
		/**	Returns image data for a NS resize cursor, along with its hotspot. */
		const PixelData& getCursorSizeNS(Vector2I& hotSpot);
		
		/**	Returns image data for a NWSE resize cursor, along with its hotspot. */
		const PixelData& getCursorSizeNWSE(Vector2I& hotSpot);
		
		/**	Returns image data for a WE resize cursor, along with its hotspot. */
		const PixelData& getCursorSizeWE(Vector2I& hotSpot);
		
		/**	Returns image data for a deny cursor, along with its hotspot. */
		const PixelData& getCursorDeny(Vector2I& hotSpot);
		
		/**	Returns image data for a move left-right cursor, along with its hotspot. */
		const PixelData& getCursorMoveLeftRight(Vector2I& hotSpot);

		/**	Returns the default application icon. */
		const PixelData& getBansheeIcon();

		/**	Returns a shader used for rendering only a diffuse texture. */
		HShader getBuiltinShader(BuiltinShader type) const;

		/**	Creates a material used for textual sprite rendering (for example text in GUI). */
		HMaterial createSpriteTextMaterial() const;

		/**	Creates a material used for image sprite rendering (for example images in GUI). */
		HMaterial createSpriteImageMaterial() const;

		/**	Creates a material used for non-transparent image sprite rendering (for example images in GUI). */
		HMaterial createSpriteNonAlphaImageMaterial() const;

		/** Creates a material used for antialiased line rendering (for example curve rendering in GUI). */
		HMaterial createSpriteLineMaterial() const;

		/**	Retrieves one of the builtin meshes. */
		HMesh getMesh(BuiltinMesh mesh) const;

		/**
		 * Loads a shader at the specified path.
		 * 
		 * @param[in]	path	Path relative to the default shader folder with no file extension.
		 */
		HShader getShader(const Path& path);

		/** Returns the default font used by the engine. */
		HFont getDefaultFont() const { return mFont; }

		/**	Retrieves one of the builtin textures. */
		static HTexture getTexture(BuiltinTexture type);

		/**	Returns image data the Banshee Engine splash screen. */
		static SPtr<PixelData> getSplashScreen();

		/**	Returns absolute path to the builtin shader include folder. */
		static Path getShaderIncludeFolder();

		/**	Returns absolute path to the builtin icons folder. */
		static Path getIconFolder();

		static const WString IconTextureName;

		static const String MultiLineLabelStyle;
	private:
		/**
		 * Imports all necessary resources and converts them to engine-ready format.
		 *
		 * @note	
		 * Normally you only want to use this during development phase and then ship with engine-ready format only.
		 */
		void preprocess();

		/**	Generates the default engine skin and all GUI element styles. */
		SPtr<GUISkin> generateGUISkin();

		/**	Generates the builtin meshes. */
		void generateMeshes();

		/**	Generates the builtin textures. */
		void generateTextures();

		/**	Loads a GUI skin texture with the specified filename. */
		HSpriteTexture getSkinTexture(const WString& name);

		/**	Loads a cursor texture with the specified filename. */
		HTexture getCursorTexture(const WString& name);

		HGUISkin mEmptySkin;
		HGUISkin mSkin;
		HFont mFont;

		SPtr<PixelData> mCursorArrow;
		SPtr<PixelData> mCursorArrowDrag;
		SPtr<PixelData> mCursorArrowLeftRight;
		SPtr<PixelData> mCursorIBeam;
		SPtr<PixelData> mCursorDeny;
		SPtr<PixelData> mCursorWait;
		SPtr<PixelData> mCursorSizeNESW;
		SPtr<PixelData> mCursorSizeNS;
		SPtr<PixelData> mCursorSizeNWSE;
		SPtr<PixelData> mCursorSizeWE;
		SPtr<PixelData> mBansheeIcon;

		HSpriteTexture mWhiteSpriteTexture;
		HSpriteTexture mDummySpriteTexture;

		HTexture mDummyTexture;

		HShader mShaderSpriteText;
		HShader mShaderSpriteImage;
		HShader mShaderSpriteNonAlphaImage;
		HShader mShaderSpriteLine;
		HShader mShaderDiffuse;

		SPtr<ResourceManifest> mResourceManifest;

		Path mBuiltinRawDataFolder;
		Path mBuiltinDataFolder;
		Path mEngineSkinSpritesFolder;
		Path mEngineShaderFolder;
		Path mEngineMeshFolder;
		Path mEngineCursorFolder;

		Path ResourceManifestPath;

		static const char* DataListFile;
		static const char* CursorFolder;
		static const char* IconFolder;
		static const char* ShaderFolder;
		static const char* ShaderIncludeFolder;
		static const char* SkinFolder;
		static const char* MeshFolder;
		static const char* TextureFolder;
		static const char* SpriteSubFolder;

		static const WString DefaultFontFilename;
		static const UINT32 DefaultFontSize;

		static const Color TextNormalColor;
		static const Color TextActiveColor;

		static const WString GUISkinFile;
		static const WString WhiteTex;
		static const wchar_t* SplashScreenName;

		static const WString ButtonNormalTex;
		static const WString ButtonHoverTex;
		static const WString ButtonActiveTex;

		static const WString ToggleNormalTex;
		static const WString ToggleHoverTex;
		static const WString ToggleNormalOnTex;
		static const WString ToggleHoverOnTex;

		static const WString InputBoxNormalTex;
		static const WString InputBoxHoverTex;
		static const WString InputBoxFocusedTex;

		static const WString ScrollBarUpNormalTex;
		static const WString ScrollBarUpHoverTex;
		static const WString ScrollBarUpActiveTex;

		static const WString ScrollBarDownNormalTex;
		static const WString ScrollBarDownHoverTex;
		static const WString ScrollBarDownActiveTex;

		static const WString ScrollBarLeftNormalTex;
		static const WString ScrollBarLeftHoverTex;
		static const WString ScrollBarLeftActiveTex;

		static const WString ScrollBarRightNormalTex;
		static const WString ScrollBarRightHoverTex;
		static const WString ScrollBarRightActiveTex;

		static const WString ScrollBarHandleHorzNormalTex;
		static const WString ScrollBarHandleHorzHoverTex;
		static const WString ScrollBarHandleHorzActiveTex;

		static const WString ScrollBarHandleVertNormalTex;
		static const WString ScrollBarHandleVertHoverTex;
		static const WString ScrollBarHandleVertActiveTex;

		static const WString ScrollBarHBgTex;
		static const WString ScrollBarVBgTex;

		static const WString DropDownBtnNormalTex;
		static const WString DropDownBtnHoverTex;
		static const WString DropDownBtnActiveTex;

		static const WString DropDownBoxBgTex;
		static const WString DropDownBoxSideBgTex;
		static const WString DropDownBoxHandleTex;

		static const WString DropDownBoxEntryNormalTex;
		static const WString DropDownBoxEntryHoverTex;

		static const WString DropDownBoxBtnUpNormalTex;
		static const WString DropDownBoxBtnUpHoverTex;

		static const WString DropDownBoxBtnDownNormalTex;
		static const WString DropDownBoxBtnDownHoverTex;

		static const WString DropDownBoxEntryExpNormalTex;
		static const WString DropDownBoxEntryExpHoverTex;

		static const WString DropDownSeparatorTex;

		static const WString CursorArrowTex;
		static const WString CursorArrowDragTex;
		static const WString CursorArrowLeftRightTex;
		static const WString CursorIBeamTex;
		static const WString CursorDenyTex;
		static const WString CursorWaitTex;
		static const WString CursorSizeNESWTex;
		static const WString CursorSizeNSTex;
		static const WString CursorSizeNWSETex;
		static const WString CursorSizeWETex;

		static const Vector2I CursorArrowHotspot;
		static const Vector2I CursorArrowDragHotspot;
		static const Vector2I CursorArrowLeftRightHotspot;
		static const Vector2I CursorIBeamHotspot;
		static const Vector2I CursorDenyHotspot;
		static const Vector2I CursorWaitHotspot;
		static const Vector2I CursorSizeNESWHotspot;
		static const Vector2I CursorSizeNSHotspot;
		static const Vector2I CursorSizeNWSEHotspot;
		static const Vector2I CursorSizeWEHotspot;

		static const WString ShaderSpriteTextFile;
		static const WString ShaderSpriteImageAlphaFile;
		static const WString ShaderSpriteImageNoAlphaFile;
		static const WString ShaderSpriteLineFile;
		static const WString ShaderDiffuseFile;

		static const WString MeshSphereFile;
		static const WString MeshBoxFile;
		static const WString MeshConeFile;
		static const WString MeshQuadFile;
		static const WString MeshDiscFile;

		static const WString TextureWhiteFile;
		static const WString TextureBlackFile;
		static const WString TextureNormalFile;
	};

	/** @} */
}