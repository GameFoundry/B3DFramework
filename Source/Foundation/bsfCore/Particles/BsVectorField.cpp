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
	template class TVectorField<false>;
	template class TVectorField<true>;
	} // namespace detail
}

using namespace bs;

VectorField::VectorField(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values)
	: TVectorField(desc)
{
	if(mDesc.CountX == 0 || mDesc.CountY == 0 || mDesc.CountZ == 0)
		BS_LOG(Warning, Particles, "Vector field count cannot be zero.");

	mDesc.CountX = std::max(1U, mDesc.CountX);
	mDesc.CountY = std::max(1U, mDesc.CountY);
	mDesc.CountZ = std::max(1U, mDesc.CountZ);

	const u32 count = mDesc.CountX * mDesc.CountY * mDesc.CountZ;
	if(count != (u32)values.size())
	{
		BS_LOG(Warning, Particles, "Number of values provided to the vector field does not match the expected number. "
								   "Expected: {0}. Got: {1}.",
			   count, (u32)values.size());
	}

	const u32 valuesToCopy = std::min(count, (u32)values.size());

	const SPtr<PixelData> pixelData = PixelData::Create(mDesc.CountX, mDesc.CountY, mDesc.CountZ, PF_RGBA16F);

	const u32 pixelSize = PixelUtil::GetNumElemBytes(PF_RGBA16F);
	u8* data = pixelData->GetData();
	for(u32 z = 0; z < (u32)mDesc.CountZ; z++)
	{
		const u32 zArrayIdx = z * mDesc.CountY * mDesc.CountX;
		const u32 zDataIdx = z * pixelData->GetSlicePitch();

		for(u32 y = 0; y < (u32)mDesc.CountY; y++)
		{
			const u32 yArrayIdx = y * mDesc.CountX;
			const u32 yDataIdx = y * pixelData->GetRowPitch();

			for(u32 x = 0; x < (u32)mDesc.CountX; x++)
			{
				const u32 arrayIdx = x + yArrayIdx + zArrayIdx;
				const u32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

				const Vector3& source = arrayIdx < valuesToCopy ? values[arrayIdx] : Vector3::kZero;
				u8* dest = data + dataIdx;
				PixelUtil::PackColor(source.X, source.Y, source.Z, 1.0f, PF_RGBA16F, dest);
			}
		}
	}

	mTexture = Texture::CreatePtrInternal(pixelData);
}

SPtr<ct::CoreObject> VectorField::CreateCore() const
{
	ct::VectorField* vectorField = new(bs_alloc<ct::VectorField>()) ct::VectorField(mDesc, mTexture->GetCore());

	SPtr<ct::VectorField> vectorFieldPtr = bs_shared_ptr<ct::VectorField>(vectorField);
	vectorFieldPtr->SetThisPtrInternal(vectorFieldPtr);

	return vectorFieldPtr;
}

SPtr<ct::VectorField> VectorField::GetCore() const
{
	return std::static_pointer_cast<ct::VectorField>(mCoreSpecific);
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* VectorField::GetRttiStatic()
{
	return VectorFieldRTTI::Instance();
}

RTTITypeBase* VectorField::GetRtti() const
{
	return VectorField::GetRttiStatic();
}

/************************************************************************/
/* 								STATICS	                      			*/
/************************************************************************/
HVectorField VectorField::Create(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values)
{
	SPtr<VectorField> vectorFieldPtr = CreatePtrInternal(desc, values);

	return static_resource_cast<VectorField>(gResources().CreateResourceHandleInternal(vectorFieldPtr));
}

SPtr<VectorField> VectorField::CreatePtrInternal(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values)
{
	auto* vectorField = new(bs_alloc<VectorField>()) VectorField(desc, values);

	SPtr<VectorField> vectorFieldPtr = bs_shared_ptr<VectorField>(vectorField);
	vectorFieldPtr->SetThisPtrInternal(vectorFieldPtr);
	vectorFieldPtr->Initialize();

	return vectorFieldPtr;
}

SPtr<VectorField> VectorField::CreateEmptyInternal()
{
	auto* vectorField = new(bs_alloc<VectorField>()) VectorField();

	SPtr<VectorField> vectorFieldPtr = bs_shared_ptr<VectorField>(vectorField);
	vectorFieldPtr->SetThisPtrInternal(vectorFieldPtr);

	return vectorFieldPtr;
}

namespace bs { namespace ct
{
VectorField::VectorField(const VECTOR_FIELD_DESC& desc, const SPtr<Texture>& texture)
	: TVectorField(desc)
{
	mTexture = texture;
}
}}

bool FGAImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtil::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"fga";
}

bool FGAImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	return true; // Plain-text so we don't even check for magic number
}

SPtr<Resource> FGAImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	String data;
	{
		Lock fileLock = FileScheduler::GetLock(filePath);

		SPtr<DataStream> stream = FileSystem::OpenFile(filePath);
		data = stream->GetAsString();
	}

	auto chars = bs_managed_stack_alloc<char>((u32)data.size() + 1);
	memcpy(chars, data.data(), data.size());
	chars[data.size()] = '\0';

	const auto parseInt = [](char* input, i32& output)
	{
		char* start = input;
		while(*input != '\0')
		{
			if(*input == ',')
			{
				*input = '\0';
				output = (i32)atoi(start);

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
	readPos = parseInt(readPos, size.X);
	readPos = parseInt(readPos, size.Y);
	readPos = parseInt(readPos, size.Z);

	if(size.X < 0 || size.Y < 0 || size.Z < 0)
	{
		BS_LOG(Error, Particles, "Invalid dimensions.");
		return nullptr;
	}

	desc.CountX = (u32)size.X;
	desc.CountY = (u32)size.Y;
	desc.CountZ = (u32)size.Z;

	if(*readPos == '\0')
	{
		BS_LOG(Error, Particles, "Unexpected end of file.");
		return nullptr;
	}

	Vector3 minBounds, maxBounds;
	readPos = parseFloat(readPos, minBounds.X);
	readPos = parseFloat(readPos, minBounds.Y);
	readPos = parseFloat(readPos, minBounds.Z);
	readPos = parseFloat(readPos, maxBounds.X);
	readPos = parseFloat(readPos, maxBounds.Y);
	readPos = parseFloat(readPos, maxBounds.Z);

	if(*readPos == '\0')
	{
		BS_LOG(Error, Particles, "Unexpected end of file.");
		return nullptr;
	}

	desc.Bounds = AABox(minBounds, maxBounds);

	const u32 count = size.X * size.Y * size.Z;
	Vector<Vector3> values;
	values.resize(count);

	for(u32 i = 0; i < count; i++)
	{
		readPos = parseFloat(readPos, values[i].X);
		readPos = parseFloat(readPos, values[i].Y);
		readPos = parseFloat(readPos, values[i].Z);

		if((i != (count - 1)) && *readPos == '\0')
		{
			BS_LOG(Error, Particles, "Unexpected end of file.");
			return nullptr;
		}
	}

	if(*readPos != '\0')
	{
		BS_LOG(Warning, Particles, "Unexpected excess data. This might indicate corrupt data. Remaining data will be truncated.");
	}

	const String fileName = filePath.GetFilename(false);
	SPtr<VectorField> vectorField = VectorField::CreatePtrInternal(desc, values);
	vectorField->SetName(fileName);

	return vectorField;
}
