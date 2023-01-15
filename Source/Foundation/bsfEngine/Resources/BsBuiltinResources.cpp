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

using namespace bs;

constexpr const char* BuiltinResources::kIconTextureName;
constexpr const char* BuiltinResources::kMultiLineLabelStyle;

/************************************************************************/
/* 								GUI TEXTURES                      		*/
/************************************************************************/

const String BuiltinResources::kWhiteTex = u8"White.psd";

/************************************************************************/
/* 							CURSOR TEXTURES                      		*/
/************************************************************************/

const String BuiltinResources::kCursorArrowTex = u8"Arrow.psd";
const String BuiltinResources::kCursorArrowDragTex = u8"ArrowDrag.psd";
const String BuiltinResources::kCursorArrowLeftRightTex = u8"ArrowLeftRight.psd";
const String BuiltinResources::kCursorIBeamTex = u8"IBeam.psd";
const String BuiltinResources::kCursorDenyTex = u8"Deny.psd";
const String BuiltinResources::kCursorWaitTex = u8"Wait.psd";
const String BuiltinResources::kCursorSizeNeswTex = u8"SizeNESW.psd";
const String BuiltinResources::kCursorSizeNsTex = u8"SizeNS.psd";
const String BuiltinResources::kCursorSizeNwseTex = u8"SizeNWSE.psd";
const String BuiltinResources::kCursorSizeWeTex = u8"SizeWE.psd";

const Vector2I BuiltinResources::kCursorArrowHotspot = Vector2I(10, 8);
const Vector2I BuiltinResources::kCursorArrowDragHotspot = Vector2I(8, 4);
const Vector2I BuiltinResources::kCursorArrowLeftRightHotspot = Vector2I(13, 9);
const Vector2I BuiltinResources::kCursorIBeamHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorDenyHotspot = Vector2I(15, 15);
const Vector2I BuiltinResources::kCursorWaitHotspot = Vector2I(15, 15);
const Vector2I BuiltinResources::kCursorSizeNeswHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorSizeNsHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorSizeNwseHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorSizeWeHotspot = Vector2I(16, 15);

/************************************************************************/
/* 									SHADERS                      		*/
/************************************************************************/

const String BuiltinResources::kShaderSpriteTextFile = u8"SpriteText.bsl";
const String BuiltinResources::kShaderSpriteImageFile = u8"SpriteImage.bsl";
const String BuiltinResources::kShaderSpriteLineFile = u8"SpriteLine.bsl";

constexpr const char* kShaderDiffuseFile = u8"Diffuse.bsl";
constexpr const char* kShaderTransparentFile = u8"Transparent.bsl";
constexpr const char* kShaderParticlesUnlitFile = u8"ParticlesUnlit.bsl";
constexpr const char* kShaderParticlesLitFile = u8"ParticlesLit.bsl";
constexpr const char* kShaderParticlesLitOpaqueFile = u8"ParticlesLitOpaque.bsl";
constexpr const char* kShaderDecalFile = u8"Decal.bsl";

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
	mEngineSkinSpritesFolder = mBuiltinDataFolder + kSkinFolder + kSpriteFolder;
	mEngineShaderFolder = mBuiltinDataFolder + kShaderFolder;
	mEngineMeshFolder = mBuiltinDataFolder + kMeshFolder;
	mEngineCursorFolder = mBuiltinDataFolder + kCursorFolder;

	ResourceManifestPath = mBuiltinDataFolder + "ResourceManifest.asset";

	// Load manifest
	if(FileSystem::Exists(ResourceManifestPath))
		mResourceManifest = ResourceManifest::Load(ResourceManifestPath, mBuiltinDataFolder);

	if(mResourceManifest == nullptr)
		mResourceManifest = ResourceManifest::Create("BuiltinResources");

	GetResources().RegisterResourceManifest(mResourceManifest);

	// Load basic resources
	mShaderSpriteText = GetShader(kShaderSpriteTextFile);
	mShaderSpriteImage = GetShader(kShaderSpriteImageFile);
	mShaderSpriteLine = GetShader(kShaderSpriteLineFile);
	mShaderDiffuse = GetShader(kShaderDiffuseFile);
	mShaderTransparent = GetShader(kShaderTransparentFile);
	mShaderParticlesUnlit = GetShader(kShaderParticlesUnlitFile);
	mShaderParticlesLit = GetShader(kShaderParticlesLitFile);
	mShaderParticlesLitOpaque = GetShader(kShaderParticlesLitOpaqueFile);
	mShaderDecal = GetShader(kShaderDecalFile);

	SPtr<PixelData> dummyPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

	dummyPixelData->SetColorAt(Color::kRed, 0, 0);
	dummyPixelData->SetColorAt(Color::kRed, 0, 1);
	dummyPixelData->SetColorAt(Color::kRed, 1, 0);
	dummyPixelData->SetColorAt(Color::kRed, 1, 1);

	mDummyTexture = Texture::Create(dummyPixelData);

	mWhiteSpriteTexture = GetSkinTexture(kWhiteTex);
	mDummySpriteTexture = SpriteTexture::Create(mDummyTexture);

	mFont = GetResources().Load<Font>(mBuiltinDataFolder + (String(kDefaultFontName) + u8".asset"));
	mSkin = GetResources().Load<GUISkin>(mBuiltinDataFolder + (String(kGuiSkinFile) + u8".json.asset"));
	mEmptySkin = GUISkin::Create();

	/************************************************************************/
	/* 								CURSOR		                     		*/
	/************************************************************************/

	HTexture cursorArrowTex = GetCursorTexture(kCursorArrowTex);
	HTexture cursorArrowDragTex = GetCursorTexture(kCursorArrowDragTex);
	HTexture cursorArrowLeftRightTex = GetCursorTexture(kCursorArrowLeftRightTex);
	HTexture cursorIBeamTex = GetCursorTexture(kCursorIBeamTex);
	HTexture cursorDenyTex = GetCursorTexture(kCursorDenyTex);
	HTexture cursorWaitTex = GetCursorTexture(kCursorWaitTex);
	HTexture cursorSizeNESWTex = GetCursorTexture(kCursorSizeNeswTex);
	HTexture cursorSizeNSTex = GetCursorTexture(kCursorSizeNsTex);
	HTexture cursorSizeNWSETex = GetCursorTexture(kCursorSizeNwseTex);
	HTexture cursorSizeWETex = GetCursorTexture(kCursorSizeWeTex);

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

	Path iconPath = mBuiltinDataFolder + kIconFolder;
	iconPath.Append(String(kIconTextureName) + u8".asset");

	HTexture iconTex = GetResources().Load<Texture>(iconPath);

	mFrameworkIcon = iconTex->GetProperties().AllocBuffer(0, 0);
	iconTex->ReadData(mFrameworkIcon);

	GetCoreThread().Submit(true);
}

HSpriteTexture BuiltinResources::GetSkinTexture(const String& name) const
{
	Path texturePath = mEngineSkinSpritesFolder;
	texturePath.Append(u8"sprite_" + name + u8".asset");

	return GetResources().Load<SpriteTexture>(texturePath);
}

HShader BuiltinResources::GetShader(const Path& path) const
{
	Path programPath = mEngineShaderFolder;
	programPath.Append(path);
	programPath.SetExtension(programPath.GetExtension() + ".asset");

	return GetResources().Load<Shader>(programPath);
}

HTexture BuiltinResources::GetCursorTexture(const String& name) const
{
	Path cursorPath = mEngineCursorFolder;
	cursorPath.Append(name + u8".asset");

	return GetResources().Load<Texture>(cursorPath);
}

const PixelData& BuiltinResources::GetCursorArrow(Vector2I& hotSpot)
{
	hotSpot = kCursorArrowHotspot;
	return *mCursorArrow.get();
}

const PixelData& BuiltinResources::GetCursorArrowDrag(Vector2I& hotSpot)
{
	hotSpot = kCursorArrowDragHotspot;
	return *mCursorArrowDrag.get();
}

const PixelData& BuiltinResources::GetCursorWait(Vector2I& hotSpot)
{
	hotSpot = kCursorWaitHotspot;
	return *mCursorWait.get();
}

const PixelData& BuiltinResources::GetCursorIBeam(Vector2I& hotSpot)
{
	hotSpot = kCursorIBeamHotspot;
	return *mCursorIBeam.get();
}

const PixelData& BuiltinResources::GetCursorSizeNesw(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeNeswHotspot;
	return *mCursorSizeNESW.get();
}

const PixelData& BuiltinResources::GetCursorSizeNs(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeNsHotspot;
	return *mCursorSizeNS.get();
}

const PixelData& BuiltinResources::GetCursorSizeNwse(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeNwseHotspot;
	return *mCursorSizeNWSE.get();
}

const PixelData& BuiltinResources::GetCursorSizeWe(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeWeHotspot;
	return *mCursorSizeWE.get();
}

const PixelData& BuiltinResources::GetCursorDeny(Vector2I& hotSpot)
{
	hotSpot = kCursorDenyHotspot;
	return *mCursorDeny.get();
}

const PixelData& BuiltinResources::GetCursorMoveLeftRight(Vector2I& hotSpot)
{
	hotSpot = kCursorArrowLeftRightHotspot;
	return *mCursorArrowLeftRight.get();
}

const PixelData& BuiltinResources::GetFrameworkIcon()
{
	return *mFrameworkIcon.get();
}

Path BuiltinResources::GetRawShaderFolder()
{
	return Paths::GetDataPath() + "Raw/" + kShaderFolder;
}

Path BuiltinResources::GetShaderIncludeFolder()
{
	return Paths::GetDataPath() + kShaderIncludeFolder;
}

Path BuiltinResources::GetIconFolder()
{
	return Paths::GetDataPath() + kIconFolder;
}

#if B3D_IS_ENGINE || defined BS_IS_ASSET_TOOL
Path BuiltinResources::GetEditorShaderIncludeFolder()
{
	return Paths::GetEditorDataPath() + kShaderIncludeFolder;
}
#endif

HMesh BuiltinResources::GetMesh(BuiltinMesh mesh) const
{
	Path meshPath = mEngineMeshFolder;

	switch(mesh)
	{
	case BuiltinMesh::Box:
		meshPath.Append(kMeshBoxFile);
		break;
	case BuiltinMesh::Sphere:
		meshPath.Append(kMeshSphereFile);
		break;
	case BuiltinMesh::Cone:
		meshPath.Append(kMeshConeFile);
		break;
	case BuiltinMesh::Cylinder:
		meshPath.Append(kMeshCylinderFile);
		break;
	case BuiltinMesh::Quad:
		meshPath.Append(kMeshQuadFile);
		break;
	case BuiltinMesh::Disc:
		meshPath.Append(kMeshDiscFile);
		break;
	}

	return GetResources().Load<Mesh>(meshPath);
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
	texturePath.Append(kTextureFolder);

	switch(type)
	{
	case BuiltinTexture::Black:
		texturePath.Append(kTextureBlackFile);
		break;
	case BuiltinTexture::White:
		texturePath.Append(kTextureWhiteFile);
		break;
	case BuiltinTexture::Normal:
		texturePath.Append(kTextureNormalFile);
		break;
	case BuiltinTexture::BokehFlare:
		texturePath.Append(u8"BokehHex.png.asset");
		break;
	case BuiltinTexture::Black3D:
		texturePath.Append(kTextureBlack3DFile);
		break;
	case BuiltinTexture::White3D:
		texturePath.Append(kTextureWhite3DFile);
		break;
	}

	return GetResources().Load<Texture>(texturePath);
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

namespace bs
{
BuiltinResources& GetBuiltinResources()
{
	return BuiltinResources::Instance();
}
} // namespace bs
