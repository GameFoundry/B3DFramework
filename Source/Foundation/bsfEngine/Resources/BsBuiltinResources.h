//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "Math/BsVector2I.h"
#include "BsApplication.h"

#include "ThirdParty/json.hpp"

namespace bs
{
	class GUIStyleSheetCascade;
	class GUIStyleSheet;
	/** @addtogroup Resources-Engine
	 *  @{
	 */

	/**	Types of builtin meshes that are always available. */
	enum class BuiltinMesh
	{
		Box,
		Sphere,
		Cone,
		Cylinder,
		Quad,
		Disc
	};

	/**	Types of builtin textures that are always available. */
	enum class BuiltinTexture
	{
		White,
		Black,
		Normal,
		BokehFlare,
		White3D,
		Black3D
	};

	/** Types of builtin shaders that are always available. */
	enum class B3D_SCRIPT_EXPORT() BuiltinShader
	{
		Custom,
		/** Physically based shader used for opaque 3D geometry. */
		Standard,
		/** Physically based shader used for transparent 3D geometry. */
		Transparent,
		/** Special shader used for rendering particles without any lighting, with support for transparency. */
		ParticlesUnlit,
		/**
		 * Special shader used for rendering particles with lighting using the forward rendering pipeline (supports
		 * transparency).
		 */
		ParticlesLit,
		/**
		 * Special shader used for rendering particles with lighting using the deferred rendering pipeline (no support
		 * for transparency).
		 */
		ParticlesLitOpaque,
		/** Special shader used for rendering decals that project onto other geometry. */
		Decal
	};

	/**	Holds references to built-in resources used by the core engine. */
	class B3D_EXPORT BuiltinResources : public bs::Module<BuiltinResources>
	{
	public:
		BuiltinResources() = default;
		~BuiltinResources();

		/** Returns the default style sheet used by the GUI, if no other style sheet is provided. */
		const HGUIStyleSheet& GetDefaultGUIStyleSheet() const { return mDefaultGUIStyleSheet; }

		/** Returns the default style sheet for GUI elements. */
		SPtr<const GUIStyleSheetCascade> GetDefaultGUIStyleSheetCascade() const { return mDefaultGUIStyleSheetCascade; }

		/**	Returns a small entirely white texture. */
		const HSpriteTexture& GetWhiteSpriteTexture() const { return mWhiteSpriteTexture; }

		/**	Returns a dummy 2x2 texture that may be used when no other is available. Don't modify the returned texture. */
		const HTexture& GetDummyTexture() const { return mDummyTexture; }

		/**	Returns image data for an arrow cursor, along with its hotspot. */
		const PixelData& GetCursorArrow(Vector2I& hotSpot);

		/**	Returns image data for an arrow with dragged object cursor, along with its hotspot. */
		const PixelData& GetCursorArrowDrag(Vector2I& hotSpot);

		/**	Returns image data for a wait cursor, along with its hotspot. */
		const PixelData& GetCursorWait(Vector2I& hotSpot);

		/**	Returns image data for an "I" beam cursor, along with its hotspot. */
		const PixelData& GetCursorIBeam(Vector2I& hotSpot);

		/**	Returns image data for a NESW resize cursor, along with its hotspot. */
		const PixelData& GetCursorSizeNesw(Vector2I& hotSpot);

		/**	Returns image data for a NS resize cursor, along with its hotspot. */
		const PixelData& GetCursorSizeNs(Vector2I& hotSpot);

		/**	Returns image data for a NWSE resize cursor, along with its hotspot. */
		const PixelData& GetCursorSizeNwse(Vector2I& hotSpot);

		/**	Returns image data for a WE resize cursor, along with its hotspot. */
		const PixelData& GetCursorSizeWe(Vector2I& hotSpot);

		/**	Returns image data for a deny cursor, along with its hotspot. */
		const PixelData& GetCursorDeny(Vector2I& hotSpot);

		/**	Returns image data for a move left-right cursor, along with its hotspot. */
		const PixelData& GetCursorMoveLeftRight(Vector2I& hotSpot);

		/**	Returns the default application icon. */
		const PixelData& GetFrameworkIcon();

		/**	Returns one of the builtin shader types. */
		HShader GetBuiltinShader(BuiltinShader type) const;

		/**	Creates a material used for textual sprite rendering (for example text in GUI). */
		HMaterial CreateSpriteTextMaterial() const;

		/**	Creates a material used for image sprite rendering (for example images in GUI). */
		HMaterial CreateSpriteImageMaterial() const;

		/** Creates a material used for antialiased line rendering (for example curve rendering in GUI). */
		HMaterial CreateSpriteLineMaterial() const;

		/**	Retrieves one of the builtin meshes. */
		HMesh GetMesh(BuiltinMesh mesh) const;

		/**
		 * Loads a shader at the specified path.
		 *
		 * @param[in]	path	Path relative to the default shader folder with no file extension.
		 */
		HShader GetShader(const Path& path) const;

		/** Attempts to return a font of the given font family. Returns the default font is provided font is not found. */
		HFont GetFont(const String& font) const; // TODO: This needs to perform a lookup in the project library. See method implementation for more information.

		/**
		 * Loads the shader with the specified name from the cache if available, or compiles the shader from source if not available.
		 *
		 * @param	path		Absolute path to the shader source file.
		 * @return				Valid shader if successful, or null otherwise.
		 */
		HShader GetOrCompileShader(const Path& path) const;

		/** Returns the default font used by the engine. */
		HFont GetDefaultFont() const { return mFont; }

		/**	Retrieves one of the builtin textures. */
		static HTexture GetTexture(BuiltinTexture type);

		/**	Returns absolute path to the builtin shader folder where raw shader files are located. */
		static Path GetRawShaderFolder();

		/**	Returns absolute path to the builtin shader folder where raw shader include files are located. */
		static Path GetRawShaderIncludeFolder();

		/**	Returns absolute path to the builtin shader include folder. */
		static Path GetShaderIncludeFolder();

		/**	Returns absolute path to the builtin icons folder. */
		static Path GetIconFolder();

#if B3D_IS_ENGINE || defined BS_IS_ASSET_TOOL
		/**	Returns absolute path to the editor builtin shader include folder. */
		static Path GetEditorShaderIncludeFolder();

		/**	Returns absolute path to the editor builtin raw shader include folder. */
		static Path GetEditorRawShaderIncludeFolder();

#endif

		static constexpr const char* kIconTextureName = "bsfIcon.png";
		static constexpr const char* kMultiLineLabelStyle = "MultiLineLabel";

		static constexpr const char* kShaderFolder = "Shaders/";
		static constexpr const char* kCursorFolder = "Cursors/";
		static constexpr const char* kIconFolder = "Icons/";
		static constexpr const char* kIcoN3DFolder = "Icons3D/";
		static constexpr const char* kSkinFolder = "Skin/";
		static constexpr const char* kAnimatedSpritesFolder = "AnimatedSprites/";
		static constexpr const char* kShaderIncludeFolder = "Shaders/Includes/";
		static constexpr const char* kMeshFolder = "Meshes/";
		static constexpr const char* kTextureFolder = "Textures/";
		static constexpr const char* kSpriteFolder = "Sprites/";
		static constexpr const char* kFontsFolder = "Fonts/";

		static constexpr const char* kMeshSphereFile = u8"Sphere.asset";
		static constexpr const char* kMeshBoxFile = u8"Box.asset";
		static constexpr const char* kMeshConeFile = u8"Cone.asset";
		static constexpr const char* kMeshCylinderFile = u8"Cylinder.asset";
		static constexpr const char* kMeshQuadFile = u8"Quad.asset";
		static constexpr const char* kMeshDiscFile = u8"Disc.asset";

		static constexpr const char* kTextureWhiteFile = u8"White.asset";
		static constexpr const char* kTextureBlackFile = u8"Black.asset";
		static constexpr const char* kTextureNormalFile = u8"Normal.asset";
		static constexpr const char* kTextureWhite3DFile = u8"White3D.asset";
		static constexpr const char* kTextureBlack3DFile = u8"Black3D.asset";

		static constexpr const char* kDefaultFontName = u8"arial.ttf";
		static constexpr const u32 kDefaultFontSize = 8;

		static constexpr const char* kGuiSkinFile = u8"GUISkin";

	private:
		void OnStartUp() override;

		/**	Loads a GUI skin texture with the specified filename. */
		HSpriteTexture GetSkinTexture(const String& name) const;

		/**	Loads a cursor texture with the specified filename. */
		HTexture GetCursorTexture(const String& name) const;

		HGUIStyleSheet mDefaultGUIStyleSheet;
		SPtr<GUIStyleSheetCascade> mDefaultGUIStyleSheetCascade;
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
		SPtr<PixelData> mFrameworkIcon;

		HSpriteTexture mWhiteSpriteTexture;

		HTexture mDummyTexture;

		HShader mShaderSpriteText;
		HShader mShaderSpriteImage;
		HShader mShaderSpriteLine;
		HShader mShaderDiffuse;
		HShader mShaderTransparent;
		HShader mShaderParticlesUnlit;
		HShader mShaderParticlesLit;
		HShader mShaderParticlesLitOpaque;
		HShader mShaderDecal;

		SPtr<ResourceManifest> mResourceManifest;
		mutable UnorderedMap<Path, HShader> mCompiledShaders;

		Path mBuiltinRawDataFolder;
		Path mBuiltinDataFolder;
		Path mEngineSkinSpritesFolder;
		Path mEngineShaderFolder;
		Path mEngineMeshFolder;
		Path mEngineCursorFolder;
		Path mEngineFontsFolder;

		Path ResourceManifestPath;

		static const String kWhiteTex;

		static const String kCursorArrowTex;
		static const String kCursorArrowDragTex;
		static const String kCursorArrowLeftRightTex;
		static const String kCursorIBeamTex;
		static const String kCursorDenyTex;
		static const String kCursorWaitTex;
		static const String kCursorSizeNeswTex;
		static const String kCursorSizeNsTex;
		static const String kCursorSizeNwseTex;
		static const String kCursorSizeWeTex;

		static const Vector2I kCursorArrowHotspot;
		static const Vector2I kCursorArrowDragHotspot;
		static const Vector2I kCursorArrowLeftRightHotspot;
		static const Vector2I kCursorIBeamHotspot;
		static const Vector2I kCursorDenyHotspot;
		static const Vector2I kCursorWaitHotspot;
		static const Vector2I kCursorSizeNeswHotspot;
		static const Vector2I kCursorSizeNsHotspot;
		static const Vector2I kCursorSizeNwseHotspot;
		static const Vector2I kCursorSizeWeHotspot;

		static const String kShaderSpriteTextFile;
		static const String kShaderSpriteImageFile;
		static const String kShaderSpriteLineFile;
	};

	namespace ct
	{
		/**	Built-in engine resources available to the render thread. */
		class B3D_EXPORT BuiltinResources : public Module<BuiltinResources>
		{
		public:
			SPtr<Texture> WhiteTexture2D;
			SPtr<Texture> BlackTexture2D;
			SPtr<Texture> NormalTexture2D;
			SPtr<Texture> WhiteTexture3D;
			SPtr<Texture> BlackTexture3D;

		private:
			friend class bs::BuiltinResources;
		};
	} // namespace ct

	/**	Provides easy access to BuiltinResources. */
	B3D_EXPORT BuiltinResources& GetBuiltinResources();

	/** @} */
} // namespace bs
