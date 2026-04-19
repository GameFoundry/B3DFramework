//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFreeImgImporter.h"
#include "Resources/B3DResource.h"
#include "Debug/B3DDebug.h"
#include "FileSystem/B3DDataStream.h"
#include "Managers/B3DTextureManager.h"
#include "Image/B3DTexture.h"
#include "Importer/B3DTextureImportOptions.h"
#include "FileSystem/B3DFileSystem.h"
#include "CoreObject/B3DRenderThread.h"
#include "Math/B3DMath.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "FreeImage.h"
#include "Utility/B3DBitwise.h"
#include "Renderer/B3DRenderer.h"

using namespace b3d;

void FreeImageLoadErrorHandler(FREE_IMAGE_FORMAT fif, const char* message)
{
	// Callback method as required by FreeImage to report problems
	const char* typeName = FreeImage_GetFormatFromFIF(fif);
	if(typeName)
		B3D_LOG(Error, LogFreeImageImporter, "FreeImage error: '{0}' when loading format {1}", message, typeName);
	else
		B3D_LOG(Error, LogFreeImageImporter, "FreeImage error: '{0}'", message);
}

FreeImgImporter::FreeImgImporter()
{
	FreeImage_Initialise(false);

	// Register codecs
	StringStream strExt;
	strExt << "Supported formats: ";
	bool first = true;
	for(int i = 0; i < FreeImage_GetFIFCount(); ++i)
	{
		// Skip DDS codec since FreeImage does not have the option
		// to keep DXT data compressed, we'll use our own codec
		if((FREE_IMAGE_FORMAT)i == FIF_DDS)
			continue;

		String exts = String(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i));
		if(!first)
			strExt << ",";

		first = false;
		strExt << exts;

		// Pull off individual formats (separated by comma by FI)
		Vector<String> extsVector = StringUtility::Split(exts, u8",");
		for(auto v = extsVector.begin(); v != extsVector.end(); ++v)
		{
			auto findIter = std::find(mExtensions.begin(), mExtensions.end(), *v);

			if(findIter == mExtensions.end())
			{
				String ext = *v;
				StringUtility::ToLowerCase(ext);

				mExtensionToFID.insert(std::make_pair(ext, i));
				mExtensions.push_back(ext);
			}
		}
	}

	// Set error handler
	FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);
}

FreeImgImporter::~FreeImgImporter()
{
	FreeImage_DeInitialise();
}

bool FreeImgImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtility::ToLowerCase(lowerCaseExt);

	return find(mExtensions.begin(), mExtensions.end(), lowerCaseExt) != mExtensions.end();
}

bool FreeImgImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	String ext = MagicNumToExtension(magicNumPtr, numBytes);

	return IsExtensionSupported(ext);
}

String FreeImgImporter::MagicNumToExtension(const u8* magic, u32 maxBytes) const
{
	// Set error handler
	FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);

	FIMEMORY* fiMem =
		FreeImage_OpenMemory((BYTE*)magic, static_cast<DWORD>(maxBytes));

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem, (int)maxBytes);
	FreeImage_CloseMemory(fiMem);

	if(fif != FIF_UNKNOWN)
	{
		String ext = String(FreeImage_GetFormatFromFIF(fif));
		StringUtility::ToLowerCase(ext);
		return ext;
	}
	else
	{
		return StringUtility::kBlank;
	}
}

SPtr<ImportOptions> FreeImgImporter::CreateImportOptions() const
{
	return B3DMakeShared<TextureImportOptions>();
}

SPtr<Resource> FreeImgImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	const TextureImportOptions* textureImportOptions = static_cast<const TextureImportOptions*>(importOptions.get());

	SPtr<PixelData> imgData = ImportRawImage(filePath);
	if(imgData == nullptr || imgData->GetData() == nullptr)
		return nullptr;

	Vector<SPtr<PixelData>> faceData;

	TextureType texType;
	if(textureImportOptions->Cubemap)
	{
		texType = TEX_TYPE_CUBE_MAP;

		std::array<SPtr<PixelData>, 6> cubemapFaces;
		if(GenerateCubemap(imgData, textureImportOptions->CubemapSourceType, cubemapFaces))
		{
			faceData.insert(faceData.begin(), cubemapFaces.begin(), cubemapFaces.end());
		}
		else // Fall-back to 2D texture
		{
			texType = TEX_TYPE_2D;
			faceData.push_back(imgData);
		}
	}
	else
	{
		texType = TEX_TYPE_2D;
		faceData.push_back(imgData);
	}

	u32 numMips = 0;
	if(textureImportOptions->GenerateMips &&
	   Bitwise::IsPow2(faceData[0]->GetWidth()) && Bitwise::IsPow2(faceData[0]->GetHeight()))
	{
		u32 maxPossibleMip = PixelUtility::GetMipmapCount(faceData[0]->GetWidth(), faceData[0]->GetHeight(), faceData[0]->GetDepth(), faceData[0]->GetFormat());

		if(textureImportOptions->MaxMip == 0)
			numMips = maxPossibleMip;
		else
			numMips = std::min(maxPossibleMip, textureImportOptions->MaxMip);
	}

	TextureUsageFlags usage = TextureUsageFlag::Default;
	if(textureImportOptions->CpuCached)
		usage |= TextureUsageFlag::CPUCached;

	const bool sRGB = textureImportOptions->SRgb;
	const String fileName = filePath.GetFilename(false);

	TextureCreateInformation texDesc;
	texDesc.Name = fileName;
	texDesc.Type = texType;
	texDesc.Width = faceData[0]->GetWidth();
	texDesc.Height = faceData[0]->GetHeight();
	texDesc.MipMapCount = numMips;
	texDesc.Format = textureImportOptions->Format;
	texDesc.Usage = usage;
	texDesc.UseHardwareSRGB = sRGB;

	SPtr<Texture> newTexture = Texture::CreateShared(texDesc);

	u32 numFaces = (u32)faceData.size();
	for(u32 i = 0; i < numFaces; i++)
	{
		Vector<SPtr<PixelData>> mipLevels;
		if(numMips > 0)
		{
			MipMapGenOptions mipOptions;
			mipOptions.IsSrgb = sRGB;

			mipLevels = PixelUtility::GenerateMipmaps(faceData[i], mipOptions);
		}
		else
			mipLevels.push_back(faceData[i]);

		for(u32 mip = 0; mip < (u32)mipLevels.size(); ++mip)
		{
			SPtr<PixelData> dst = newTexture->GetProperties().AllocBuffer(0, mip);

			PixelUtility::BulkPixelConversion(*mipLevels[mip], *dst);
			newTexture->WriteData(dst, i, mip);
		}
	}

	return newTexture;
}

SPtr<PixelData> FreeImgImporter::ImportRawImage(const Path& filePath)
{
	UPtr<MemoryDataStream> memStream;
	FREE_IMAGE_FORMAT imageFormat;
	{
		SPtr<DataStream> fileData = FileSystem::OpenFile(filePath, true);
		if(fileData->Size() > std::numeric_limits<u32>::max())
		{
			B3D_ENSURE_LOG(false, "File size larger than supported!");
			return nullptr;
		}

		u32 magicLen = std::min((u32)fileData->Size(), 32u);
		u8 magicBuf[32];
		fileData->Read(magicBuf, magicLen);
		fileData->Seek(0);

		String fileExtension = MagicNumToExtension(magicBuf, magicLen);
		auto findFormat = mExtensionToFID.find(fileExtension);
		if(findFormat == mExtensionToFID.end())
		{
			B3D_ENSURE_LOG(false, "Type of the file provided is not supported by this importer. File type: {0}", fileExtension);
			return nullptr;
		}

		imageFormat = (FREE_IMAGE_FORMAT)findFormat->second;

		// Set error handler
		FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);

		// Buffer stream into memory (TODO: override IO functions instead?)
		memStream = B3DMakeUnique<MemoryDataStream>(fileData);
		fileData->Close();
	}

	if(!memStream)
		return nullptr;

	FIMEMORY* fiMem = FreeImage_OpenMemory(memStream->Data(), static_cast<DWORD>(memStream->Size()));

	FIBITMAP* fiBitmap = FreeImage_LoadFromMemory(
		(FREE_IMAGE_FORMAT)imageFormat, fiMem);
	if(!fiBitmap)
	{
		B3D_ENSURE_LOG(false, "Error decoding image");
		return nullptr;
	}

	u32 width = FreeImage_GetWidth(fiBitmap);
	u32 height = FreeImage_GetHeight(fiBitmap);
	PixelFormat format = PF_UNKNOWN;

	// Must derive format first, this may perform conversions

	FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(fiBitmap);
	FREE_IMAGE_COLOR_TYPE colourType = FreeImage_GetColorType(fiBitmap);
	unsigned bpp = FreeImage_GetBPP(fiBitmap);
	unsigned srcElemSize = 0;

	switch(imageType)
	{
	case FIT_UNKNOWN:
	case FIT_COMPLEX:
	case FIT_UINT32:
	case FIT_INT32:
	case FIT_DOUBLE:
	default:
		B3D_ENSURE_LOG(false, "Unknown or unsupported image format");
		return nullptr;
	case FIT_BITMAP:
		// Standard image type
		// Perform any colour conversions for greyscale
		if(colourType == FIC_MINISWHITE || colourType == FIC_MINISBLACK)
		{
			FIBITMAP* newBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
			// free old bitmap and replace
			FreeImage_Unload(fiBitmap);
			fiBitmap = newBitmap;
			// get new formats
			bpp = FreeImage_GetBPP(fiBitmap);
			colourType = FreeImage_GetColorType(fiBitmap);
		}
		// Perform any colour conversions for RGB
		else if(bpp < 8 || colourType == FIC_PALETTE || colourType == FIC_CMYK)
		{
			FIBITMAP* newBitmap = FreeImage_ConvertTo24Bits(fiBitmap);
			// free old bitmap and replace
			FreeImage_Unload(fiBitmap);
			fiBitmap = newBitmap;
			// get new formats
			bpp = FreeImage_GetBPP(fiBitmap);
			colourType = FreeImage_GetColorType(fiBitmap);
		}

		// by this stage, 8-bit is greyscale, 16/24/32 bit are RGB[A]
		switch(bpp)
		{
		case 8:
			format = PF_R8;
			srcElemSize = 1;
			break;
		case 16:
			// Determine 555 or 565 from green mask
			// cannot be 16-bit greyscale since that's FIT_u16
			if(FreeImage_GetGreenMask(fiBitmap) == FI16_565_GREEN_MASK)
			{
				B3D_ASSERT(false && "Format not supported by the engine. TODO.");
				return nullptr;
			}
			else
			{
				B3D_ASSERT(false && "Format not supported by the engine. TODO.");
				return nullptr;
				// FreeImage doesn't support 4444 format so must be 1555
			}
			srcElemSize = 2;
			break;
		case 24:
			// FreeImage differs per platform
			//     PF_BYTE_BGR[A] for little endian (== PF_ARGB native)
			//     PF_BYTE_RGB[A] for big endian (== PF_RGBA native)
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
			format = PF_RGB8;
#else
			format = PF_BGR8;
#endif
			srcElemSize = 3;
			break;
		case 32:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
			format = PF_RGBA8;
#else
			format = PF_BGRA8;
#endif
			srcElemSize = 4;
			break;
		};
		break;
	case FIT_UINT16:
	case FIT_INT16:
		// 16-bit greyscale
		B3D_ASSERT(false && "No INT pixel formats supported currently. TODO.");
		return nullptr;
		break;
	case FIT_FLOAT:
		// Single-component floating point data
		format = PF_R32F;
		srcElemSize = 4;
		break;
	case FIT_RGB16:
		format = PF_RGBA16;
		srcElemSize = 2 * 3;
		break;
	case FIT_RGBA16:
		format = PF_RGBA16;
		srcElemSize = 2 * 4;
		break;
	case FIT_RGBF:
		format = PF_RGB32F;
		srcElemSize = 4 * 3;
		break;
	case FIT_RGBAF:
		format = PF_RGBA32F;
		srcElemSize = 4 * 4;
		break;
	};

	unsigned char* srcData = FreeImage_GetBits(fiBitmap);
	unsigned srcPitch = FreeImage_GetPitch(fiBitmap);

	// Final data - invert image and trim pitch at the same time
	u32 dstElemSize = PixelUtility::GetElementByteCount(format);
	u32 dstPitch = width * PixelUtility::GetElementByteCount(format);

	// Bind output buffer
	SPtr<PixelData> texData = B3DMakeShared<PixelData>(width, height, 1, format);
	texData->AllocateInternalBuffer();
	u8* output = texData->GetData();

	u8* pSrc;
	u8* pDst = output;

	// Copy row by row, which is faster
	if(srcElemSize == dstElemSize)
	{
		for(u32 y = 0; y < height; ++y)
		{
			pSrc = srcData + (height - y - 1) * srcPitch;
			memcpy(pDst, pSrc, dstPitch);
			pDst += dstPitch;
		}
	}
	else
	{
		for(u32 y = 0; y < height; ++y)
		{
			pSrc = srcData + (height - y - 1) * srcPitch;

			for(u32 x = 0; x < width; ++x)
				memcpy(pDst + x * dstElemSize, pSrc + x * srcElemSize, srcElemSize);

			pDst += dstPitch;
		}
	}

	FreeImage_Unload(fiBitmap);
	FreeImage_CloseMemory(fiMem);

	return texData;
}

/**
 * Reads the source texture as a horizontal or vertical list of 6 cubemap faces.
 *
 * @param[in]	source		Source texture to read.
 * @param[out]	output		Output array that will contain individual cubemap faces.
 * @param[in]	faceSize	Size of a single face, in pixels. Both width & height must match.
 * @param[in]	vertical	True if the faces are laid out vertically, false if horizontally.
 */
void ReadCubemapList(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, u32 faceSize, bool vertical)
{
	Vector2I faceStart(kZeroTag);
	for(u32 i = 0; i < 6; i++)
	{
		output[i] = PixelData::Create(faceSize, faceSize, 1, source->GetFormat());

		PixelVolume volume(faceStart.X, faceStart.Y, faceStart.X + faceSize, faceStart.Y + faceSize);
		PixelUtility::Copy(*source, *output[i], faceStart.X, faceStart.Y);

		if(vertical)
			faceStart.Y += faceSize;
		else
			faceStart.X += faceSize;
	}
}

/**
 * Reads the source texture as a horizontal or vertical "cross" of 6 cubemap faces.
 *
 * Vertical layout:
 *    +Y
 * -X +Z +X
 *    -Y
 *    -Z
 *
 * Horizontal layout:
 *    +Y
 * -X +Z +X -Z
 *    -Y
 *
 * @param[in]	source		Source texture to read.
 * @param[out]	output		Output array that will contain individual cubemap faces.
 * @param[in]	faceSize	Size of a single face, in pixels. Both width & height must match.
 * @param[in]	vertical	True if the faces are laid out vertically, false if horizontally.
 */
void ReadCubemapCross(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, u32 faceSize, bool vertical)
{
	const static u32 kVertFaceIndices[] = { 5, 3, 1, 7, 4, 10 };
	const static u32 kHorzFaceIndices[] = { 6, 4, 1, 9, 5, 7 };

	const u32* faceIndices = vertical ? kVertFaceIndices : kHorzFaceIndices;
	u32 numFacesInRow = vertical ? 3 : 4;

	for(u32 i = 0; i < 6; i++)
	{
		output[i] = PixelData::Create(faceSize, faceSize, 1, source->GetFormat());

		u32 faceX = (faceIndices[i] % numFacesInRow) * faceSize;
		u32 faceY = (faceIndices[i] / numFacesInRow) * faceSize;

		PixelVolume volume(faceX, faceY, faceX + faceSize, faceY + faceSize);
		PixelUtility::Copy(*source, *output[i], faceX, faceY);
	}

	// Flip -Z as it's upside down
	if(vertical)
		PixelUtility::Mirror(*output[5], MirrorModeBits::X | MirrorModeBits::Y);
}

/** Method that maps a direction to a point on a plane in range [0, 1] using spherical mapping. */
Vector2 MapCubemapDirToSpherical(const Vector3& dir)
{
	// Using the OpenGL spherical mapping formula
	Vector3 nrmDir = Vector3::Normalize(dir);
	nrmDir.Z += 1.0f;

	float m = 2 * nrmDir.Length();

	float u = nrmDir.X / m + 0.5f;
	float v = nrmDir.Y / m + 0.5f;

	return Vector2(u, v);
}

/**
 * Method that maps a direction to a point on a plane in range [0, 1] using cylindrical mapping. This mapping is also
 * know as longitude-latitude mapping, Blinn/Newell mapping or equirectangular cylindrical mapping.
 */
Vector2 MapCubemapDirToCylindrical(const Vector3& dir)
{
	Vector3 nrmDir = Vector3::Normalize(dir);

	float u = (atan2(nrmDir.X, nrmDir.Z) + Math::kPi) / Math::kTwoPi;
	float v = acos(nrmDir.Y) / Math::kPi;

	return Vector2(u, v);
}

/** Resizes the provided cubemap faces and outputs a new set of resized faces. */
void DownsampleCubemap(const std::array<SPtr<PixelData>, 6>& input, std::array<SPtr<PixelData>, 6>& output, u32 size)
{
	for(u32 i = 0; i < 6; i++)
	{
		output[i] = PixelData::Create(size, size, 1, input[i]->GetFormat());
		PixelUtility::Scale(*input[i], *output[i]);
	}
}

/**
 * Reads the source texture and remaps its data into six faces of a cubemap.
 *
 * @param[in]	source		Source texture to remap.
 * @param[out]	output		Remapped faces of the cubemap.
 * @param[in]	faceSize	Width/height of each individual face, in pixels.
 * @param[in]	remap		Function to use for remapping the cubemap direction to UV.
 */
void ReadCubemapUvRemap(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, u32 faceSize, const std::function<Vector2(Vector3)>& remap)
{
	struct RemapInfo
	{
		int Idx[3];
		Vector3 Sign;
	};

	// Mapping from default (X, Y, 1.0f) plane to relevant cube face. Also flipping Y so it corresponds to how pixel
	// coordinates are mapped.
	static const RemapInfo kRemapLookup[] = {
		{ { 2, 1, 0 }, { -1.0f, -1.0f, 1.0f } }, // X+
		{ { 2, 1, 0 }, { 1.0f, -1.0f, -1.0f } }, // X-
		{ { 0, 2, 1 }, { 1.0f, 1.0f, 1.0f } }, // Y+
		{ { 0, 2, 1 }, { 1.0f, -1.0f, -1.0f } }, // Y-
		{ { 0, 1, 2 }, { 1.0f, -1.0f, 1.0f } }, // Z+
		{ { 0, 1, 2 }, { -1.0f, -1.0f, -1.0f } } // Z-
	};

	float invSize = 1.0f / faceSize;
	for(u32 faceIdx = 0; faceIdx < 6; faceIdx++)
	{
		output[faceIdx] = PixelData::Create(faceSize, faceSize, 1, source->GetFormat());

		const RemapInfo& remapInfo = kRemapLookup[faceIdx];
		for(u32 y = 0; y < faceSize; y++)
		{
			for(u32 x = 0; x < faceSize; x++)
			{
				// Map from pixel coordinates to [-1, 1] range.
				Vector2 sourceCoord = (Vector2((float)x, (float)y) * invSize) * 2.0f - Vector2(1.0f, 1.0f);
				Vector3 direction = Vector3(sourceCoord.X, sourceCoord.Y, 1.0f);

				direction *= remapInfo.Sign;

				// Rotate towards current face
				Vector3 rotatedDir;
				rotatedDir[remapInfo.Idx[0]] = direction.X;
				rotatedDir[remapInfo.Idx[1]] = direction.Y;
				rotatedDir[remapInfo.Idx[2]] = direction.Z;

				// Find location in the source to sample from
				Vector2 sourceUV = remap(rotatedDir);
				Color color = source->SampleColorAt(sourceUV);

				// Write the sampled color
				output[faceIdx]->SetColorAt(color, x, y);
			}
		}
	}
}

bool FreeImgImporter::GenerateCubemap(const SPtr<PixelData>& source, CubemapSourceType sourceType, std::array<SPtr<PixelData>, 6>& output)
{
	// Note: Expose this as a parameter if needed:
	u32 cubemapSupersampling = 1; // Set to amount of samples

	Vector2I faceSize;
	bool cross = false;
	bool vertical = false;

	switch(sourceType)
	{
	case CubemapSourceType::Faces:
		{
			float aspect = source->GetWidth() / (float)source->GetHeight();

			if(Math::ApproxEquals(aspect, 6.0f)) // Horizontal list
			{
				faceSize.X = source->GetWidth() / 6;
				faceSize.Y = source->GetHeight();
			}
			else if(Math::ApproxEquals(aspect, 1.0f / 6.0f)) // Vertical list
			{
				faceSize.X = source->GetWidth();
				faceSize.Y = source->GetHeight() / 6;
				vertical = true;
			}
			else if(Math::ApproxEquals(aspect, 4.0f / 3.0f)) // Horizontal cross
			{
				faceSize.X = source->GetWidth() / 4;
				faceSize.Y = source->GetHeight() / 3;
				cross = true;
			}
			else if(Math::ApproxEquals(aspect, 3.0f / 4.0f)) // Vertical cross
			{
				faceSize.X = source->GetWidth() / 3;
				faceSize.Y = source->GetHeight() / 4;
				cross = true;
				vertical = true;
			}
			else
			{
				B3D_LOG(Warning, LogFreeImageImporter, "Unable to generate a cubemap: unrecognized face configuration.");
				return false;
			}
		}
		break;
	case CubemapSourceType::Cylindrical:
	case CubemapSourceType::Spherical:
		// Half of the source size will be used for each cube face, which should yield good enough quality
		faceSize.X = std::max(source->GetWidth(), source->GetHeight()) / 2;
		faceSize.X = Bitwise::ClosestPow2(faceSize.X);

		// Don't allow sizes larger than 4096
		faceSize.X = std::min(faceSize.X, 4096);

		faceSize.Y = faceSize.X;

		break;
	default: // Assuming single-image
		faceSize.X = source->GetWidth();
		faceSize.Y = source->GetHeight();
		break;
	}

	if(faceSize.X != faceSize.Y)
	{
		B3D_LOG(Warning, LogFreeImageImporter, "Unable to generate a cubemap: width & height must match.");
		return false;
	}

	if(!Bitwise::IsPow2(faceSize.X))
	{
		B3D_LOG(Warning, LogFreeImageImporter, "Unable to generate a cubemap: width & height must be powers of 2.");
		return false;
	}

	switch(sourceType)
	{
	case CubemapSourceType::Faces:
		{
			if(cross)
				ReadCubemapCross(source, output, faceSize.X, vertical);
			else
				ReadCubemapList(source, output, faceSize.X, vertical);
		}
		break;
	case CubemapSourceType::Cylindrical:
		{
			u32 superSampledFaceSize = faceSize.X * cubemapSupersampling;

			std::array<SPtr<PixelData>, 6> superSampledOutput;
			ReadCubemapUvRemap(source, superSampledOutput, superSampledFaceSize, &MapCubemapDirToCylindrical);

			if(faceSize.X != (i32)superSampledFaceSize)
				DownsampleCubemap(superSampledOutput, output, faceSize.X);
			else
				output = superSampledOutput;
		}
		break;
	case CubemapSourceType::Spherical:
		{
			u32 superSampledFaceSize = faceSize.X * cubemapSupersampling;

			std::array<SPtr<PixelData>, 6> superSampledOutput;
			ReadCubemapUvRemap(source, superSampledOutput, superSampledFaceSize, &MapCubemapDirToSpherical);

			if(faceSize.X != (i32)superSampledFaceSize)
				DownsampleCubemap(superSampledOutput, output, faceSize.X);
			else
				output = superSampledOutput;
		}
		break;
	default: // Single-image
		for(u32 i = 0; i < 6; i++)
			output[i] = source;

		break;
	}

	return true;
}
