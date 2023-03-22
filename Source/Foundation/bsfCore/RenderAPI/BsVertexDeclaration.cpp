//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexDeclaration.h"

#include "BsGpuDeviceCapabilities.h"
#include "Private/RTTI/BsVertexDeclarationRTTI.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

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

	B3D_EXCEPT(InvalidParametersException, "Invalid type");
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
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
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

size_t VertexElement::CalculateHash(const VertexElement& element)
{
	size_t hash = 0;
	B3DCombineHash(hash, element.mType);
	B3DCombineHash(hash, element.mIndex);
	B3DCombineHash(hash, element.mOffset);
	B3DCombineHash(hash, element.mSemantic);
	B3DCombineHash(hash, element.mStreamIndex);
	B3DCombineHash(hash, element.mInstanceStepRate);

	return hash;
}

VertexDeclarationProperties::VertexDeclarationProperties(const Vector<VertexElement>& elements)
{
	for(auto& elem : elements)
	{
		VertexElementType type = elem.GetType();

		if(elem.GetType() == VET_COLOR)
			type = VertexElement::GetBestColorVertexElementType();

		mElementList.push_back(VertexElement(type, elem.GetSemantic(), elem.GetSemanticIndex(), elem.GetStreamIndex(), elem.GetInstanceStepRate(), elem.GetOffset()));
	}
}

bool VertexDeclarationProperties::operator==(const VertexDeclarationProperties& rhs) const
{
	if(mElementList.size() != rhs.mElementList.size())
		return false;

	auto myIter = mElementList.begin();
	auto theirIter = rhs.mElementList.begin();

	for(; myIter != mElementList.end() && theirIter != rhs.mElementList.end(); ++myIter, ++theirIter)
	{
		if(!(*myIter == *theirIter))
			return false;
	}

	return true;
}

bool VertexDeclarationProperties::operator!=(const VertexDeclarationProperties& rhs) const
{
	return !(*this == rhs);
}

const VertexElement* VertexDeclarationProperties::GetElement(u16 index) const
{
	B3D_ASSERT(index < mElementList.size() && "Index out of bounds");

	auto iter = mElementList.begin();
	for(u16 i = 0; i < index; ++i)
		++iter;

	return &(*iter);
}

const VertexElement* VertexDeclarationProperties::FindElementBySemantic(VertexElementSemantic sem, u16 index) const
{
	for(auto& elem : mElementList)
	{
		if(elem.GetSemantic() == sem && elem.GetSemanticIndex() == index)
		{
			return &elem;
		}
	}

	return nullptr;
}

Vector<VertexElement> VertexDeclarationProperties::FindElementsBySource(u16 source) const
{
	Vector<VertexElement> retList;
	for(auto& elem : mElementList)
	{
		if(elem.GetStreamIndex() == source)
			retList.push_back(elem);
	}

	return retList;
}

u32 VertexDeclarationProperties::GetVertexSize(u16 source) const
{
	u32 size = 0;

	for(auto& elem : mElementList)
	{
		if(elem.GetStreamIndex() == source)
		{
			size += elem.GetSize();
		}
	}

	return size;
}

VertexDeclaration::VertexDeclaration(const Vector<VertexElement>& elements)
	: mProperties(elements)
{
}

SPtr<ct::VertexDeclaration> VertexDeclaration::GetCore() const
{
	return std::static_pointer_cast<ct::VertexDeclaration>(mCoreSpecific);
}

SPtr<ct::CoreObject> VertexDeclaration::CreateCore() const
{
	return ct::HardwareBufferManager::Instance().CreateVertexDeclarationInternal(mProperties.mElementList);
}

SPtr<VertexDeclaration> VertexDeclaration::Create(const SPtr<VertexDataDesc>& desc)
{
	return HardwareBufferManager::Instance().CreateVertexDeclaration(desc);
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* VertexDeclaration::GetRttiStatic()
{
	return VertexDeclarationRTTI::Instance();
}

RTTITypeBase* VertexDeclaration::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs
{
	String ToString(const VertexElementSemantic& val)
	{
		switch(val)
		{
		case VES_POSITION:
			return "POSITION";
		case VES_BLEND_WEIGHTS:
			return "BLEND_WEIGHTS";
		case VES_BLEND_INDICES:
			return "BLEND_INDICES";
		case VES_NORMAL:
			return "NORMAL";
		case VES_COLOR:
			return "COLOR";
		case VES_TEXCOORD:
			return "TEXCOORD";
		case VES_BITANGENT:
			return "BITANGENT";
		case VES_TANGENT:
			return "TANGENT";
		case VES_POSITIONT:
			return "POSITIONT";
		case VES_PSIZE:
			return "PSIZE";
		}

		return "";
	}
}

namespace bs { namespace ct
{
u32 VertexDeclaration::NextFreeId = 0;

VertexDeclaration::VertexDeclaration(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask)
	: mProperties(elements)
{
}

VertexDeclaration::VertexDeclaration(const SmallVector<VertexElement, 8>& elements)
	: mProperties(Vector<VertexElement>(elements.begin(), elements.end()))
{
}

void VertexDeclaration::Initialize()
{
	mId = NextFreeId++;
	CoreObject::Initialize();
}

SPtr<VertexDeclaration> VertexDeclaration::Create(const SPtr<VertexDataDesc>& desc, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateVertexDeclaration(desc, deviceMask);
}

bool VertexDeclaration::IsCompatible(const SPtr<VertexDeclaration>& shaderDecl)
{
	const Vector<VertexElement>& shaderElems = shaderDecl->GetProperties().GetElements();
	const Vector<VertexElement>& bufferElems = GetProperties().GetElements();

	for(auto shaderIter = shaderElems.begin(); shaderIter != shaderElems.end(); ++shaderIter)
	{
		const VertexElement* foundElement = nullptr;
		for(auto bufferIter = bufferElems.begin(); bufferIter != bufferElems.end(); ++bufferIter)
		{
			if(shaderIter->GetSemantic() == bufferIter->GetSemantic() && shaderIter->GetSemanticIndex() == bufferIter->GetSemanticIndex())
			{
				foundElement = &(*bufferIter);
				break;
			}
		}

		if(foundElement == nullptr)
			return false;
	}

	return true;
}

Vector<VertexElement> VertexDeclaration::GetMissingElements(const SPtr<VertexDeclaration>& shaderDecl)
{
	Vector<VertexElement> missingElements;

	const Vector<VertexElement>& shaderElems = shaderDecl->GetProperties().GetElements();
	const Vector<VertexElement>& bufferElems = GetProperties().GetElements();

	for(auto shaderIter = shaderElems.begin(); shaderIter != shaderElems.end(); ++shaderIter)
	{
		const VertexElement* foundElement = nullptr;
		for(auto bufferIter = bufferElems.begin(); bufferIter != bufferElems.end(); ++bufferIter)
		{
			if(shaderIter->GetSemantic() == bufferIter->GetSemantic() && shaderIter->GetSemanticIndex() == bufferIter->GetSemanticIndex())
			{
				foundElement = &(*bufferIter);
				break;
			}
		}

		if(foundElement == nullptr)
			missingElements.push_back(*shaderIter);
	}

	return missingElements;
}
}}
