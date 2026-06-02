//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DImporterTestSuite.h"

#include "Importer/B3DImporter.h"
#include "Importer/B3DTextureImportOptions.h"
#include "Audio/B3DAudioClipImportOptions.h"
#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "Image/B3DTextureCompressor.h"
#include "Image/B3DGenerateMipmap.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DColor.h"
#include "Audio/B3DAudioClip.h"
#include "Resources/B3DBuiltinResources.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "B3DApplication.h"

#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace b3d;

namespace
{
	double PsnrFromError(double squaredError, u64 samples)
	{
		const double mse = squaredError / (double)samples;
		if(mse <= 0.0)
			return 99.0;
		return 10.0 * std::log10(255.0 * 255.0 / mse);
	}

	// ---- Optional debug image dump. The compression tests are otherwise purely numeric (PSNR); set the env var
	// B3D_DUMP_COMPRESS_IMAGES=1 to also write side-by-side [source | decoded] PNGs to the working directory. ----

	bool DumpCompressImagesEnabled()
	{
		const char* v = std::getenv("B3D_DUMP_COMPRESS_IMAGES");
		return v != nullptr && v[0] != '\0' && !(v[0] == '0' && v[1] == '\0');
	}

	// Writes a [source | gap | decoded] comparison PNG. Both inputs are tightly-packed RGBA8 buffers (width*height*4,
	// top-left origin) already mapped to a displayable interpretation of the format. @p name yields "<name>.png".
	void WriteSideBySidePNG(const String& name, u32 width, u32 height, const Vector<u8>& srcRGBA, const Vector<u8>& decRGBA)
	{
		const u32 gap = 4;
		const u32 compositeWidth = width * 2 + gap;
		const TShared<PixelData> img = PixelData::Create(compositeWidth, height, 1, PF_RGBA8);
		u8* const dst = img->GetData();
		const u32 rowPitch = img->GetRowPitch();

		for(u32 y = 0; y < height; ++y)
		{
			u8* const row = dst + y * rowPitch;
			for(u32 x = 0; x < width; ++x)
			{
				const u8* const s = &srcRGBA[(y * width + x) * 4];
				const u8* const d = &decRGBA[(y * width + x) * 4];
				u8* const lp = row + x * 4;                       // left panel: source
				u8* const rp = row + (width + gap + x) * 4;       // right panel: decoded
				lp[0] = s[0]; lp[1] = s[1]; lp[2] = s[2]; lp[3] = 255;
				rp[0] = d[0]; rp[1] = d[1]; rp[2] = d[2]; rp[3] = 255;
			}
			// Magenta separator so the seam is obvious.
			for(u32 g = 0; g < gap; ++g)
			{
				u8* const sep = row + (width + g) * 4;
				sep[0] = 255; sep[1] = 0; sep[2] = 255; sep[3] = 255;
			}
		}

		const Path outPath = Path(String("compress_") + name);
		if(PixelUtility::SaveImage(img, outPath, ImageFormat::PNG, true))
			B3D_LOG(Info, LogPixelUtility, "Wrote compression comparison image '{0}.png'", name);
	}

	// Writes a [source | gap | decoded] comparison OpenEXR. Both inputs are tightly-packed RGB float buffers
	// (width*height*3, top-left origin) holding raw linear HDR values. @p name yields "<name>.exr". Used for BC6H so the
	// full dynamic range is preserved (PNG can't hold HDR), rather than tone-mapping down to 8-bit.
	void WriteSideBySideEXR(const String& name, u32 width, u32 height, const Vector<float>& srcRGB, const Vector<float>& decRGB)
	{
		const u32 gap = 4;
		const u32 compositeWidth = width * 2 + gap;
		const TShared<PixelData> img = PixelData::Create(compositeWidth, height, 1, PF_RGB32F);

		for(u32 y = 0; y < height; ++y)
		{
			for(u32 x = 0; x < width; ++x)
			{
				const float* const s = &srcRGB[(y * width + x) * 3];
				const float* const d = &decRGB[(y * width + x) * 3];
				img->SetColorAt(Color(s[0], s[1], s[2], 1.0f), x, y);                 // left panel: source
				img->SetColorAt(Color(d[0], d[1], d[2], 1.0f), width + gap + x, y);   // right panel: decoded
			}
			// Magenta separator so the seam is obvious.
			for(u32 g = 0; g < gap; ++g)
				img->SetColorAt(Color(1.0f, 0.0f, 1.0f, 1.0f), width + g, y);
		}

		const Path outPath = Path(String("compress_") + name);
		if(PixelUtility::SaveImage(img, outPath, ImageFormat::EXR))
			B3D_LOG(Info, LogPixelUtility, "Wrote compression comparison image '{0}.exr'", name);
	}
} // anonymous namespace

ImporterTestSuite::ImporterTestSuite()
	: TestSuite("ImporterTestSuite")
{
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_Default)
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_WithMips)
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_BC3)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuCompress_Psnr)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuCompress_BC6H_Psnr)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuMipmaps_BoxFilter)
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

void ImporterTestSuite::TestPngImport_BC3()
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

void ImporterTestSuite::TestGpuCompress_Psnr()
{
	// GPU compression needs a real backend; skip on a headless / Null backend so the suite stays
	// green on machines without a usable GPU.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Load the source image as RGBA8 and read it back to the CPU.
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	TAsyncOp<TShared<PixelData>> readOp = texture->ReadData(0, 0);
	readOp.BlockUntilComplete();
	TShared<PixelData> source = readOp.GetReturnValue();
	B3D_TEST_ASSERT(source != nullptr)

	const u32 width = source->GetWidth();
	const u32 height = source->GetHeight();
	const u32 blocksX = width / 4;
	const u32 blocksY = height / 4;

	// Cache source pixels as RGBA8 for comparison.
	Vector<u8> src(width * height * 4);
	for(u32 y = 0; y < height; ++y)
		for(u32 x = 0; x < width; ++x)
		{
			const Color c = source->GetColorAt(x, y);
			u8* p = &src[(y * width + x) * 4];
			p[0] = (u8)Math::Clamp((i32)std::lround(c.R * 255.0f), 0, 255);
			p[1] = (u8)Math::Clamp((i32)std::lround(c.G * 255.0f), 0, 255);
			p[2] = (u8)Math::Clamp((i32)std::lround(c.B * 255.0f), 0, 255);
			p[3] = (u8)Math::Clamp((i32)std::lround(c.A * 255.0f), 0, 255);
		}

	struct Case { PixelFormat Format; const char* Name; u32 BlockBytes; };
	const Case cases[] = {
		{ PF_BC1, "BC1", 8 },
		{ PF_BC3, "BC3", 16 },
		{ PF_BC4, "BC4", 8 },
		{ PF_BC5, "BC5", 16 },
		{ PF_BC7, "BC7", 16 },
	};

	const bool dump = DumpCompressImagesEnabled();

	for(const Case& test : cases)
	{
		TShared<PixelData> compressed = PixelData::Create(width, height, 1, test.Format);
		CompressionOptions co;
		co.Format = test.Format;
		// Call the GPU compressor directly: PixelUtility::Compress routes through nvtt while B3D_USE_NVTT is on (the
		// default), so the test must target the GPU path explicitly to keep exercising it until the switch is complete.
		const bool compressOk = GpuTextureCompressor::Compress(*source, *compressed, co);
		B3D_TEST_ASSERT(compressOk)

		const u8* blocks = compressed->GetData();
		double error = 0.0;
		u64 samples = 0;

		// Display-mapped source/decoded surfaces, filled only when dumping comparison images.
		Vector<u8> srcDisp, decDisp;
		if(dump) { srcDisp.resize(width * height * 4); decDisp.resize(width * height * 4); }

		for(u32 by = 0; by < blocksY; ++by)
		{
			for(u32 bx = 0; bx < blocksX; ++bx)
			{
				const u8* block = blocks + (by * blocksX + bx) * test.BlockBytes;

				u8 decRGBA[64];
				u8 decR[16];
				u8 decG[16];
				if(test.Format == PF_BC1)
					TextureCompressionUtility::DecodeBC1(block, false, decRGBA);
				else if(test.Format == PF_BC3)
				{
					TextureCompressionUtility::DecodeBC4(block, decR);            // alpha block
					TextureCompressionUtility::DecodeBC1(block + 8, true, decRGBA); // color block (always 4-colour)
				}
				else if(test.Format == PF_BC4)
					TextureCompressionUtility::DecodeBC4(block, decR);
				else if(test.Format == PF_BC7)
					TextureCompressionUtility::DecodeBC7(block, decRGBA);
				else // BC5
				{
					TextureCompressionUtility::DecodeBC4(block, decR);
					TextureCompressionUtility::DecodeBC4(block + 8, decG);
				}

				for(u32 i = 0; i < 16; ++i)
				{
					const u32 px = bx * 4 + (i % 4);
					const u32 py = by * 4 + (i / 4);
					const u8* s = &src[(py * width + px) * 4];

					if(test.Format == PF_BC1)
					{
						for(i32 c = 0; c < 3; ++c) { double d = (double)s[c] - decRGBA[i * 4 + c]; error += d * d; }
						samples += 3;
					}
					else if(test.Format == PF_BC3)
					{
						for(i32 c = 0; c < 3; ++c) { double d = (double)s[c] - decRGBA[i * 4 + c]; error += d * d; }
						double da = (double)s[3] - decR[i]; error += da * da; // alpha from BC4 block
						samples += 4;
					}
					else if(test.Format == PF_BC4)
					{
						double d = (double)s[0] - decR[i]; error += d * d; samples += 1;
					}
					else if(test.Format == PF_BC7)
					{
						for(i32 c = 0; c < 4; ++c) { double d = (double)s[c] - decRGBA[i * 4 + c]; error += d * d; }
						samples += 4;
					}
					else // BC5
					{
						double dr = (double)s[0] - decR[i]; double dg = (double)s[1] - decG[i];
						error += dr * dr + dg * dg; samples += 2;
					}

					if(dump)
					{
						// Map both source and decoded through the same channel interpretation for a fair comparison.
						u8 sr, sg, sb, dr, dg, db;
						if(test.Format == PF_BC4)      { sr = sg = sb = s[0]; dr = dg = db = decR[i]; }
						else if(test.Format == PF_BC5) { sr = s[0]; sg = s[1]; sb = 0; dr = decR[i]; dg = decG[i]; db = 0; }
						else                           { sr = s[0]; sg = s[1]; sb = s[2]; dr = decRGBA[i * 4 + 0]; dg = decRGBA[i * 4 + 1]; db = decRGBA[i * 4 + 2]; }
						u8* const sp = &srcDisp[(py * width + px) * 4]; sp[0] = sr; sp[1] = sg; sp[2] = sb; sp[3] = 255;
						u8* const dp = &decDisp[(py * width + px) * 4]; dp[0] = dr; dp[1] = dg; dp[2] = db; dp[3] = 255;
					}
				}
			}
		}

		const double psnr = PsnrFromError(error, samples);
		B3D_LOG(Info, LogPixelUtility, "GPU compression {0}: PSNR = {1} dB", test.Name, psnr);

		if(dump)
			WriteSideBySidePNG(test.Name, width, height, srcDisp, decDisp);

		// Sanity floor: a working encoder is far above this; random/garbage blocks would be ~8-10 dB.
		B3D_TEST_ASSERT(psnr > 15.0)
	}
}

void ImporterTestSuite::TestGpuCompress_BC6H_Psnr()
{
	// GPU compression needs a real backend; skip on a headless / Null backend.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Synthesize a smooth HDR gradient (values 0..8, beyond the LDR range) so BC6H has real HDR to encode.
	const u32 size = 64;
	TShared<PixelData> source = PixelData::Create(size, size, 1, PF_RGBA32F);
	for(u32 y = 0; y < size; ++y)
		for(u32 x = 0; x < size; ++x)
		{
			Color c;
			c.R = (float)x / (float)(size - 1) * 4.0f;
			c.G = (float)y / (float)(size - 1) * 6.0f;
			c.B = (float)(x + y) / (float)(2 * (size - 1)) * 8.0f;
			c.A = 1.0f;
			source->SetColorAt(c, x, y);
		}

	TShared<PixelData> compressed = PixelData::Create(size, size, 1, PF_BC6H);
	CompressionOptions co;
	co.Format = PF_BC6H;
	// Call the GPU compressor directly (see note in TestGpuCompress_Psnr): PixelUtility::Compress uses nvtt by default.
	const bool compressOk = GpuTextureCompressor::Compress(*source, *compressed, co);
	B3D_TEST_ASSERT(compressOk)

	const u32 blocksX = size / 4, blocksY = size / 4;
	const u8* blocks = compressed->GetData();
	double error = 0.0, peak = 0.0;
	u64 samples = 0;

	// Raw HDR display surfaces (RGB float), filled only when dumping a comparison image; written out as OpenEXR.
	const bool dump = DumpCompressImagesEnabled();
	Vector<float> srcDisp, decDisp;
	if(dump) { srcDisp.resize(size * size * 3); decDisp.resize(size * size * 3); }

	for(u32 by = 0; by < blocksY; ++by)
		for(u32 bx = 0; bx < blocksX; ++bx)
		{
			float dec[16][3];
			TextureCompressionUtility::DecodeBC6H_UF16(blocks + (by * blocksX + bx) * 16, dec);

			for(u32 i = 0; i < 16; ++i)
			{
				const u32 px = bx * 4 + (i % 4);
				const u32 py = by * 4 + (i / 4);
				const Color s = source->GetColorAt(px, py);
				const float sc[3] = { s.R, s.G, s.B };
				for(i32 ch = 0; ch < 3; ++ch)
				{
					const double d = (double)sc[ch] - (double)dec[i][ch];
					error += d * d;
					peak = std::max(peak, (double)sc[ch]);
					++samples;
				}

				if(dump)
				{
					float* const sp = &srcDisp[(py * size + px) * 3];
					sp[0] = sc[0]; sp[1] = sc[1]; sp[2] = sc[2];
					float* const dp = &decDisp[(py * size + px) * 3];
					dp[0] = dec[i][0]; dp[1] = dec[i][1]; dp[2] = dec[i][2];
				}
			}
		}

	const double mse = error / (double)samples;
	const double psnr = (mse <= 0.0) ? 99.0 : 10.0 * std::log10(peak * peak / mse);
	B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: PSNR = {0} dB (peak {1})", psnr, peak);

	if(dump)
		WriteSideBySideEXR("BC6H", size, size, srcDisp, decDisp);

	// A working one-region BC6H encoder on a smooth gradient is far above this floor.
	B3D_TEST_ASSERT(psnr > 30.0)
}

void ImporterTestSuite::TestGpuMipmaps_BoxFilter()
{
	// GPU mip generation needs a real backend; skip on a headless / Null backend.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Build a deterministic 4x4 RGBA8 source so the expected box averages are known.
	const u32 size = 4;
	TShared<PixelData> source = PixelData::Create(size, size, 1, PF_RGBA8);
	for(u32 y = 0; y < size; ++y)
	{
		for(u32 x = 0; x < size; ++x)
		{
			Color c;
			c.R = (float)((x * 37 + y * 11) % 256) / 255.0f;
			c.G = (float)((x * 17 + y * 53) % 256) / 255.0f;
			c.B = (float)((x * 7 + y * 101) % 256) / 255.0f;
			c.A = (float)((x * 23 + y * 5) % 256) / 255.0f;
			source->SetColorAt(c, x, y);
		}
	}

	MipMapGenOptions options; // Box filter, linear (no sRGB), no normalization.
	// Call the GPU generator directly: PixelUtility::GenerateMipmaps routes through nvtt while B3D_USE_NVTT is on (the
	// default), so the test must target the GPU path explicitly to keep exercising it until the switch is complete.
	Vector<TShared<PixelData>> mips;
	const bool mipsOk = GpuGenerateMipmap::Generate(source, options, mips);
	B3D_TEST_ASSERT(mipsOk)

	// 4x4 produces a full chain: 4x4, 2x2, 1x1.
	B3D_TEST_ASSERT(mips.size() == 3)
	B3D_TEST_ASSERT(mips[0]->GetWidth() == 4 && mips[0]->GetHeight() == 4)
	B3D_TEST_ASSERT(mips[1]->GetWidth() == 2 && mips[1]->GetHeight() == 2)
	B3D_TEST_ASSERT(mips[2]->GetWidth() == 1 && mips[2]->GetHeight() == 1)

	// 8-bit round-trip plus a single average leaves at most ~1.5/255 of error per channel.
	const float tolerance = 3.0f / 255.0f;

	// Mip 0 is the unfiltered source.
	for(u32 y = 0; y < size; ++y)
	{
		for(u32 x = 0; x < size; ++x)
		{
			const Color s = source->GetColorAt(x, y);
			const Color m = mips[0]->GetColorAt(x, y);
			B3D_TEST_ASSERT(std::fabs(s.R - m.R) < tolerance)
			B3D_TEST_ASSERT(std::fabs(s.G - m.G) < tolerance)
			B3D_TEST_ASSERT(std::fabs(s.B - m.B) < tolerance)
			B3D_TEST_ASSERT(std::fabs(s.A - m.A) < tolerance)
		}
	}

	// Mip 1: each texel is the box average of the corresponding 2x2 source block.
	for(u32 y = 0; y < 2; ++y)
	{
		for(u32 x = 0; x < 2; ++x)
		{
			float r = 0, g = 0, b = 0, a = 0;
			for(u32 dy = 0; dy < 2; ++dy)
			{
				for(u32 dx = 0; dx < 2; ++dx)
				{
					const Color s = source->GetColorAt(x * 2 + dx, y * 2 + dy);
					r += s.R; g += s.G; b += s.B; a += s.A;
				}
			}
			r *= 0.25f; g *= 0.25f; b *= 0.25f; a *= 0.25f;

			const Color m = mips[1]->GetColorAt(x, y);
			B3D_TEST_ASSERT(std::fabs(r - m.R) < tolerance)
			B3D_TEST_ASSERT(std::fabs(g - m.G) < tolerance)
			B3D_TEST_ASSERT(std::fabs(b - m.B) < tolerance)
			B3D_TEST_ASSERT(std::fabs(a - m.A) < tolerance)
		}
	}

	// Mip 2: the average of all 16 source texels.
	{
		float r = 0, g = 0, b = 0, a = 0;
		for(u32 y = 0; y < size; ++y)
		{
			for(u32 x = 0; x < size; ++x)
			{
				const Color s = source->GetColorAt(x, y);
				r += s.R; g += s.G; b += s.B; a += s.A;
			}
		}
		const float inv = 1.0f / 16.0f;
		r *= inv; g *= inv; b *= inv; a *= inv;

		const Color m = mips[2]->GetColorAt(0, 0);
		B3D_TEST_ASSERT(std::fabs(r - m.R) < tolerance)
		B3D_TEST_ASSERT(std::fabs(g - m.G) < tolerance)
		B3D_TEST_ASSERT(std::fabs(b - m.B) < tolerance)
		B3D_TEST_ASSERT(std::fabs(a - m.A) < tolerance)
	}
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
