//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DVertexDescription.h"

#include "B3DGpuBackend.h"
#include "B3DGpuDevice.h"
#include "B3DGpuDeviceCapabilities.h"
#include "Image/B3DColor.h"
#include "RTTI/B3DVertexDescriptionRTTI.h"

using namespace b3d;

VertexElement::VertexElement(VertexElementType type, VertexElementSemantic semantic, u16 semanticIndex, u32 streamIndex, u32 instanceStepRate, u32 offset)
	: mStreamIndex(streamIndex), mOffset(offset), mType(type), mSemantic(semantic), mIndex(semanticIndex), mInstanceStepRate(instanceStepRate)
{
}

u32 VertexElement::GetSize(void) const
{
	return GetSizeForType(mType);
}

u32 VertexElement::GetSizeForType(VertexElementType type)
{
	switch(type)
	{
	case VET_COLOR:
	case VET_COLOR_ABGR:
	case VET_COLOR_ARGB:
		return sizeof(RGBA);
	case VET_UBYTE4_NORM:
		return sizeof(u32);
	case VET_FLOAT1:
		return sizeof(float);
	case VET_FLOAT2:
		return sizeof(float) * 2;
	case VET_FLOAT3:
		return sizeof(float) * 3;
	case VET_FLOAT4:
		return sizeof(float) * 4;
	case VET_USHORT1:
		return sizeof(u16);
	case VET_USHORT2:
		return sizeof(u16) * 2;
	case VET_USHORT4:
		return sizeof(u16) * 4;
	case VET_SHORT1:
		return sizeof(i16);
	case VET_SHORT2:
		return sizeof(i16) * 2;
	case VET_SHORT4:
		return sizeof(i16) * 4;
	case VET_UINT1:
		return sizeof(u32);
	case VET_UINT2:
		return sizeof(u32) * 2;
	case VET_UINT3:
		return sizeof(u32) * 3;
	case VET_UINT4:
		return sizeof(u32) * 4;
	case VET_INT4:
		return sizeof(i32) * 4;
	case VET_INT1:
		return sizeof(i32);
	case VET_INT2:
		return sizeof(i32) * 2;
	case VET_INT3:
		return sizeof(i32) * 3;
	case VET_UBYTE4:
		return sizeof(u8) * 4;
	default:
		break;
	}

	return 0;
}

unsigned short VertexElement::GetComponentCountForType(VertexElementType type)
{
	switch(type)
	{
	case VET_COLOR:
	case VET_COLOR_ABGR:
	case VET_COLOR_ARGB:
		return 4;
	case VET_FLOAT1:
	case VET_SHORT1:
	case VET_USHORT1:
	case VET_INT1:
	case VET_UINT1:
		return 1;
	case VET_FLOAT2:
	case VET_SHORT2:
	case VET_USHORT2:
	case VET_INT2:
	case VET_UINT2:
		return 2;
	case VET_FLOAT3:
	case VET_INT3:
	case VET_UINT3:
		return 3;
	case VET_FLOAT4:
	case VET_SHORT4:
	case VET_USHORT4:
	case VET_INT4:
	case VET_UINT4:
	case VET_UBYTE4:
	case VET_UBYTE4_NORM:
		return 4;
	default:
		break;
	}

	B3D_ENSURE_LOG(false, "Invalid type");
	return 0;
}

VertexElementType VertexElement::GetBestColorVertexElementType()
{
	// Use the current render system to determine if possible
	if(GpuBackend::InstancePtr() != nullptr && GpuBackend::Instance().GetDevice(0) != nullptr)
	{
		return GpuBackend::Instance().GetDevice(0)->GetCapabilities().VertexColorType;
	}
	else
	{
		// We can't know the specific type right now, so pick a type based on platform
#if B3D_PLATFORM_WIN32
		return VET_COLOR_ARGB; // prefer D3D format on Windows
#else
		return VET_COLOR_ABGR; // prefer GL format on everything else
#endif
	}
}

bool VertexElement::operator==(const VertexElement& rhs) const
{
	if(mType != rhs.mType || mIndex != rhs.mIndex || mOffset != rhs.mOffset ||
	   mSemantic != rhs.mSemantic || mStreamIndex != rhs.mStreamIndex || mInstanceStepRate != rhs.mInstanceStepRate)
	{
		return false;
	}
	else
		return true;
}

bool VertexElement::operator!=(const VertexElement& rhs) const
{
	return !(*this == rhs);
}

static Mutex gVertexDescriptionCacheMutex;
static UnorderedMap<VertexDescription, u32> gVertexDescriptionCache;
static u32 gNextVertexDescriptionId = 1;

VertexDescription::VertexDescription(const TArrayView<VertexElement>& elements, bool calculateOffsets)
	: mVertexElements(TInlineArray<VertexElement, 8>(elements.begin(), elements.end()))
{
	// Sort by stream, but preserve remaining ordering
	std::stable_sort(mVertexElements.begin(), mVertexElements.end(), [](const VertexElement& lhs, const VertexElement& rhs)
	{
		return lhs.GetStreamIndex() < rhs.GetStreamIndex();
	});

	if(calculateOffsets)
		CalculateOffsets();

	Lock lock(gVertexDescriptionCacheMutex);
	auto found = gVertexDescriptionCache.find(*this);
	if(found != gVertexDescriptionCache.end())
		mId = found->second;
	else
	{
		mId = gNextVertexDescriptionId++;
		gVertexDescriptionCache[*this] = mId;
	}
}

bool VertexDescription::operator==(const VertexDescription& rhs) const
{
	const TInlineArray<VertexElement, 8>& otherElements = rhs.GetElements();

	if(mVertexElements.size() != otherElements.size())
		return false;

	for(u32 elementIndex = 0; elementIndex < mVertexElements.size(); ++elementIndex)
	{
		if(mVertexElements[elementIndex] != otherElements[elementIndex])
			return false;
	}

	return true;
}

void VertexDescription::CalculateOffsets()
{
	const u32 largestStreamIndex = GetLargestStreamIndex();
	const u32 streamCount = largestStreamIndex + 1;

	u32* streamOffsets = B3DStackAllocate<u32>(streamCount);
	B3DZeroOut(streamOffsets, streamCount);

	for(auto& element : mVertexElements)
	{
		const u32 streamIndex = element.GetStreamIndex();

		element.mOffset = streamOffsets[streamIndex];
		streamOffsets[streamIndex] += element.GetSize();
	}

	B3DStackFree(streamOffsets);
}

u32 VertexDescription::GetLargestStreamIndex() const
{
	u32 maxStreamIdx = 0;
	u32 numElems = (u32)mVertexElements.size();
	for(u32 i = 0; i < numElems; i++)
	{
		for(auto& vertElem : mVertexElements)
		{
			maxStreamIdx = std::max((u32)maxStreamIdx, (u32)vertElem.GetStreamIndex());
		}
	}

	return maxStreamIdx;
}

bool VertexDescription::HasStream(u32 streamIndex) const
{
	for(auto& vertElem : mVertexElements)
	{
		if(vertElem.GetStreamIndex() == streamIndex)
			return true;
	}

	return false;
}

bool VertexDescription::HasElement(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIndex, streamIndex](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIndex() == semanticIndex && x.GetStreamIndex() == streamIndex; });

	if(findIter != mVertexElements.end())
	{
		return true;
	}

	return false;
}

u32 VertexDescription::GetElementSize(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	for(auto& element : mVertexElements)
	{
		if(element.GetSemantic() == semantic && element.GetSemanticIndex() == semanticIndex && element.GetStreamIndex() == streamIndex)
			return element.GetSize();
	}

	return -1;
}

u32 VertexDescription::GetElementOffsetFromStream(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	u32 vertexOffset = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIndex() != streamIndex)
			continue;

		if(element.GetSemantic() == semantic && element.GetSemanticIndex() == semanticIndex)
			break;

		vertexOffset += element.GetSize();
	}

	return vertexOffset;
}

u32 VertexDescription::GetVertexStride(u32 streamIndex) const
{
	u32 vertexStride = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIndex() == streamIndex)
			vertexStride += element.GetSize();
	}

	return vertexStride;
}

u32 VertexDescription::GetVertexStride() const
{
	u32 vertexStride = 0;
	for(auto& element : mVertexElements)
	{
		vertexStride += element.GetSize();
	}

	return vertexStride;
}

u32 VertexDescription::GetStreamOffset(u32 streamIndex) const
{
	u32 streamOffset = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIndex() == streamIndex)
			break;

		streamOffset += element.GetSize();
	}

	return streamOffset;
}

const VertexElement* VertexDescription::GetElement(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIndex, streamIndex](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIndex() == semanticIndex && x.GetStreamIndex() == streamIndex; });

	if(findIter != mVertexElements.end())
		return &(*findIter);

	return nullptr;
}

bool VertexDescription::IsCompatibleWithShaderInputs(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription)
{
	const TInlineArray<VertexElement, 8>& vertexBufferElements = vertexBufferDescription.GetElements();
	const TInlineArray<VertexElement, 8>& shaderInputElements = shaderInputDescription.GetElements();

	for(const auto& shaderInputElement : shaderInputElements)
	{
		bool isElementFound = false;
		for(const auto& vertexBufferElement : vertexBufferElements)
		{
			if(shaderInputElement.GetSemantic() == vertexBufferElement.GetSemantic() && shaderInputElement.GetSemanticIndex() == vertexBufferElement.GetSemanticIndex())
			{
				isElementFound = true;
				break;
			}
		}

		if(!isElementFound)
			return false;
	}

	return true;
}

TInlineArray<VertexElement, 8> VertexDescription::GetMissingElementsForShaderInput(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription)
{
	TInlineArray<VertexElement, 8> missingElements;

	const TInlineArray<VertexElement, 8>& vertexBufferElements = vertexBufferDescription.GetElements();
	const TInlineArray<VertexElement, 8>& shaderInputElements = shaderInputDescription.GetElements();

	for(const auto& shaderInputElement : shaderInputElements)
	{
		bool isElementFound = false;
		for(const auto& vertexBufferElement : vertexBufferElements)
		{
			if(shaderInputElement.GetSemantic() == vertexBufferElement.GetSemantic() && shaderInputElement.GetSemanticIndex() == vertexBufferElement.GetSemanticIndex())
			{
				isElementFound = true;
				break;
			}
		}

		if(!isElementFound)
			missingElements.Add(shaderInputElement);
	}

	return missingElements;
}
/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* VertexDescription::GetRttiStatic()
{
	return VertexDescriptionRTTI::Instance();
}

RTTIType* VertexDescription::GetRtti() const
{
	return VertexDescription::GetRttiStatic();
}
