//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUISkin.h"
#include "Utility/BsModule.h"
#include "Math/BsVector2I.h"
#include "BsApplication.h"

#include "ThirdParty/json.hpp"

namespace bs
{
	/** @addtogroup Resources-Engine
	 *  @{
	 */

	/**	Types of builtin meshes that are always available. */
	enum class BuiltinMesh
	{
		Box, Sphere, Cone, Cylinder, Quad, Disc
	};

	/**	Types of builtin textures that are always available. */
	enum class BuiltinTexture
	{
		White, Black, Normal, BokehFlare
	};

	/** Types of builtin shaders that are always available. */
	enum class BS_SCRIPT_EXPORT() BuiltinShader
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
		const PixelData& getFrameworkIcon();

		/**	Returns one of the builtin shader types. */
		HShader getBuiltinShader(BuiltinShader type) const;

		/**	Creates a material used for textual sprite rendering (for example text in GUI). */
		HMaterial createSpriteTextMaterial() const;

		/**	Creates a material used for image sprite rendering (for example images in GUI). */
		HMaterial createSpriteImageMaterial() const;

		/** Creates a material used for antialiased line rendering (for example curve rendering in GUI). */
		HMaterial createSpriteLineMaterial() const;

		/**	Retrieves one of the builtin meshes. */
		HMesh getMesh(BuiltinMesh mesh) const;

		/**
		 * Loads a shader at the specified path.
		 *
		 * @param[in]	path	Path relative to the default shader folder with no file extension.
		 */
		HShader getShader(const Path& path) const;

		/** Returns the default font used by the engine. */
		HFont getDefaultFont() const { return mFont; }

		/**	Retrieves one of the builtin textures. */
		static HTexture getTexture(BuiltinTexture type);

		/**	Returns absolute path to the builtin shader folder where raw shader files are located. */
		static Path getRawShaderFolder();

		/**	Returns absolute path to the builtin shader include folder. */
		static Path getShaderIncludeFolder();

		/**	Returns absolute path to the builtin icons folder. */
		static Path getIconFolder();

#if BS_IS_BANSHEE3D || defined BS_IS_ASSET_TOOL
		/**	Returns absolute path to the editor builtin shader include folder. */
		static Path getEditorShaderIncludeFolder();

#endif

		static constexpr const char* IconTextureName = "bsfIcon.png";
		static constexpr const char* MultiLineLabelStyle = "MultiLineLabel";

		static constexpr const char* SHADER_FOLDER = "Shaders/";
		static constexpr const char* CURSOR_FOLDER = "Cursors/";
		static constexpr const char* ICON_FOLDER = "Icons/";
		static constexpr const char* ICON3D_FOLDER = "Icons3D/";
		static constexpr const char* SKIN_FOLDER = "Skin/";
		static constexpr const char* ANIMATED_SPRITES_FOLDER = "AnimatedSprites/";
		static constexpr const char* SHADER_INCLUDE_FOLDER = "Shaders/Includes/";
		static constexpr const char* MESH_FOLDER = "Meshes/";
		static constexpr const char* TEXTURE_FOLDER = "Textures/";
		static constexpr const char* SPRITE_FOLDER = "Sprites/";

		static constexpr const char* MESH_SPHERE_FILE = u8"Sphere.asset";
		static constexpr const char* MESH_BOX_FILE = u8"Box.asset";
		static constexpr const char* MESH_CONE_FILE = u8"Cone.asset";
		static constexpr const char* MESH_CYLINDER_FILE = u8"Cylinder.asset";
		static constexpr const char* MESH_QUAD_FILE = u8"Quad.asset";
		static constexpr const char* MESH_DISC_FILE = u8"Disc.asset";

		static constexpr const char* TEXTURE_WHITE_FILE = u8"White.asset";
		static constexpr const char* TEXTURE_BLACK_FILE = u8"Black.asset";
		static constexpr const char* TEXTURE_NORMAL_FILE = u8"Normal.asset";

		static constexpr const char* DEFAULT_FONT_NAME = u8"arial.ttf";
		static constexpr const UINT32 DEFAULT_FONT_SIZE = 8;

		static constexpr const char* GUI_SKIN_FILE = u8"GUISkin";
	private:
		/**	Loads a GUI skin texture with the specified filename. */
		HSpriteTexture getSkinTexture(const String& name) const;

		/**	Loads a cursor texture with the specified filename. */
		HTexture getCursorTexture(const String& name) const;

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
		SPtr<PixelData> mFrameworkIcon;

		HSpriteTexture mWhiteSpriteTexture;
		HSpriteTexture mDummySpriteTexture;

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

		Path mBuiltinRawDataFolder;
		Path mBuiltinDataFolder;
		Path mEngineSkinSpritesFolder;
		Path mEngineShaderFolder;
		Path mEngineMeshFolder;
		Path mEngineCursorFolder;

		Path ResourceManifestPath;

		static const String WhiteTex;

		static const String CursorArrowTex;
		static const String CursorArrowDragTex;
		static const String CursorArrowLeftRightTex;
		static const String CursorIBeamTex;
		static const String CursorDenyTex;
		static const String CursorWaitTex;
		static const String CursorSizeNESWTex;
		static const String CursorSizeNSTex;
		static const String CursorSizeNWSETex;
		static const String CursorSizeWETex;

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

		static const String ShaderSpriteTextFile;
		static const String ShaderSpriteImageFile;
		static const String ShaderSpriteLineFile;
	};

	/**	Provides easy access to BuiltinResources. */
	BS_EXPORT BuiltinResources& gBuiltinResources();

	/** @} */
}
