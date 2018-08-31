//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsVectorField.h"
#include "Private/RTTI/BsVectorFieldRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"

namespace bs 
{
	namespace detail
	{
		template class TVectorField <false>;
		template class TVectorField <true>;
	}

	VectorField::VectorField(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values)
		:TVectorField(desc)
	{
		if(mDesc.countX == 0 || mDesc.countY == 0 || mDesc.countZ == 0)
			LOGWRN("Vector field count cannot be zero.");

		mDesc.countX = std::max(1U, mDesc.countX);
		mDesc.countY = std::max(1U, mDesc.countY);
		mDesc.countZ = std::max(1U, mDesc.countZ);

		const UINT32 count = mDesc.countX * mDesc.countY * mDesc.countZ;
		if(count != (UINT32)values.size())
		{
			LOGWRN(StringUtil::format("Number of values provided to the vector field does not match the expected number. \
				Expected: {0}. Got: {1}.", count, values.size()));
		}

		const UINT32 valuesToCopy = std::min(count, (UINT32)values.size());

		const SPtr<PixelData> pixelData = PixelData::create(mDesc.countX, mDesc.countY, mDesc.countZ, PF_RGBA16F);

		const UINT32 pixelSize = PixelUtil::getNumElemBytes(PF_RGBA16F);
		UINT8* data = pixelData->getData();
		for(UINT32 z = 0; z < (UINT32)mDesc.countZ; z++)
		{
			const UINT32 zArrayIdx = z * mDesc.countY * mDesc.countX;
			const UINT32 zDataIdx = z * pixelData->getSlicePitch() * pixelSize;

			for(UINT32 y = 0; y < (UINT32)mDesc.countY; y++)
			{
				const UINT32 yArrayIdx = y * mDesc.countX;
				const UINT32 yDataIdx = y * pixelData->getRowPitch() * pixelSize;

				for(UINT32 x = 0; x < (UINT32)mDesc.countX; x++)
				{
					const UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					const UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					const Vector3& source = arrayIdx < valuesToCopy ? values[arrayIdx] : Vector3::ZERO;
					UINT8* dest = data + dataIdx;
					PixelUtil::packColor(source.x, source.y, source.z, 1.0f, PF_RGBA16F, dest);
				}
			}
		}

		mTexture = Texture::_createPtr(pixelData);
	}

	SPtr<ct::CoreObject> VectorField::createCore() const
	{
		ct::VectorField* vectorField = new (bs_alloc<ct::VectorField>()) ct::VectorField(mDesc, mTexture->getCore());

		SPtr<ct::VectorField> vectorFieldPtr = bs_shared_ptr<ct::VectorField>(vectorField);
		vectorFieldPtr->_setThisPtr(vectorFieldPtr);

		return vectorFieldPtr;
	}

	SPtr<ct::VectorField> VectorField::getCore() const
	{
		return std::static_pointer_cast<ct::VectorField>(mCoreSpecific);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* VectorField::getRTTIStatic()
	{
		return VectorFieldRTTI::instance();
	}

	RTTITypeBase* VectorField::getRTTI() const
	{
		return VectorField::getRTTIStatic();
	}

	/************************************************************************/
	/* 								STATICS	                      			*/
	/************************************************************************/
	HVectorField VectorField::create(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values)
	{
		SPtr<VectorField> vectorFieldPtr = _createPtr(desc, values);

		return static_resource_cast<VectorField>(gResources()._createResourceHandle(vectorFieldPtr));
	}

	SPtr<VectorField> VectorField::_createPtr(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values)
	{
		auto* vectorField = new (bs_alloc<VectorField>()) VectorField(desc, values);

		SPtr<VectorField> vectorFieldPtr = bs_shared_ptr<VectorField>(vectorField);
		vectorFieldPtr->_setThisPtr(vectorFieldPtr);
		vectorFieldPtr->initialize();

		return vectorFieldPtr;
	}

	SPtr<VectorField> VectorField::_createEmpty()
	{
		auto* vectorField = new (bs_alloc<VectorField>()) VectorField();

		SPtr<VectorField> vectorFieldPtr = bs_shared_ptr<VectorField>(vectorField);
		vectorFieldPtr->_setThisPtr(vectorFieldPtr);

		return vectorFieldPtr;
	}

	namespace ct
	{
		VectorField::VectorField(const VECTOR_FIELD_DESC& desc, const SPtr<Texture>& texture)
			:TVectorField(desc)
		{
			mTexture = texture;
		}
	}

	bool FGAImporter::isExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == u8"fga";
	}

	bool FGAImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // Plain-text so we don't even check for magic number
	}

	SPtr<Resource> FGAImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		String data;
		{
			Lock fileLock = FileScheduler::getLock(filePath);

			SPtr<DataStream> stream = FileSystem::openFile(filePath);
			data = stream->getAsString();
		}

		auto chars = bs_managed_stack_alloc<char>((UINT32)data.size() + 1);
		memcpy(chars, data.data(), data.size());
		chars[data.size()] = '\0';

		const auto parseInt = [](char* input, INT32& output)
		{
			char* start = input;
			while(*input != '\0')
			{
				if(*input == ',')
				{
					*input = '\0';
					output = (INT32)atoi(start);

					return input + 1;
				}

				input++;
			}

			return input;
		};

		const auto parseFloat = [](char* input, float& output)
		{
			char* start = input;
			while(*input != '\0')
			{
				if(*input == ',')
				{
					*input = '\0';
					output = (float)atof(start);

					return input + 1;
				}

				input++;
			}

			return input;
		};

		VECTOR_FIELD_DESC desc;
		char* readPos = chars;

		// Read X, Y, Z sizes
		Vector3I size;
		readPos = parseInt(readPos, size.x);
		readPos = parseInt(readPos, size.y);
		readPos = parseInt(readPos, size.z);

		if(size.x < 0 || size.y < 0 || size.z < 0)
		{
			LOGERR("Invalid dimensions.");
			return nullptr;
		}

		desc.countX = (UINT32)size.x;
		desc.countY = (UINT32)size.y;
		desc.countZ = (UINT32)size.z;
		
		if(*readPos == '\0')
		{
			LOGERR("Unexpected end of file.");
			return nullptr;
		}
		
		Vector3 minBounds, maxBounds;
		readPos = parseFloat(readPos, minBounds.x);
		readPos = parseFloat(readPos, minBounds.y);
		readPos = parseFloat(readPos, minBounds.z);
		readPos = parseFloat(readPos, maxBounds.x);
		readPos = parseFloat(readPos, maxBounds.y);
		readPos = parseFloat(readPos, maxBounds.z);

		if(*readPos == '\0')
		{
			LOGERR("Unexpected end of file.");
			return nullptr;
		}

		desc.bounds = AABox(minBounds, maxBounds);

		const UINT32 count = size.x * size.y * size.z;
		Vector<Vector3> values;
		values.resize(count);

		for(UINT32 i = 0; i < count; i++)
		{
			readPos = parseFloat(readPos, values[i].x);
			readPos = parseFloat(readPos, values[i].y);
			readPos = parseFloat(readPos, values[i].z);

			if ((i != (count - 1)) && *readPos == '\0')
			{
				LOGERR("Unexpected end of file.");
				return nullptr;
			}
		}

		if(*readPos != '\0')
		{
			LOGWRN("Unexpected excess data. This might indicate corrupt data. Remaining data will be truncated.");
		}

		const String fileName = filePath.getFilename(false);
		SPtr<VectorField> vectorField = VectorField::_createPtr(desc, values);
		vectorField->setName(fileName);

		return vectorField;
	}
}
