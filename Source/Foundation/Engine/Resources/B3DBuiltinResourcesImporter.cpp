#include "B3DApplication.h"
#include "B3DEngineConfig.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DBuiltinResources.h"
#include "B3DBuiltinResourcesHelper.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DPath.h"
#include "FileSystem/B3DDataStream.h"
#include "ThirdParty/json.hpp"
#include "Utility/B3DShapeMeshes3D.h"
#include "Mesh/B3DMesh.h"
#include "Renderer/B3DRendererMeshData.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Serialization/B3DFileSerializer.h"
#include "Importer/B3DImporter.h"
#include "Importer/B3DTextureImportOptions.h"


#define B3DIMPORTTOOL_WAIT_FOR_DEBUGGER 0
#if B3DIMPORTTOOL_WAIT_FOR_DEBUGGER
#include <windows.h>
#endif

using namespace b3d;

static constexpr const char* kTimestampName = u8"Timestamp.asset";
static constexpr const char* kManifestName = u8"ResourceManifest.asset";
static constexpr const char* kDependenciesJsonName = u8"ShaderDependencies.json";
static constexpr const char* kDataListJson = u8"DataList.json";

static Path sInputFolder;
static Path sOutputFolder;
static Path sManifestPath;

void ProcessAssets(bool, bool, time_t);

int main(int argc, char* argv[])
{
	using namespace b3d;

#if B3DIMPORTTOOL_WAIT_FOR_DEBUGGER
	while (!::IsDebuggerPresent())
	{
		::Sleep(100);
	}
	::DebugBreak();
#endif

	if(argc < 3)
		return 2;

	ApplicationCreateInformation desc;
	desc.RenderApi = B3D_RENDER_BACKEND;
	desc.Renderer = B3D_RENDERER;
	desc.Audio = B3D_AUDIO_BACKEND;
	desc.Physics = B3D_PHYSICS_BACKEND;

	desc.Importers.push_back("bsfFreeImgImporter");
	desc.Importers.push_back("bsfFBXImporter");
	desc.Importers.push_back("bsfFontImporter");
	desc.Importers.push_back("bsfSL");

	desc.PrimaryWindow.VideoMode = VideoMode(64, 64);
	desc.PrimaryWindow.Fullscreen = false;
	desc.PrimaryWindow.Title = "bsf importer";
	desc.PrimaryWindow.Hidden = true;

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

			ProcessAssets(generateGenerated, fullReimport, lastUpdateTime);
			BuiltinResourcesHelper::WriteTimestamp(sOutputFolder + kTimestampName);

			Application::ShutDown();
			return 1;
		}
	}

	Application::ShutDown();
	return 0;
}

using namespace b3d;

void GenerateTextures()
{
	SPtr<PixelData> blackPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	blackPixelData->SetColorAt(Color::kBlack, 0, 0);
	blackPixelData->SetColorAt(Color::kBlack, 0, 1);
	blackPixelData->SetColorAt(Color::kBlack, 1, 0);
	blackPixelData->SetColorAt(Color::kBlack, 1, 1);

	SPtr<Texture> blackTexture = Texture::CreateShared(blackPixelData);

	SPtr<PixelData> whitePixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
	whitePixelData->SetColorAt(Color::kWhite, 0, 0);
	whitePixelData->SetColorAt(Color::kWhite, 0, 1);
	whitePixelData->SetColorAt(Color::kWhite, 1, 0);
	whitePixelData->SetColorAt(Color::kWhite, 1, 1);

	SPtr<Texture> whiteTexture = Texture::CreateShared(whitePixelData);

	SPtr<PixelData> normalPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

	Color encodedNormal(0.5f, 0.5f, 1.0f);
	normalPixelData->SetColorAt(encodedNormal, 0, 0);
	normalPixelData->SetColorAt(encodedNormal, 0, 1);
	normalPixelData->SetColorAt(encodedNormal, 1, 0);
	normalPixelData->SetColorAt(encodedNormal, 1, 1);

	SPtr<Texture> normalTexture = Texture::CreateShared(normalPixelData);

	SPtr<PixelData> black3DPixelData = PixelData::Create(2, 2, 2, PF_RGBA8);
	black3DPixelData->SetColorAt(Color::kBlack, 0, 0, 0);
	black3DPixelData->SetColorAt(Color::kBlack, 0, 1, 0);
	black3DPixelData->SetColorAt(Color::kBlack, 1, 0, 0);
	black3DPixelData->SetColorAt(Color::kBlack, 1, 1, 0);
	black3DPixelData->SetColorAt(Color::kBlack, 0, 0, 1);
	black3DPixelData->SetColorAt(Color::kBlack, 0, 1, 1);
	black3DPixelData->SetColorAt(Color::kBlack, 1, 0, 1);
	black3DPixelData->SetColorAt(Color::kBlack, 1, 1, 1);

	SPtr<Texture> black3DTexture = Texture::CreateShared(black3DPixelData);

	SPtr<PixelData> white3DPixelData = PixelData::Create(2, 2, 2, PF_RGBA8);
	white3DPixelData->SetColorAt(Color::kWhite, 0, 0, 0);
	white3DPixelData->SetColorAt(Color::kWhite, 0, 1, 0);
	white3DPixelData->SetColorAt(Color::kWhite, 1, 0, 0);
	white3DPixelData->SetColorAt(Color::kWhite, 1, 1, 0);
	white3DPixelData->SetColorAt(Color::kWhite, 0, 0, 1);
	white3DPixelData->SetColorAt(Color::kWhite, 0, 1, 1);
	white3DPixelData->SetColorAt(Color::kWhite, 1, 0, 1);
	white3DPixelData->SetColorAt(Color::kWhite, 1, 1, 1);

	SPtr<Texture> white3DTexture = Texture::CreateShared(white3DPixelData);


	// Save all textures
	Path outputDir = sOutputFolder + BuiltinResources::kTextureFolder;

	auto fnSaveTexture = [](const Path& folder, const String& name, const SPtr<Texture>& texture, const String& uuid)
	{
		HResource textureResource = GetResources().CreateResourceHandle(texture, b3d::UUID(uuid));
		GetResources().SaveAsSinglePackage(textureResource, folder, name);
	};

	fnSaveTexture(outputDir, BuiltinResources::kTextureWhiteFile, whiteTexture, "1f7d0e3f-d81b-42ee-9d31-cb6c6fc55824");
	fnSaveTexture(outputDir, BuiltinResources::kTextureBlackFile, blackTexture, "149a5c05-9570-4915-9dbd-69acf88b865b");
	fnSaveTexture(outputDir, BuiltinResources::kTextureNormalFile, normalTexture, "afb29163-1ef0-4440-9cfb-c1ebb3b3d452");
	fnSaveTexture(outputDir, BuiltinResources::kTextureWhite3DFile, white3DTexture, "dba4e37b-7ab3-407c-af71-7943cc262129");
	fnSaveTexture(outputDir, BuiltinResources::kTextureBlack3DFile, black3DTexture, "36c8e112-0bb9-4c22-894e-d4ad160e498e");
}

void GenerateMeshes()
{
	TInlineArray<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_NORMAL));
	vertexElements.Add(VertexElement(VET_FLOAT4, VES_TANGENT));
	vertexElements.Add(VertexElement(VET_COLOR, VES_COLOR));

	SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

	u32 boxNumVertices = 0;
	u32 boxNumIndices = 0;
	ShapeMeshes3D::GetNumElementsAaBox(boxNumVertices, boxNumIndices);
	SPtr<MeshData> boxMeshData = MeshData::Create(boxNumVertices, boxNumIndices, vertexDescription);
	AABox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

	ShapeMeshes3D::SolidAaBox(box, boxMeshData, 0, 0);
	SPtr<Mesh> boxMesh = Mesh::CreateShared(RendererMeshData::Convert(boxMeshData));

	u32 sphereNumVertices = 0;
	u32 sphereNumIndices = 0;
	ShapeMeshes3D::GetNumElementsSphere(3, sphereNumVertices, sphereNumIndices);
	SPtr<MeshData> sphereMeshData = B3DMakeShared<MeshData>(sphereNumVertices, sphereNumIndices, vertexDescription);

	ShapeMeshes3D::SolidSphere(Sphere(Vector3::kZero, 1.0f), sphereMeshData, 0, 0, 3);
	SPtr<Mesh> sphereMesh = Mesh::CreateShared(RendererMeshData::Convert(sphereMeshData));

	u32 coneNumVertices = 0;
	u32 coneNumIndices = 0;
	ShapeMeshes3D::GetNumElementsCone(10, coneNumVertices, coneNumIndices);
	SPtr<MeshData> coneMeshData = B3DMakeShared<MeshData>(coneNumVertices, coneNumIndices, vertexDescription);

	ShapeMeshes3D::SolidCone(Vector3::kZero, Vector3::kUnitY, 1.0f, 1.0f, Vector2::kOne, coneMeshData, 0, 0);
	SPtr<Mesh> coneMesh = Mesh::CreateShared(RendererMeshData::Convert(coneMeshData));

	u32 cylinderNumVertices = 0;
	u32 cylinderNumIndices = 0;
	ShapeMeshes3D::GetNumElementsCylinder(10, cylinderNumVertices, cylinderNumIndices);
	SPtr<MeshData> cylinderMeshData = B3DMakeShared<MeshData>(cylinderNumVertices, cylinderNumIndices, vertexDescription);

	ShapeMeshes3D::SolidCylinder(Vector3::kZero, Vector3::kUnitY, 1.0f, 1.0f, Vector2::kOne, cylinderMeshData, 0, 0);
	SPtr<Mesh> cylinderMesh = Mesh::CreateShared(RendererMeshData::Convert(cylinderMeshData));

	u32 quadNumVertices = 8;
	u32 quadNumIndices = 12;
	ShapeMeshes3D::GetNumElementsQuad(quadNumVertices, quadNumIndices);
	SPtr<MeshData> quadMeshData = B3DMakeShared<MeshData>(quadNumVertices, quadNumIndices, vertexDescription);

	std::array<Vector3, 2> axes = { { Vector3::kUnitX, Vector3::kUnitZ } };
	std::array<float, 2> sizes = { { 1.0f, 1.0f } };
	Rect3 rect(Vector3::kZero, axes, sizes);
	ShapeMeshes3D::SolidQuad(rect, quadMeshData, 0, 0);
	SPtr<Mesh> quadMesh = Mesh::CreateShared(RendererMeshData::Convert(quadMeshData));

	u32 discNumVertices = 0;
	u32 discNumIndices = 0;
	ShapeMeshes3D::GetNumElementsDisc(10, discNumVertices, discNumIndices);
	SPtr<MeshData> discMeshData = B3DMakeShared<MeshData>(discNumVertices, discNumIndices, vertexDescription);

	ShapeMeshes3D::SolidDisc(Vector3::kZero, 1.0f, Vector3::kUnitY, discMeshData, 0, 0);
	SPtr<Mesh> discMesh = Mesh::CreateShared(RendererMeshData::Convert(discMeshData));

	// Save all meshes
	const Path outputDir = sOutputFolder + BuiltinResources::kMeshFolder;

	auto fnSaveMesh = [](const Path& folder, const String& name, const SPtr<Mesh>& mesh, const String& uuid)
	{
		HResource meshResource = GetResources().CreateResourceHandle(mesh, b3d::UUID(uuid));
		GetResources().SaveAsSinglePackage(meshResource, folder, name);
	};

	fnSaveMesh(outputDir, BuiltinResources::kMeshBoxFile, boxMesh, "bc1d20ca-7fe6-489b-8b5c-dbf798badc95");
	fnSaveMesh(outputDir, BuiltinResources::kMeshSphereFile, sphereMesh, "040642f3-04d6-419e-9dba-f7824161c205");
	fnSaveMesh(outputDir, BuiltinResources::kMeshConeFile, coneMesh, "b8cf6db5-1736-47ac-852f-82ecd88b4d46");
	fnSaveMesh(outputDir, BuiltinResources::kMeshCylinderFile, cylinderMesh, "e6b2b797-4e72-7e49-61ba-4e7275bd561d");
	fnSaveMesh(outputDir, BuiltinResources::kMeshQuadFile, quadMesh, "06592bf3-f82a-472e-a034-26a98225fbe1");
	fnSaveMesh(outputDir, BuiltinResources::kMeshDiscFile, discMesh, "6f496313-344a-495c-83e8-152e3053c52d");
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
	json fontsJSON = dataListJSON["Fonts"];
	json splashScreenJSON = dataListJSON["SplashScreen"];
	json texturesJSON = dataListJSON["Textures"];

	const Path rawSkinFolder = sInputFolder + BuiltinResources::kSkinFolder;
	const Path rawAnimatedSpritesFolder = sInputFolder + BuiltinResources::kAnimatedSpritesFolder;
	const Path rawCursorFolder = sInputFolder + BuiltinResources::kCursorFolder;
	const Path rawIconFolder = sInputFolder + BuiltinResources::kIconFolder;
	const Path rawIcon3DFolder = sInputFolder + BuiltinResources::kIcoN3DFolder;
	const Path rawShaderIncludeFolder = sInputFolder + BuiltinResources::kShaderIncludeFolder;
	const Path rawTexturesFolder = sInputFolder + BuiltinResources::kTextureFolder;
	const Path rawFontsFolder = sInputFolder + BuiltinResources::kFontsFolder;

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

	if(!fontsJSON.is_null())
	{
		updatedDataLists |= BuiltinResourcesHelper::UpdateJson(
			rawFontsFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			fontsJSON);
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

		if(!fontsJSON.is_null())
			dataListJSON["Fonts"] = fontsJSON;

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
	const Path texturesFolder = sOutputFolder + BuiltinResources::kTextureFolder;
	const Path fontsFolder = sOutputFolder + BuiltinResources::kFontsFolder;
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

		if(FileSystem::Exists(skinFolder))
			FileSystem::Remove(skinFolder);

		if(FileSystem::Exists(animatedSpriteFolder))
			FileSystem::Remove(animatedSpriteFolder);

		if(FileSystem::Exists(fontsFolder))
			FileSystem::Remove(fontsFolder);

		FileSystem::Remove(shaderDependenciesFile);
	}

	// Import cursors
	if(!cursorsJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Normal);
	}

	// Import icons
	if(!iconsJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Normal);
	}

	// Import sprite icons
	if(!spriteIconsJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Sprite);
	}

	// Import 3D sprite icons
	if(!spriteIcons3DJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Sprite,
			false, true);
	}

	// Import GUI sprites
	if(!skinJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Sprite);
	}

	// Import animated sprites
	if(!animatedSpritesJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Sprite);
	}

	// Import textures
	if(!texturesJSON.is_null())
	{
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
			BuiltinResourcesHelper::AssetType::Normal,
			false, true);
	}

	// Import fonts
	if(!fontsJSON.is_null())
	{
		if(FileSystem::Exists(rawFontsFolder))
		{
			bool outputExists = FileSystem::Exists(fontsFolder);
			if(!outputExists)
				FileSystem::CreateDir(fontsFolder);

			for(auto& entry : fontsJSON)
			{
				std::string path = entry["Path"];
				std::string uuidStr = entry["UUID"];

				const bool antialiasing = entry.count("Antialiasing") > 0 ? entry["Antialiasing"] : true;

				json fontSizesJSON = entry["Sizes"];
				Vector<float> fontSizes;

				if(fontSizesJSON.is_array())
				{
					for(auto& sizeEntry : fontSizesJSON)
						fontSizes.push_back(sizeEntry);
				}

				String inputName(path.data(), path.size());
				b3d::UUID UUID(String(uuidStr.data(), uuidStr.size()));

				const Path fontSourcePath = rawFontsFolder + inputName;

				BuiltinResourcesHelper::ImportFont(fontSourcePath, fontSourcePath.GetFilename(), fontsFolder, fontSizes, antialiasing, UUID);
			}
		}
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
