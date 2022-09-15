//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsBuiltinResources.h"
#include "GUI/BsGUILabel.h"
#include "Image/BsSpriteTexture.h"
#include "Text/BsFont.h"
#include "Image/BsTexture.h"
#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResourcesHelper.h"
#include "Resources/BsResourceManifest.h"
#include "Material/BsShader.h"
#include "Material/BsMaterial.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsFileSystem.h"
#include "CoreThread/BsCoreThread.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Mesh/BsMesh.h"

using json = nlohmann::json;

namespace bs
{
	constexpr const char* BuiltinResources::IconTextureName;
	constexpr const char* BuiltinResources::MultiLineLabelStyle;

	/************************************************************************/
	/* 								GUI TEXTURES                      		*/
	/************************************************************************/

	const String BuiltinResources::WhiteTex = u8"White.psd";

	/************************************************************************/
	/* 							CURSOR TEXTURES                      		*/
	/************************************************************************/

	const String BuiltinResources::CursorArrowTex = u8"Arrow.psd";
	const String BuiltinResources::CursorArrowDragTex = u8"ArrowDrag.psd";
	const String BuiltinResources::CursorArrowLeftRightTex = u8"ArrowLeftRight.psd";
	const String BuiltinResources::CursorIBeamTex = u8"IBeam.psd";
	const String BuiltinResources::CursorDenyTex = u8"Deny.psd";
	const String BuiltinResources::CursorWaitTex = u8"Wait.psd";
	const String BuiltinResources::CursorSizeNESWTex = u8"SizeNESW.psd";
	const String BuiltinResources::CursorSizeNSTex = u8"SizeNS.psd";
	const String BuiltinResources::CursorSizeNWSETex = u8"SizeNWSE.psd";
	const String BuiltinResources::CursorSizeWETex = u8"SizeWE.psd";

	const Vector2I BuiltinResources::CursorArrowHotspot = Vector2I(10, 8);
	const Vector2I BuiltinResources::CursorArrowDragHotspot = Vector2I(8, 4);
	const Vector2I BuiltinResources::CursorArrowLeftRightHotspot = Vector2I(13, 9);
	const Vector2I BuiltinResources::CursorIBeamHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorDenyHotspot = Vector2I(15, 15);
	const Vector2I BuiltinResources::CursorWaitHotspot = Vector2I(15, 15);
	const Vector2I BuiltinResources::CursorSizeNESWHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorSizeNSHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorSizeNWSEHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorSizeWEHotspot = Vector2I(16, 15);

	/************************************************************************/
	/* 									SHADERS                      		*/
	/************************************************************************/

	const String BuiltinResources::ShaderSpriteTextFile = u8"SpriteText.bsl";
	const String BuiltinResources::ShaderSpriteImageFile = u8"SpriteImage.bsl";
	const String BuiltinResources::ShaderSpriteLineFile = u8"SpriteLine.bsl";

	constexpr const char* ShaderDiffuseFile = u8"Diffuse.bsl";
	constexpr const char* ShaderTransparentFile = u8"Transparent.bsl";
	constexpr const char* ShaderParticlesUnlitFile = u8"ParticlesUnlit.bsl";
	constexpr const char* ShaderParticlesLitFile = u8"ParticlesLit.bsl";
	constexpr const char* ShaderParticlesLitOpaqueFile = u8"ParticlesLitOpaque.bsl";
	constexpr const char* ShaderDecalFile = u8"Decal.bsl";

	BuiltinResources::~BuiltinResources()
	{
		mCursorArrow = nullptr;
		mCursorArrowDrag = nullptr;
		mCursorArrowLeftRight = nullptr;
		mCursorIBeam = nullptr;
		mCursorDeny = nullptr;
		mCursorWait = nullptr;
		mCursorSizeNESW = nullptr;
		mCursorSizeNS = nullptr;
		mCursorSizeNWSE = nullptr;
		mCursorSizeWE = nullptr;
		mFrameworkIcon = nullptr;
	}

	BuiltinResources::BuiltinResources()
	{
		// Set up paths
		mBuiltinRawDataFolder = Paths::GetDataPath() + u8"Raw/";

		mBuiltinDataFolder = Paths::GetDataPath();
		mEngineSkinSpritesFolder = mBuiltinDataFolder + SKIN_FOLDER + SPRITE_FOLDER;
		mEngineShaderFolder = mBuiltinDataFolder + SHADER_FOLDER;
		mEngineMeshFolder = mBuiltinDataFolder + MESH_FOLDER;
		mEngineCursorFolder = mBuiltinDataFolder + CURSOR_FOLDER;

		ResourceManifestPath = mBuiltinDataFolder + "ResourceManifest.asset";

		// Load manifest
		if (FileSystem::Exists(ResourceManifestPath))
			mResourceManifest = ResourceManifest::Load(ResourceManifestPath, mBuiltinDataFolder);

		if (mResourceManifest == nullptr)
			mResourceManifest = ResourceManifest::Create("BuiltinResources");

		gResources().RegisterResourceManifest(mResourceManifest);

		// Load basic resources
		mShaderSpriteText = GetShader(ShaderSpriteTextFile);
		mShaderSpriteImage = GetShader(ShaderSpriteImageFile);
		mShaderSpriteLine = GetShader(ShaderSpriteLineFile);
		mShaderDiffuse = GetShader(ShaderDiffuseFile);
		mShaderTransparent = GetShader(ShaderTransparentFile);
		mShaderParticlesUnlit = GetShader(ShaderParticlesUnlitFile);
		mShaderParticlesLit = GetShader(ShaderParticlesLitFile);
		mShaderParticlesLitOpaque = GetShader(ShaderParticlesLitOpaqueFile);
		mShaderDecal = GetShader(ShaderDecalFile);

		SPtr<PixelData> dummyPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

		dummyPixelData->SetColorAt(Color::Red, 0, 0);
		dummyPixelData->SetColorAt(Color::Red, 0, 1);
		dummyPixelData->SetColorAt(Color::Red, 1, 0);
		dummyPixelData->SetColorAt(Color::Red, 1, 1);

		mDummyTexture = Texture::Create(dummyPixelData);

		mWhiteSpriteTexture = GetSkinTexture(WhiteTex);
		mDummySpriteTexture = SpriteTexture::Create(mDummyTexture);

		mFont = gResources().Load<Font>(mBuiltinDataFolder + (String(DEFAULT_FONT_NAME) + u8".asset"));
		mSkin = gResources().Load<GUISkin>(mBuiltinDataFolder + (String(GUI_SKIN_FILE) + u8".json.asset"));
		mEmptySkin = GUISkin::Create();

		/************************************************************************/
		/* 								CURSOR		                     		*/
		/************************************************************************/

		HTexture cursorArrowTex = GetCursorTexture(CursorArrowTex);
		HTexture cursorArrowDragTex = GetCursorTexture(CursorArrowDragTex);
		HTexture cursorArrowLeftRightTex = GetCursorTexture(CursorArrowLeftRightTex);
		HTexture cursorIBeamTex = GetCursorTexture(CursorIBeamTex);
		HTexture cursorDenyTex = GetCursorTexture(CursorDenyTex);
		HTexture cursorWaitTex = GetCursorTexture(CursorWaitTex);
		HTexture cursorSizeNESWTex = GetCursorTexture(CursorSizeNESWTex);
		HTexture cursorSizeNSTex = GetCursorTexture(CursorSizeNSTex);
		HTexture cursorSizeNWSETex = GetCursorTexture(CursorSizeNWSETex);
		HTexture cursorSizeWETex = GetCursorTexture(CursorSizeWETex);

		mCursorArrow = cursorArrowTex->GetProperties().AllocBuffer(0, 0);
		cursorArrowTex->ReadData(mCursorArrow);

		mCursorArrowDrag = cursorArrowDragTex->GetProperties().AllocBuffer(0, 0);
		cursorArrowDragTex->ReadData(mCursorArrowDrag);

		mCursorArrowLeftRight = cursorArrowLeftRightTex->GetProperties().AllocBuffer(0, 0);
		cursorArrowLeftRightTex->ReadData(mCursorArrowLeftRight);

		mCursorIBeam = cursorIBeamTex->GetProperties().AllocBuffer(0, 0);
		cursorIBeamTex->ReadData(mCursorIBeam);

		mCursorDeny = cursorDenyTex->GetProperties().AllocBuffer(0, 0);
		cursorDenyTex->ReadData(mCursorDeny);

		mCursorWait = cursorWaitTex->GetProperties().AllocBuffer(0, 0);
		cursorWaitTex->ReadData(mCursorWait);

		mCursorSizeNESW = cursorSizeNESWTex->GetProperties().AllocBuffer(0, 0);
		cursorSizeNESWTex->ReadData(mCursorSizeNESW);

		mCursorSizeNS = cursorSizeNSTex->GetProperties().AllocBuffer(0, 0);
		cursorSizeNSTex->ReadData(mCursorSizeNS);

		mCursorSizeNWSE = cursorSizeNWSETex->GetProperties().AllocBuffer(0, 0);
		cursorSizeNWSETex->ReadData(mCursorSizeNWSE);

		mCursorSizeWE = cursorSizeWETex->GetProperties().AllocBuffer(0, 0);
		cursorSizeWETex->ReadData(mCursorSizeWE);

		/************************************************************************/
		/* 								ICON		                     		*/
		/************************************************************************/

		Path iconPath = mBuiltinDataFolder + ICON_FOLDER;
		iconPath.Append(String(IconTextureName) + u8".asset");

		HTexture iconTex = gResources().Load<Texture>(iconPath);

		mFrameworkIcon = iconTex->GetProperties().AllocBuffer(0, 0);
		iconTex->ReadData(mFrameworkIcon);

		gCoreThread().Submit(true);
	}

	HSpriteTexture BuiltinResources::GetSkinTexture(const String& name) const
	{
		Path texturePath = mEngineSkinSpritesFolder;
		texturePath.Append(u8"sprite_" + name + u8".asset");

		return gResources().Load<SpriteTexture>(texturePath);
	}

	HShader BuiltinResources::GetShader(const Path& path) const
	{
		Path programPath = mEngineShaderFolder;
		programPath.Append(path);
		programPath.SetExtension(programPath.GetExtension() + ".asset");

		return gResources().Load<Shader>(programPath);
	}

	HTexture BuiltinResources::GetCursorTexture(const String& name) const
	{
		Path cursorPath = mEngineCursorFolder;
		cursorPath.Append(name + u8".asset");

		return gResources().Load<Texture>(cursorPath);
	}

	const PixelData& BuiltinResources::GetCursorArrow(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowHotspot;
		return *mCursorArrow.get();
	}

	const PixelData& BuiltinResources::GetCursorArrowDrag(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowDragHotspot;
		return *mCursorArrowDrag.get();
	}

	const PixelData& BuiltinResources::GetCursorWait(Vector2I& hotSpot)
	{
		hotSpot = CursorWaitHotspot;
		return *mCursorWait.get();
	}

	const PixelData& BuiltinResources::GetCursorIBeam(Vector2I& hotSpot)
	{
		hotSpot = CursorIBeamHotspot;
		return *mCursorIBeam.get();
	}

	const PixelData& BuiltinResources::GetCursorSizeNesw(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNESWHotspot;
		return *mCursorSizeNESW.get();
	}

	const PixelData& BuiltinResources::GetCursorSizeNs(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNSHotspot;
		return *mCursorSizeNS.get();
	}

	const PixelData& BuiltinResources::GetCursorSizeNwse(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNWSEHotspot;
		return *mCursorSizeNWSE.get();
	}

	const PixelData& BuiltinResources::GetCursorSizeWe(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeWEHotspot;
		return *mCursorSizeWE.get();
	}

	const PixelData& BuiltinResources::GetCursorDeny(Vector2I& hotSpot)
	{
		hotSpot = CursorDenyHotspot;
		return *mCursorDeny.get();
	}

	const PixelData& BuiltinResources::GetCursorMoveLeftRight(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowLeftRightHotspot;
		return *mCursorArrowLeftRight.get();
	}

	const PixelData& BuiltinResources::GetFrameworkIcon()
	{
		return *mFrameworkIcon.get();
	}

	Path BuiltinResources::GetRawShaderFolder()
	{
		return Paths::GetDataPath() + "Raw/" + SHADER_FOLDER;
	}

	Path BuiltinResources::GetShaderIncludeFolder()
	{
		return Paths::GetDataPath() + SHADER_INCLUDE_FOLDER;
	}

	Path BuiltinResources::GetIconFolder()
	{
		return Paths::GetDataPath() + ICON_FOLDER;
	}

#if BS_IS_BANSHEE3D || defined BS_IS_ASSET_TOOL
	Path BuiltinResources::GetEditorShaderIncludeFolder()
	{
		return Paths::GetEditorDataPath() + SHADER_INCLUDE_FOLDER;
	}
#endif

	HMesh BuiltinResources::GetMesh(BuiltinMesh mesh) const
	{
		Path meshPath = mEngineMeshFolder;

		switch (mesh)
		{
		case BuiltinMesh::Box:
			meshPath.Append(MESH_BOX_FILE);
			break;
		case BuiltinMesh::Sphere:
			meshPath.Append(MESH_SPHERE_FILE);
			break;
		case BuiltinMesh::Cone:
			meshPath.Append(MESH_CONE_FILE);
			break;
		case BuiltinMesh::Cylinder:
			meshPath.Append(MESH_CYLINDER_FILE);
			break;
		case BuiltinMesh::Quad:
			meshPath.Append(MESH_QUAD_FILE);
			break;
		case BuiltinMesh::Disc:
			meshPath.Append(MESH_DISC_FILE);
			break;
		}

		return gResources().Load<Mesh>(meshPath);
	}

	HShader BuiltinResources::GetBuiltinShader(BuiltinShader type) const
	{
		switch(type)
		{
		case BuiltinShader::Standard:
			return mShaderDiffuse;
		case BuiltinShader::Transparent:
			return mShaderTransparent;
		case BuiltinShader::ParticlesUnlit:
			return mShaderParticlesUnlit;
		case BuiltinShader::ParticlesLit:
			return mShaderParticlesLit;
		case BuiltinShader::ParticlesLitOpaque:
			return mShaderParticlesLitOpaque;
		case BuiltinShader::Decal:
			return mShaderDecal;
		default:
			break;
		}

		return HShader();
	}

	HTexture BuiltinResources::GetTexture(BuiltinTexture type)
	{
		Path texturePath = Paths::GetDataPath();
		texturePath.Append(TEXTURE_FOLDER);

		switch (type)
		{
		case BuiltinTexture::Black:
			texturePath.Append(TEXTURE_BLACK_FILE);
			break;
		case BuiltinTexture::White:
			texturePath.Append(TEXTURE_WHITE_FILE);
			break;
		case BuiltinTexture::Normal:
			texturePath.Append(TEXTURE_NORMAL_FILE);
			break;
		case BuiltinTexture::BokehFlare:
			texturePath.Append(u8"BokehHex.png.asset");
			break;
		}

		return gResources().Load<Texture>(texturePath);
	}

	HMaterial BuiltinResources::CreateSpriteTextMaterial() const
	{
		return Material::Create(mShaderSpriteText);
	}

	HMaterial BuiltinResources::CreateSpriteImageMaterial() const
	{
		return Material::Create(mShaderSpriteImage);
	}

	HMaterial BuiltinResources::CreateSpriteLineMaterial() const
	{
		return Material::Create(mShaderSpriteLine);
	}

	BuiltinResources& gBuiltinResources()
	{
		return BuiltinResources::Instance();
	}
}
