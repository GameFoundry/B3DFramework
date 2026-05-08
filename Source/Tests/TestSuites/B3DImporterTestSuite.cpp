//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DImporterTestSuite.h"

#include "Importer/B3DImporter.h"
#include "Importer/B3DTextureImportOptions.h"
#include "Audio/B3DAudioClipImportOptions.h"
#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "Audio/B3DAudioClip.h"
#include "Resources/B3DBuiltinResources.h"

using namespace b3d;

ImporterTestSuite::ImporterTestSuite()
	: TestSuite("ImporterTestSuite")
{
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_Default)
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_WithMips)
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_NvttBC3)
	B3D_ADD_TEST(ImporterTestSuite::TestOggImport_Default)
	B3D_ADD_TEST(ImporterTestSuite::TestOggImport_KeepCompressed)
	B3D_ADD_TEST(ImporterTestSuite::TestFlacImport_Default)
}

void ImporterTestSuite::StartUp()
{
	const Path dataFolder = Path::Combine(BuiltinResources::GetUnitTestDataFolder(), "ImporterTests");
	mImagePath = Path::Combine(dataFolder, "TestImage.png");
	mOggPath = Path::Combine(dataFolder, "TestAudio.ogg");
	mFlacPath = Path::Combine(dataFolder, "TestAudio.flac");
}

void ImporterTestSuite::TestPngImport_Default()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& props = texture->GetProperties();
	B3D_TEST_ASSERT(props.Format == PF_RGBA8)
	B3D_TEST_ASSERT(props.Width == 64)
	B3D_TEST_ASSERT(props.Height == 64)
	B3D_TEST_ASSERT(props.MipMapCount == 0)
}

void ImporterTestSuite::TestPngImport_WithMips()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	options->GenerateMips = true;

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& props = texture->GetProperties();
	B3D_TEST_ASSERT(props.Format == PF_RGBA8)
	B3D_TEST_ASSERT(props.Width == 64)
	B3D_TEST_ASSERT(props.Height == 64)
	B3D_TEST_ASSERT(props.MipMapCount == 6)
}

void ImporterTestSuite::TestPngImport_NvttBC3()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	options->GenerateMips = true;
	options->Format = PF_BC3;

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& props = texture->GetProperties();
	B3D_TEST_ASSERT(props.Format == PF_BC3)
	B3D_TEST_ASSERT(props.Width == 64)
	B3D_TEST_ASSERT(props.Height == 64)
	B3D_TEST_ASSERT(props.MipMapCount == 6)
}

void ImporterTestSuite::TestOggImport_Default()
{
	TShared<AudioClipImportOptions> options = AudioClipImportOptions::Create();
	options->Is3D = false;

	HAudioClip clip = GetImporter().Import<AudioClip>(mOggPath, options);
	B3D_TEST_ASSERT(clip != nullptr)
	B3D_TEST_ASSERT(clip.IsLoaded())

	B3D_TEST_ASSERT(clip->GetFormat() == AudioFormat::PCM)
	B3D_TEST_ASSERT(clip->GetReadMode() == AudioReadMode::LoadDecompressed)
	B3D_TEST_ASSERT(clip->GetFrequency() > 0)
	B3D_TEST_ASSERT(clip->GetChannelCount() >= 1 && clip->GetChannelCount() <= 2)
	B3D_TEST_ASSERT(clip->GetBitDepth() == 16)
	B3D_TEST_ASSERT(clip->GetSampleCount() > 0)
	B3D_TEST_ASSERT(clip->GetLength() > 0.0f)
}

void ImporterTestSuite::TestOggImport_KeepCompressed()
{
	TShared<AudioClipImportOptions> options = AudioClipImportOptions::Create();
	options->Is3D = false;
	options->Format = AudioFormat::VORBIS;
	options->ReadMode = AudioReadMode::LoadCompressed;

	HAudioClip clip = GetImporter().Import<AudioClip>(mOggPath, options);
	B3D_TEST_ASSERT(clip != nullptr)
	B3D_TEST_ASSERT(clip.IsLoaded())

	B3D_TEST_ASSERT(clip->GetFormat() == AudioFormat::VORBIS)
	B3D_TEST_ASSERT(clip->GetReadMode() == AudioReadMode::LoadCompressed)
	B3D_TEST_ASSERT(clip->GetFrequency() > 0)
	B3D_TEST_ASSERT(clip->GetChannelCount() >= 1 && clip->GetChannelCount() <= 2)
	B3D_TEST_ASSERT(clip->GetSampleCount() > 0)
	B3D_TEST_ASSERT(clip->GetLength() > 0.0f)
}

void ImporterTestSuite::TestFlacImport_Default()
{
	TShared<AudioClipImportOptions> options = AudioClipImportOptions::Create();
	options->Is3D = false;

	HAudioClip clip = GetImporter().Import<AudioClip>(mFlacPath, options);
	B3D_TEST_ASSERT(clip != nullptr)
	B3D_TEST_ASSERT(clip.IsLoaded())

	// Fixture metadata: stereo, 44.1 kHz, 16-bit, 16870 frames.
	B3D_TEST_ASSERT(clip->GetFormat() == AudioFormat::PCM)
	B3D_TEST_ASSERT(clip->GetFrequency() == 44100)
	B3D_TEST_ASSERT(clip->GetChannelCount() == 2)
	B3D_TEST_ASSERT(clip->GetBitDepth() == 16)
	B3D_TEST_ASSERT(clip->GetSampleCount() > 0)
	B3D_TEST_ASSERT(clip->GetLength() > 0.0f)
}
