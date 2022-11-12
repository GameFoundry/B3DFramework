#include "BsApplication.h"
#include "BsEngineConfig.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "BsBuiltinResourcesHelper.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsPath.h"
#include "FileSystem/BsDataStream.h"
#include "Resources/BsResourceManifest.h"
#include "ThirdParty/json.hpp"
#include "Utility/BsShapeMeshes3D.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsRendererMeshData.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Serialization/BsFileSerializer.h"
#include "Importer/BsImporter.h"
#include "Importer/BsTextureImportOptions.h"

using namespace bs;

static constexpr const char* kTimestampName = u8"Timestamp.asset";
static constexpr const char* kManifestName = u8"ResourceManifest.asset";
static constexpr const char* kDependenciesJsonName = u8"ShaderDependencies.json";
static constexpr const char* kDataListJson = u8"DataList.json";
static constexpr const char* kGuiSkinJson = u8"GUISkin.json";

static Path sInputFolder;
static Path sOutputFolder;
static Path sManifestPath;
static SPtr<ResourceManifest> sManifest;

void ProcessAssets(bool, bool, time_t);

int main(int argc, char* argv[])
{
	using namespace bs;

	if(argc < 3)
		return 2;

	START_UP_DESC desc;
	desc.RenderApi = B3D_RENDER_BACKEND;
	desc.Renderer = B3D_RENDERER;
	desc.Audio = B3D_AUDIO_BACKEND;
	desc.Physics = B3D_PHYSICS_BACKEND;

	desc.Importers.push_back("bsfFreeImgImporter");
	desc.Importers.push_back("bsfFBXImporter");
	desc.Importers.push_back("bsfFontImporter");
	desc.Importers.push_back("bsfSL");

	desc.PrimaryWindowDesc.VideoMode = VideoMode(64, 64);
	desc.PrimaryWindowDesc.Fullscreen = false;
	desc.PrimaryWindowDesc.Title = "bsf importer";
	desc.PrimaryWindowDesc.Hidden = true;

	Application::StartUp(desc);

	sInputFolder = argv[1];
	sOutputFolder = argv[2];
	sManifestPath = sOutputFolder + kManifestName;

	bool generateGenerated = true;
	bool forceImport = false;
	for(int i = 3; i < argc; i++)
	{
		if(strcmp(argv[i], "--editor") == 0)
			generateGenerated = false;
		else if(strcmp(argv[i], "--force") == 0)
			forceImport = true;
	}

	if(FileSystem::Exists(sInputFolder))
	{
		time_t lastUpdateTime;
		u32 modifications = BuiltinResourcesHelper::CheckForModifications(
			sInputFolder,
			sOutputFolder + kTimestampName,
			lastUpdateTime);

		if(forceImport)
			modifications = 2;

		// Check if manifest needs to be rebuilt
		if(modifications == 0 && !FileSystem::Exists(sManifestPath))
			modifications = 1;

		if(modifications > 0)
		{
			const bool fullReimport = modifications == 2;

			sManifest = ResourceManifest::Create("BuiltinResources");
			GetResources().RegisterResourceManifest(sManifest);

			ProcessAssets(generateGenerated, fullReimport, lastUpdateTime);
			BuiltinResourcesHelper::WriteTimestamp(sOutputFolder + kTimestampName);

			ResourceManifest::Save(sManifest, sManifestPath, sOutputFolder);

			Application::ShutDown();
			return 1;
		}
	}

	Application::ShutDown();
	return 0;
}

using namespace bs;

void GenerateTextures()
{
	SPtr<PixelData> blackPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	blackPixelData->SetColorAt(Color::kBlack, 0, 0);
	blackPixelData->SetColorAt(Color::kBlack, 0, 1);
	blackPixelData->SetColorAt(Color::kBlack, 1, 0);
	blackPixelData->SetColorAt(Color::kBlack, 1, 1);

	SPtr<Texture> blackTexture = Texture::CreatePtrInternal(blackPixelData);

	SPtr<PixelData> whitePixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	whitePixelData->SetColorAt(Color::kWhite, 0, 0);
	whitePixelData->SetColorAt(Color::kWhite, 0, 1);
	whitePixelData->SetColorAt(Color::kWhite, 1, 0);
	whitePixelData->SetColorAt(Color::kWhite, 1, 1);

	SPtr<Texture> whiteTexture = Texture::CreatePtrInternal(whitePixelData);

	SPtr<PixelData> normalPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

	Color encodedNormal(0.5f, 0.5f, 1.0f);
	normalPixelData->SetColorAt(encodedNormal, 0, 0);
	normalPixelData->SetColorAt(encodedNormal, 0, 1);
	normalPixelData->SetColorAt(encodedNormal, 1, 0);
	normalPixelData->SetColorAt(encodedNormal, 1, 1);

	SPtr<Texture> normalTexture = Texture::CreatePtrInternal(normalPixelData);

	// Save all textures
	Path outputDir = sOutputFolder + BuiltinResources::kTextureFolder;

	auto saveTexture = [&](const Path& path, const SPtr<Texture>& texture, const String& uuid)
	{
		HResource textureResource = GetResources().CreateResourceHandleInternal(texture, UUID(uuid));

		GetResources().Save(textureResource, path, true);
		sManifest->RegisterResource(textureResource.GetUuid(), path);
	};

	Path whitePath = outputDir + BuiltinResources::kTextureWhiteFile;
	saveTexture(whitePath, whiteTexture, "1f7d0e3f-d81b-42ee-9d31-cb6c6fc55824");

	Path blackPath = outputDir + BuiltinResources::kTextureBlackFile;
	saveTexture(blackPath, blackTexture, "149a5c05-9570-4915-9dbd-69acf88b865b");

	Path normalPath = outputDir + BuiltinResources::kTextureNormalFile;
	saveTexture(normalPath, normalTexture, "afb29163-1ef0-4440-9cfb-c1ebb3b3d452");
}

void GenerateMeshes()
{
	SPtr<VertexDataDesc> vertexDesc = B3DMakeShared<VertexDataDesc>();
	vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
	vertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);
	vertexDesc->AddVertElem(VET_FLOAT3, VES_NORMAL);
	vertexDesc->AddVertElem(VET_FLOAT4, VES_TANGENT);
	vertexDesc->AddVertElem(VET_COLOR, VES_COLOR);

	u32 boxNumVertices = 0;
	u32 boxNumIndices = 0;
	ShapeMeshes3D::GetNumElementsAaBox(boxNumVertices, boxNumIndices);
	SPtr<MeshData> boxMeshData = MeshData::Create(boxNumVertices, boxNumIndices, vertexDesc);
	AABox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

	ShapeMeshes3D::SolidAaBox(box, boxMeshData, 0, 0);
	SPtr<Mesh> boxMesh = Mesh::CreateShared(RendererMeshData::Convert(boxMeshData));

	u32 sphereNumVertices = 0;
	u32 sphereNumIndices = 0;
	ShapeMeshes3D::GetNumElementsSphere(3, sphereNumVertices, sphereNumIndices);
	SPtr<MeshData> sphereMeshData = B3DMakeShared<MeshData>(sphereNumVertices, sphereNumIndices, vertexDesc);

	ShapeMeshes3D::SolidSphere(Sphere(Vector3::kZero, 1.0f), sphereMeshData, 0, 0, 3);
	SPtr<Mesh> sphereMesh = Mesh::CreateShared(RendererMeshData::Convert(sphereMeshData));

	u32 coneNumVertices = 0;
	u32 coneNumIndices = 0;
	ShapeMeshes3D::GetNumElementsCone(10, coneNumVertices, coneNumIndices);
	SPtr<MeshData> coneMeshData = B3DMakeShared<MeshData>(coneNumVertices, coneNumIndices, vertexDesc);

	ShapeMeshes3D::SolidCone(Vector3::kZero, Vector3::kUnitY, 1.0f, 1.0f, Vector2::kOne, coneMeshData, 0, 0);
	SPtr<Mesh> coneMesh = Mesh::CreateShared(RendererMeshData::Convert(coneMeshData));

	u32 cylinderNumVertices = 0;
	u32 cylinderNumIndices = 0;
	ShapeMeshes3D::GetNumElementsCylinder(10, cylinderNumVertices, cylinderNumIndices);
	SPtr<MeshData> cylinderMeshData = B3DMakeShared<MeshData>(cylinderNumVertices, cylinderNumIndices, vertexDesc);

	ShapeMeshes3D::SolidCylinder(Vector3::kZero, Vector3::kUnitY, 1.0f, 1.0f, Vector2::kOne, cylinderMeshData, 0, 0);
	SPtr<Mesh> cylinderMesh = Mesh::CreateShared(RendererMeshData::Convert(cylinderMeshData));

	u32 quadNumVertices = 8;
	u32 quadNumIndices = 12;
	ShapeMeshes3D::GetNumElementsQuad(quadNumVertices, quadNumIndices);
	SPtr<MeshData> quadMeshData = B3DMakeShared<MeshData>(quadNumVertices, quadNumIndices, vertexDesc);

	std::array<Vector3, 2> axes = { { Vector3::kUnitX, Vector3::kUnitZ } };
	std::array<float, 2> sizes = { { 1.0f, 1.0f } };
	Rect3 rect(Vector3::kZero, axes, sizes);
	ShapeMeshes3D::SolidQuad(rect, quadMeshData, 0, 0);
	SPtr<Mesh> quadMesh = Mesh::CreateShared(RendererMeshData::Convert(quadMeshData));

	u32 discNumVertices = 0;
	u32 discNumIndices = 0;
	ShapeMeshes3D::GetNumElementsDisc(10, discNumVertices, discNumIndices);
	SPtr<MeshData> discMeshData = B3DMakeShared<MeshData>(discNumVertices, discNumIndices, vertexDesc);

	ShapeMeshes3D::SolidDisc(Vector3::kZero, 1.0f, Vector3::kUnitY, discMeshData, 0, 0);
	SPtr<Mesh> discMesh = Mesh::CreateShared(RendererMeshData::Convert(discMeshData));

	// Save all meshes
	const Path outputDir = sOutputFolder + BuiltinResources::kMeshFolder;

	auto saveMesh = [&](const Path& path, const SPtr<Mesh>& mesh, const String& uuid)
	{
		HResource meshResource = GetResources().CreateResourceHandleInternal(mesh, UUID(uuid));

		GetResources().Save(meshResource, path, true);
		sManifest->RegisterResource(meshResource.GetUuid(), path);
	};

	Path boxPath = outputDir + BuiltinResources::kMeshBoxFile;
	saveMesh(boxPath, boxMesh, "bc1d20ca-7fe6-489b-8b5c-dbf798badc95");

	Path spherePath = outputDir + BuiltinResources::kMeshSphereFile;
	saveMesh(spherePath, sphereMesh, "040642f3-04d6-419e-9dba-f7824161c205");

	Path conePath = outputDir + BuiltinResources::kMeshConeFile;
	saveMesh(conePath, coneMesh, "b8cf6db5-1736-47ac-852f-82ecd88b4d46");

	Path cylinderPath = outputDir + BuiltinResources::kMeshCylinderFile;
	saveMesh(cylinderPath, cylinderMesh, "e6b2b797-4e72-7e49-61ba-4e7275bd561d");

	Path quadPath = outputDir + BuiltinResources::kMeshQuadFile;
	saveMesh(quadPath, quadMesh, "06592bf3-f82a-472e-a034-26a98225fbe1");

	Path discPath = outputDir + BuiltinResources::kMeshDiscFile;
	saveMesh(discPath, discMesh, "6f496313-344a-495c-83e8-152e3053c52d");
}

SPtr<GUISkin> GenerateGuiSkin()
{
	using nlohmann::json;

	const Path skinFolder = sOutputFolder + BuiltinResources::kSkinFolder + BuiltinResources::kSpriteFolder;
	BuiltinResourceGUIElementStyleLoader loader(sOutputFolder, skinFolder);

	Path guiSkinPath = sInputFolder + kGuiSkinJson;
	SPtr<DataStream> guiSkinStream = FileSystem::OpenFile(guiSkinPath);
	json guiSkinJSON = json::parse(guiSkinStream->GetAsString().c_str());

	SPtr<GUISkin> skin = GUISkin::CreatePtrInternal();

	for(auto& entry : guiSkinJSON)
	{
		std::string name = entry["name"];

		GUIElementStyle style = BuiltinResourcesHelper::LoadGuiStyleFromJson(entry, loader);
		skin->SetStyle(name.c_str(), style);
	}

	return skin;
}

void ProcessAssets(bool generateGenerated, bool forceImport, time_t lastUpdateTime)
{
	using nlohmann::json;

	// Hidden dependency: Textures need to be generated before shaders as they may use the default textures
	if(generateGenerated)
	{
		GenerateTextures();
		GenerateMeshes();
	}

	const Path dataListsFilePath = sInputFolder + kDataListJson;
	SPtr<DataStream> dataListStream = FileSystem::OpenFile(dataListsFilePath);
	json dataListJSON = json::parse(dataListStream->GetAsString().c_str());

	json skinJSON = dataListJSON["Skin"];
	json animatedSpritesJSON = dataListJSON["AnimatedSprites"];
	json cursorsJSON = dataListJSON["Cursors"];
	json iconsJSON = dataListJSON["Icons"];
	json spriteIconsJSON = dataListJSON["SpriteIcons"];
	json spriteIcons3DJSON = dataListJSON["SpriteIcons3D"];
	json includesJSON = dataListJSON["Includes"];
	json shadersJSON = dataListJSON["Shaders"];
	json fontsJSON = dataListJSON["Fonts"];
	json guiSkinJSON = dataListJSON["GUISkin"];
	json splashScreenJSON = dataListJSON["SplashScreen"];
	json texturesJSON = dataListJSON["Textures"];

	const Path rawSkinFolder = sInputFolder + BuiltinResources::kSkinFolder;
	const Path rawAnimatedSpritesFolder = sInputFolder + BuiltinResources::kAnimatedSpritesFolder;
	const Path rawCursorFolder = sInputFolder + BuiltinResources::kCursorFolder;
	const Path rawIconFolder = sInputFolder + BuiltinResources::kIconFolder;
	const Path rawIcon3DFolder = sInputFolder + BuiltinResources::kIcoN3DFolder;
	const Path rawShaderFolder = sInputFolder + BuiltinResources::kShaderFolder;
	const Path rawShaderIncludeFolder = sInputFolder + BuiltinResources::kShaderIncludeFolder;
	const Path rawTexturesFolder = sInputFolder + BuiltinResources::kTextureFolder;

	// Update DataList.json if needed
	bool updatedDataLists = false;

	if(!cursorsJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawCursorFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			cursorsJSON);
	}

	if(!iconsJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawIconFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			iconsJSON);
	}

	if(!spriteIconsJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawIconFolder,
			BuiltinResourcesHelper::AssetType::Sprite,
			spriteIconsJSON);
	}

	if(!spriteIcons3DJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawIcon3DFolder,
			BuiltinResourcesHelper::AssetType::Sprite,
			spriteIcons3DJSON);
	}

	if(!includesJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawShaderIncludeFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			includesJSON);
	}

	if(!shadersJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawShaderFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			shadersJSON);
	}

	if(!skinJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawSkinFolder,
			BuiltinResourcesHelper::AssetType::Sprite,
			skinJSON);
	}

	if(!texturesJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawTexturesFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			texturesJSON);
	}

	dataListStream->Close();

	if(updatedDataLists)
	{
		FileSystem::Remove(dataListsFilePath);

		if(!skinJSON.is_null())
			dataListJSON["Skin"] = skinJSON;

		if(!animatedSpritesJSON.is_null())
			dataListJSON["AnimatedSprites"] = animatedSpritesJSON;

		if(!cursorsJSON.is_null())
			dataListJSON["Cursors"] = cursorsJSON;

		if(!iconsJSON.is_null())
			dataListJSON["Icons"] = iconsJSON;

		if(!spriteIconsJSON.is_null())
			dataListJSON["SpriteIcons"] = spriteIconsJSON;

		if(!spriteIcons3DJSON.is_null())
			dataListJSON["SpriteIcons3D"] = spriteIcons3DJSON;

		if(!includesJSON.is_null())
			dataListJSON["Includes"] = includesJSON;

		if(!shadersJSON.is_null())
			dataListJSON["Shaders"] = shadersJSON;

		if(!fontsJSON.is_null())
			dataListJSON["Fonts"] = fontsJSON;

		if(!guiSkinJSON.is_null())
			dataListJSON["GUISkin"] = guiSkinJSON;

		if(!splashScreenJSON.is_null())
			dataListJSON["SplashScreen"] = splashScreenJSON;

		if(!texturesJSON.is_null())
			dataListJSON["Textures"] = texturesJSON;

		String jsonString = dataListJSON.dump(4).c_str();
		dataListStream = FileSystem::CreateAndOpenFile(dataListsFilePath);
		dataListStream->WriteString(jsonString);
		dataListStream->Close();
	}

	const Path skinFolder = sOutputFolder + BuiltinResources::kSkinFolder;
	const Path animatedSpriteFolder = sOutputFolder + BuiltinResources::kAnimatedSpritesFolder;
	const Path cursorFolder = sOutputFolder + BuiltinResources::kCursorFolder;
	const Path iconFolder = sOutputFolder + BuiltinResources::kIconFolder;
	const Path icon3DFolder = sOutputFolder + BuiltinResources::kIcoN3DFolder;
	const Path shaderFolder = sOutputFolder + BuiltinResources::kShaderFolder;
	const Path shaderIncludeFolder = sOutputFolder + BuiltinResources::kShaderIncludeFolder;
	const Path texturesFolder = sOutputFolder + BuiltinResources::kTextureFolder;
	const Path shaderDependenciesFile = sInputFolder + kDependenciesJsonName;

	// If forcing import, clear all data folders since everything will be recreated anyway
	if(forceImport)
	{
		if(FileSystem::Exists(cursorFolder))
			FileSystem::Remove(cursorFolder);

		if(FileSystem::Exists(iconFolder))
			FileSystem::Remove(iconFolder);

		if(FileSystem::Exists(icon3DFolder))
			FileSystem::Remove(icon3DFolder);

		if(FileSystem::Exists(shaderIncludeFolder))
			FileSystem::Remove(shaderIncludeFolder);

		if(FileSystem::Exists(shaderFolder))
			FileSystem::Remove(shaderFolder);

		if(FileSystem::Exists(skinFolder))
			FileSystem::Remove(skinFolder);

		if(FileSystem::Exists(animatedSpriteFolder))
			FileSystem::Remove(animatedSpriteFolder);

		FileSystem::Remove(shaderDependenciesFile);
	}

	// Read shader dependencies JSON
	json shaderDependenciesJSON;
	if(FileSystem::Exists(shaderDependenciesFile))
	{
		SPtr<DataStream> stream = FileSystem::OpenFile(shaderDependenciesFile);
		shaderDependenciesJSON = json::parse(stream->GetAsString().c_str());
		stream->Close();
	}

	// Import cursors
	if(!cursorsJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			cursorFolder,
			cursorsJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);

		Vector<bool> importFlags = BuiltinResourcesHelper::GenerateImportFlags(
			cursorsJSON,
			rawCursorFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			cursorsJSON,
			importFlags,
			rawCursorFolder,
			cursorFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);
	}

	// Import icons
	if(!iconsJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			iconFolder,
			iconsJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);

		Vector<bool> importFlags = BuiltinResourcesHelper::GenerateImportFlags(
			iconsJSON,
			rawIconFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			iconsJSON,
			importFlags,
			rawIconFolder,
			iconFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);
	}

	// Import sprite icons
	if(!spriteIconsJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			iconFolder,
			spriteIconsJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);

		Vector<bool> importFlags = BuiltinResourcesHelper::GenerateImportFlags(
			spriteIconsJSON,
			rawIconFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			spriteIconsJSON,
			importFlags,
			rawIconFolder,
			iconFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);
	}

	// Import 3D sprite icons
	if(!spriteIcons3DJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			icon3DFolder,
			spriteIcons3DJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);

		Vector<bool> importFlags = BuiltinResourcesHelper::GenerateImportFlags(
			spriteIcons3DJSON,
			rawIcon3DFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			spriteIcons3DJSON,
			importFlags,
			rawIcon3DFolder,
			icon3DFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite,
			nullptr, false, true);
	}

	// Import shaders
	if(!shadersJSON.is_null() && !includesJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			shaderIncludeFolder,
			includesJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);

		BuiltinResourcesHelper::UpdateManifest(
			shaderFolder,
			shadersJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);

		Vector<bool> includeImportFlags = BuiltinResourcesHelper::GenerateImportFlags(
			includesJSON,
			rawShaderIncludeFolder,
			lastUpdateTime,
			forceImport);

		Vector<bool> shaderImportFlags = BuiltinResourcesHelper::GenerateImportFlags(
			shadersJSON,
			rawShaderFolder,
			lastUpdateTime,
			forceImport,
			&shaderDependenciesJSON,
			rawShaderIncludeFolder);

		// Hidden dependency: Includes must be imported before shaders, but import flags for shaders must be generated
		// before includes are imported, since the process checks if imports changed
		BuiltinResourcesHelper::ImportAssets(
			includesJSON,
			includeImportFlags,
			rawShaderIncludeFolder,
			shaderIncludeFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal,
			nullptr,
			true);

		BuiltinResourcesHelper::ImportAssets(
			shadersJSON,
			shaderImportFlags,
			rawShaderFolder,
			shaderFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal,
			&shaderDependenciesJSON,
			true);
	}

	// Import GUI sprites
	if(!skinJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			skinFolder,
			skinJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);

		Vector<bool> skinImportFlags = BuiltinResourcesHelper::GenerateImportFlags(
			skinJSON,
			rawSkinFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			skinJSON,
			skinImportFlags,
			rawSkinFolder,
			skinFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);
	}

	// Import animated sprites
	if(!animatedSpritesJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			animatedSpriteFolder,
			animatedSpritesJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);

		Vector<bool> importFlags = BuiltinResourcesHelper::GenerateImportFlags(
			animatedSpritesJSON,
			rawAnimatedSpritesFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			animatedSpritesJSON,
			importFlags,
			rawAnimatedSpritesFolder,
			animatedSpriteFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Sprite);
	}

	// Import textures
	if(!texturesJSON.is_null())
	{
		BuiltinResourcesHelper::UpdateManifest(
			texturesFolder,
			texturesJSON,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal);

		Vector<bool> importFlags = BuiltinResourcesHelper::GenerateImportFlags(
			texturesJSON,
			rawTexturesFolder,
			lastUpdateTime,
			forceImport);

		BuiltinResourcesHelper::ImportAssets(
			texturesJSON,
			importFlags,
			rawTexturesFolder,
			texturesFolder,
			sManifest,
			BuiltinResourcesHelper::AssetType::Normal,
			nullptr, false, true);
	}

	// Update shader dependencies JSON
	{
		String jsonString = shaderDependenciesJSON.dump(4).c_str();

		dataListStream = FileSystem::CreateAndOpenFile(shaderDependenciesFile);
		dataListStream->WriteString(jsonString);
		dataListStream->Close();
	}

	// Import fonts
	if(!fontsJSON.is_null())
	{
		for(auto& entry : fontsJSON)
		{
			std::string path = entry["Path"];
			std::string name = entry["Name"];
			std::string uuidStr = entry["UUID"];
			const bool antialiasing = entry["Antialiasing"];

			json fontSizesJSON = entry["Sizes"];
			Vector<u32> fontSizes;
			for(auto& sizeEntry : fontSizesJSON)
				fontSizes.push_back(sizeEntry);

			String inputName(path.data(), path.size());
			String outputName(name.data(), name.size());
			UUID UUID(String(uuidStr.data(), uuidStr.size()));

			const Path fontSourcePath = sInputFolder + inputName;

			BuiltinResourcesHelper::ImportFont(fontSourcePath, outputName, sOutputFolder, fontSizes, antialiasing, UUID, sManifest);
		}
	}

	// Generate & save GUI skin
	if(!guiSkinJSON.is_null())
	{
		std::string name = guiSkinJSON["Path"];
		std::string uuidStr = guiSkinJSON["UUID"];

		String fileName(name.data(), name.size());
		UUID UUID(String(uuidStr.data(), uuidStr.size()));

		const SPtr<GUISkin> skin = GenerateGuiSkin();
		const Path outputPath = sOutputFolder + (fileName + u8".asset");

		HResource skinResource = GetResources().CreateResourceHandleInternal(skin, UUID);

		GetResources().Save(skinResource, outputPath, true);
		sManifest->RegisterResource(skinResource.GetUuid(), outputPath);
	}

	// Generate & save splash screen
	if(!splashScreenJSON.is_null())
	{
		std::string name = splashScreenJSON["Path"];
		String fileName(name.data(), name.size());

		Path inputPath = sInputFolder + fileName;
		Path outputPath = sOutputFolder + (fileName + ".asset");

		auto textureIO = GetImporter().CreateImportOptions<TextureImportOptions>(inputPath);
		textureIO->CpuCached = true;
		textureIO->GenerateMips = false;
		HTexture splashTexture = GetImporter().Import<Texture>(inputPath, textureIO);

		SPtr<PixelData> splashPixelData = splashTexture->GetProperties().AllocBuffer(0, 0);
		splashTexture->ReadCachedData(*splashPixelData);

		FileEncoder fe(outputPath);
		fe.Encode(splashPixelData.get());
	}
}
