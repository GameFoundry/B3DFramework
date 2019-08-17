//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFreeImgImporter.h"
#include "Resources/BsResource.h"
#include "Debug/BsDebug.h"
#include "FileSystem/BsDataStream.h"
#include "Managers/BsTextureManager.h"
#include "Image/BsTexture.h"
#include "Importer/BsTextureImportOptions.h"
#include "FileSystem/BsFileSystem.h"
#include "BsCoreApplication.h"
#include "CoreThread/BsCoreThread.h"
#include "Math/BsMath.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "FreeImage.h"
#include "Utility/BsBitwise.h"
#include "Renderer/BsRenderer.h"

using namespace std::placeholders;

namespace bs
{
	void FreeImageLoadErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
	{
		// Callback method as required by FreeImage to report problems
		const char* typeName = FreeImage_GetFormatFromFIF(fif);
		if (typeName)
			BS_LOG(Error, FreeImageImporter, "FreeImage error: '{0}' when loading format {1}", message, typeName);
		else
			BS_LOG(Error, FreeImageImporter, "FreeImage error: '{0}'", message);
	}

	FreeImgImporter::FreeImgImporter()
	{
		FreeImage_Initialise(false);

		// Register codecs
		StringStream strExt;
		strExt << "Supported formats: ";
		bool first = true;
		for (int i = 0; i < FreeImage_GetFIFCount(); ++i)
		{
			// Skip DDS codec since FreeImage does not have the option
			// to keep DXT data compressed, we'll use our own codec
			if ((FREE_IMAGE_FORMAT)i == FIF_DDS)
				continue;

			String exts = String(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i));
			if (!first)
				strExt << ",";

			first = false;
			strExt << exts;

			// Pull off individual formats (separated by comma by FI)
			Vector<String> extsVector = StringUtil::split(exts, u8",");
			for (auto v = extsVector.begin(); v != extsVector.end(); ++v)
			{
				auto findIter = std::find(mExtensions.begin(), mExtensions.end(), *v);

				if(findIter == mExtensions.end())
				{
					String ext = *v;
					StringUtil::toLowerCase(ext);

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

	bool FreeImgImporter::isExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return find(mExtensions.begin(), mExtensions.end(), lowerCaseExt) != mExtensions.end();
	}

	bool FreeImgImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		String ext = magicNumToExtension(magicNumPtr, numBytes);

		return isExtensionSupported(ext);
	}

	String FreeImgImporter::magicNumToExtension(const UINT8* magic, UINT32 maxBytes) const
	{
		// Set error handler
		FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);

		FIMEMORY* fiMem =
			FreeImage_OpenMemory((BYTE*)magic, static_cast<DWORD>(maxBytes));

		FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem, (int)maxBytes);
		FreeImage_CloseMemory(fiMem);

		if (fif != FIF_UNKNOWN)
		{
			String ext = String(FreeImage_GetFormatFromFIF(fif));
			StringUtil::toLowerCase(ext);
			return ext;
		}
		else
		{
			return StringUtil::BLANK;
		}
	}

	SPtr<ImportOptions> FreeImgImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<TextureImportOptions>();
	}

	SPtr<Resource> FreeImgImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		const TextureImportOptions* textureImportOptions = static_cast<const TextureImportOptions*>(importOptions.get());

		SPtr<PixelData> imgData = importRawImage(filePath);
		if(imgData == nullptr || imgData->getData() == nullptr)
			return nullptr;

		Vector<SPtr<PixelData>> faceData;

		TextureType texType;
		if(textureImportOptions->cubemap)
		{
			texType = TEX_TYPE_CUBE_MAP;

			std::array<SPtr<PixelData>, 6> cubemapFaces;
			if (generateCubemap(imgData, textureImportOptions->cubemapSourceType, cubemapFaces))
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

		UINT32 numMips = 0;
		if (textureImportOptions->generateMips &&
			Bitwise::isPow2(faceData[0]->getWidth()) && Bitwise::isPow2(faceData[0]->getHeight()))
		{
			UINT32 maxPossibleMip = PixelUtil::getMaxMipmaps(faceData[0]->getWidth(), faceData[0]->getHeight(),
				faceData[0]->getDepth(), faceData[0]->getFormat());

			if (textureImportOptions->maxMip == 0)
				numMips = maxPossibleMip;
			else
				numMips = std::min(maxPossibleMip, textureImportOptions->maxMip);
		}

		int usage = TU_DEFAULT;
		if (textureImportOptions->cpuCached)
			usage |= TU_CPUCACHED;

		bool sRGB = textureImportOptions->sRGB;

		TEXTURE_DESC texDesc;
		texDesc.type = texType;
		texDesc.width = faceData[0]->getWidth();
		texDesc.height = faceData[0]->getHeight();
		texDesc.numMips = numMips;
		texDesc.format = textureImportOptions->format;
		texDesc.usage = usage;
		texDesc.hwGamma = sRGB;

		SPtr<Texture> newTexture = Texture::_createPtr(texDesc);

		UINT32 numFaces = (UINT32)faceData.size();
		for (UINT32 i = 0; i < numFaces; i++)
		{
			Vector<SPtr<PixelData>> mipLevels;
			if (numMips > 0)
			{
				MipMapGenOptions mipOptions;
				mipOptions.isSRGB = sRGB;

				mipLevels = PixelUtil::genMipmaps(*faceData[i], mipOptions);
			}
			else
				mipLevels.push_back(faceData[i]);

			for (UINT32 mip = 0; mip < (UINT32)mipLevels.size(); ++mip)
			{
				SPtr<PixelData> dst = newTexture->getProperties().allocBuffer(0, mip);

				PixelUtil::bulkPixelConversion(*mipLevels[mip], *dst);
				newTexture->writeData(dst, i, mip);
			}
		}

		const String fileName = filePath.getFilename(false);
		newTexture->setName(fileName);

		return newTexture;
	}

	SPtr<PixelData> FreeImgImporter::importRawImage(const Path& filePath)
	{
		UPtr<MemoryDataStream> memStream;
		FREE_IMAGE_FORMAT imageFormat;
		{
			Lock lock = FileScheduler::getLock(filePath);

			SPtr<DataStream> fileData = FileSystem::openFile(filePath, true);
			if (fileData->size() > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "File size larger than supported!");
			}

			UINT32 magicLen = std::min((UINT32)fileData->size(), 32u);
			UINT8 magicBuf[32];
			fileData->read(magicBuf, magicLen);
			fileData->seek(0);

			String fileExtension = magicNumToExtension(magicBuf, magicLen);
			auto findFormat = mExtensionToFID.find(fileExtension);
			if (findFormat == mExtensionToFID.end())
			{
				BS_EXCEPT(InvalidParametersException, "Type of the file provided is not supported by this importer. File type: " + fileExtension);
			}

			imageFormat = (FREE_IMAGE_FORMAT)findFormat->second;

			// Set error handler
			FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);

			// Buffer stream into memory (TODO: override IO functions instead?)
			memStream = bs_unique_ptr_new<MemoryDataStream>(fileData);
			fileData->close();
		}

		if(!memStream)
			return nullptr;

		FIMEMORY* fiMem = FreeImage_OpenMemory(memStream->data(), static_cast<DWORD>(memStream->size()));

		FIBITMAP* fiBitmap = FreeImage_LoadFromMemory(
			(FREE_IMAGE_FORMAT)imageFormat, fiMem);
		if (!fiBitmap)
		{
			BS_EXCEPT(InternalErrorException, "Error decoding image");
		}

		UINT32 width = FreeImage_GetWidth(fiBitmap);
		UINT32 height = FreeImage_GetHeight(fiBitmap);
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
			BS_EXCEPT(InternalErrorException, "Unknown or unsupported image format");

			break;
		case FIT_BITMAP:
			// Standard image type
			// Perform any colour conversions for greyscale
			if (colourType == FIC_MINISWHITE || colourType == FIC_MINISBLACK)
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
			else if (bpp < 8 || colourType == FIC_PALETTE || colourType == FIC_CMYK)
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
				// cannot be 16-bit greyscale since that's FIT_UINT16
				if(FreeImage_GetGreenMask(fiBitmap) == FI16_565_GREEN_MASK)
				{
					assert(false && "Format not supported by the engine. TODO.");
					return nullptr;
				}
				else
				{
					assert(false && "Format not supported by the engine. TODO.");
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
			assert(false && "No INT pixel formats supported currently. TODO.");
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
		UINT32 dstElemSize = PixelUtil::getNumElemBytes(format);
		UINT32 dstPitch = width * PixelUtil::getNumElemBytes(format);

		// Bind output buffer
		SPtr<PixelData> texData = bs_shared_ptr_new<PixelData>(width, height, 1, format);
		texData->allocateInternalBuffer();
		UINT8* output = texData->getData();

		UINT8* pSrc;
		UINT8* pDst = output;

		// Copy row by row, which is faster
		if (srcElemSize == dstElemSize)
		{
			for (UINT32 y = 0; y < height; ++y)
			{
				pSrc = srcData + (height - y - 1) * srcPitch;
				memcpy(pDst, pSrc, dstPitch);
				pDst += dstPitch;
			}
		}
		else
		{
			for (UINT32 y = 0; y < height; ++y)
			{
				pSrc = srcData + (height - y - 1) * srcPitch;

				for(UINT32 x = 0; x < width; ++x)
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
	void readCubemapList(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, UINT32 faceSize, bool vertical)
	{
		Vector2I faceStart;
		for(UINT32 i = 0; i < 6; i++)
		{
			output[i] = PixelData::create(faceSize, faceSize, 1, source->getFormat());

			PixelVolume volume(faceStart.x, faceStart.y, faceStart.x + faceSize, faceStart.y + faceSize);
			PixelUtil::copy(*source, *output[i], faceStart.x, faceStart.y);

			if (vertical)
				faceStart.y += faceSize;
			else
				faceStart.x += faceSize;
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
	void readCubemapCross(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, UINT32 faceSize,
		bool vertical)
	{
		const static UINT32 vertFaceIndices[] = { 5, 3, 1, 7, 4, 10 };
		const static UINT32 horzFaceIndices[] = { 6, 4, 1, 9, 5, 7 };

		const UINT32* faceIndices = vertical ? vertFaceIndices : horzFaceIndices;
		UINT32 numFacesInRow = vertical ? 3 : 4;

		for (UINT32 i = 0; i < 6; i++)
		{
			output[i] = PixelData::create(faceSize, faceSize, 1, source->getFormat());

			UINT32 faceX = (faceIndices[i] % numFacesInRow) * faceSize;
			UINT32 faceY = (faceIndices[i] / numFacesInRow) * faceSize;

			PixelVolume volume(faceX, faceY, faceX + faceSize, faceY + faceSize);
			PixelUtil::copy(*source, *output[i], faceX, faceY);
		}

		// Flip -Z as it's upside down
		if (vertical)
			PixelUtil::mirror(*output[5], MirrorModeBits::X | MirrorModeBits::Y);
	}

	/** Method that maps a direction to a point on a plane in range [0, 1] using spherical mapping. */
	Vector2 mapCubemapDirToSpherical(const Vector3& dir)
	{
		// Using the OpenGL spherical mapping formula
		Vector3 nrmDir = Vector3::normalize(dir);
		nrmDir.z += 1.0f;

		float m = 2 * nrmDir.length();

		float u = nrmDir.x / m + 0.5f;
		float v = nrmDir.y / m + 0.5f;

		return Vector2(u, v);
	}

	/**
	 * Method that maps a direction to a point on a plane in range [0, 1] using cylindrical mapping. This mapping is also
	 * know as longitude-latitude mapping, Blinn/Newell mapping or equirectangular cylindrical mapping.
	 */
	Vector2 mapCubemapDirToCylindrical(const Vector3& dir)
	{
		Vector3 nrmDir = Vector3::normalize(dir);

		float u = (atan2(nrmDir.x, nrmDir.z) + Math::PI) / Math::TWO_PI;
		float v = acos(nrmDir.y) / Math::PI;

		return Vector2(u, v);
	}

	/** Resizes the provided cubemap faces and outputs a new set of resized faces. */
	void downsampleCubemap(const std::array<SPtr<PixelData>, 6>& input, std::array<SPtr<PixelData>, 6>& output, UINT32 size)
	{
		for(UINT32 i = 0; i < 6; i++)
		{
			output[i] = PixelData::create(size, size, 1, input[i]->getFormat());
			PixelUtil::scale(*input[i], *output[i]);
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
	void readCubemapUVRemap(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, UINT32 faceSize,
		const std::function<Vector2(Vector3)>& remap)
	{
		struct RemapInfo
		{
			int idx[3];
			Vector3 sign;
		};

		// Mapping from default (X, Y, 1.0f) plane to relevant cube face. Also flipping Y so it corresponds to how pixel
		// coordinates are mapped.
		static const RemapInfo remapLookup[] =
		{
			{ {2, 1, 0}, { -1.0f, -1.0f,  1.0f }}, // X+
			{ {2, 1, 0}, {  1.0f, -1.0f, -1.0f }}, // X-
			{ {0, 2, 1}, {  1.0f,  1.0f,  1.0f }}, // Y+
			{ {0, 2, 1}, {  1.0f, -1.0f, -1.0f }}, // Y-
			{ {0, 1, 2}, {  1.0f, -1.0f,  1.0f }}, // Z+
			{ {0, 1, 2}, { -1.0f, -1.0f, -1.0f }}  // Z-
		};

		float invSize = 1.0f / faceSize;
		for (UINT32 faceIdx = 0; faceIdx < 6; faceIdx++)
		{
			output[faceIdx] = PixelData::create(faceSize, faceSize, 1, source->getFormat());

			const RemapInfo& remapInfo = remapLookup[faceIdx];
			for (UINT32 y = 0; y < faceSize; y++)
			{
				for (UINT32 x = 0; x < faceSize; x++)
				{
					// Map from pixel coordinates to [-1, 1] range.
					Vector2 sourceCoord = (Vector2((float)x, (float)y) * invSize) * 2.0f - Vector2(1.0f, 1.0f);
					Vector3 direction = Vector3(sourceCoord.x, sourceCoord.y, 1.0f);

					direction *= remapInfo.sign;

					// Rotate towards current face
					Vector3 rotatedDir;
					rotatedDir[remapInfo.idx[0]] = direction.x;
					rotatedDir[remapInfo.idx[1]] = direction.y;
					rotatedDir[remapInfo.idx[2]] = direction.z;

					// Find location in the source to sample from
					Vector2 sourceUV = remap(rotatedDir);
					Color color = source->sampleColorAt(sourceUV);

					// Write the sampled color
					output[faceIdx]->setColorAt(color, x, y);
				}
			}
		}
	}

	bool FreeImgImporter::generateCubemap(const SPtr<PixelData>& source, CubemapSourceType sourceType,
						 std::array<SPtr<PixelData>, 6>& output)
	{
		// Note: Expose this as a parameter if needed:
		UINT32 cubemapSupersampling = 1; // Set to amount of samples

		Vector2I faceSize;
		bool cross = false;
		bool vertical = false;

		switch(sourceType)
		{
		case CubemapSourceType::Faces:
			{
				float aspect = source->getWidth() / (float)source->getHeight();
				
				if(Math::approxEquals(aspect, 6.0f)) // Horizontal list
				{
					faceSize.x = source->getWidth() / 6;
					faceSize.y = source->getHeight();
				}
				else if(Math::approxEquals(aspect, 1.0f / 6.0f)) // Vertical list
				{
					faceSize.x = source->getWidth();
					faceSize.y = source->getHeight() / 6;
					vertical = true;
				}
				else if(Math::approxEquals(aspect, 4.0f / 3.0f)) // Horizontal cross
				{
					faceSize.x = source->getWidth() / 4;
					faceSize.y = source->getHeight() / 3;
					cross = true;
				}
				else if(Math::approxEquals(aspect, 3.0f / 4.0f)) // Vertical cross
				{
					faceSize.x = source->getWidth() / 3;
					faceSize.y = source->getHeight() / 4;
					cross = true;
					vertical = true;
				}
				else
				{
					BS_LOG(Warning, FreeImageImporter, "Unable to generate a cubemap: unrecognized face configuration.");
					return false;
				}
			}
			break;
		case CubemapSourceType::Cylindrical:
		case CubemapSourceType::Spherical:
			// Half of the source size will be used for each cube face, which should yield good enough quality
			faceSize.x = std::max(source->getWidth(), source->getHeight()) / 2;
			faceSize.x = Bitwise::closestPow2(faceSize.x);

			// Don't allow sizes larger than 4096
			faceSize.x = std::min(faceSize.x, 4096);

			faceSize.y = faceSize.x;

			break;
		default: // Assuming single-image
			faceSize.x = source->getWidth();
			faceSize.y = source->getHeight();
			break;
		}

		if (faceSize.x != faceSize.y)
		{
			BS_LOG(Warning, FreeImageImporter, "Unable to generate a cubemap: width & height must match.");
			return false;
		}

		if (!Bitwise::isPow2(faceSize.x))
		{
			BS_LOG(Warning, FreeImageImporter, "Unable to generate a cubemap: width & height must be powers of 2.");
			return false;
		}

		switch (sourceType)
		{
		case CubemapSourceType::Faces:
		{
			if (cross)
				readCubemapCross(source, output, faceSize.x, vertical);
			else
				readCubemapList(source, output, faceSize.x, vertical);
		}
		break;
		case CubemapSourceType::Cylindrical:
		{			
			UINT32 superSampledFaceSize = faceSize.x * cubemapSupersampling;

			std::array<SPtr<PixelData>, 6> superSampledOutput;
			readCubemapUVRemap(source, superSampledOutput, superSampledFaceSize, &mapCubemapDirToCylindrical);

			if (faceSize.x != (INT32)superSampledFaceSize)
				downsampleCubemap(superSampledOutput, output, faceSize.x);
			else
				output = superSampledOutput;
		}
		break;
		case CubemapSourceType::Spherical:
		{
			UINT32 superSampledFaceSize = faceSize.x * cubemapSupersampling;

			std::array<SPtr<PixelData>, 6> superSampledOutput;
			readCubemapUVRemap(source, superSampledOutput, superSampledFaceSize, &mapCubemapDirToSpherical);

			if (faceSize.x != (INT32)superSampledFaceSize)
				downsampleCubemap(superSampledOutput, output, faceSize.x);
			else
				output = superSampledOutput;
		}
		break;
		default: // Single-image
			for (UINT32 i = 0; i < 6; i++)
				output[i] = source;

			break;
		}

		return true;
	}
}
