//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexDeclaration.h"
#include "Private/RTTI/BsVertexDeclarationRTTI.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	VertexElement::VertexElement(UINT16 source, UINT32 offset,
		VertexElementType theType, VertexElementSemantic semantic, UINT16 index, UINT32 instanceStepRate)
		: MSource(source), mOffset(offset), mType(theType), mSemantic(semantic), mIndex(index)
		, MInstanceStepRate(instanceStepRate)
	{
	}

	UINT32 VertexElement::GetSize(void) const
	{
		return GetTypeSize(mType);
	}

	UINT32 VertexElement::GetTypeSize(VertexElementType etype)
	{
		switch(etype)
		{
		case VET_COLOR:
		case VET_COLOR_ABGR:
		case VET_COLOR_ARGB:
			return Sizeof(RGBA);
		case VET_UBYTE4_NORM:
			return Sizeof(UINT32);
		case VET_FLOAT1:
			return Sizeof(float);
		case VET_FLOAT2:
			return Sizeof(float) * 2;
		case VET_FLOAT3:
			return Sizeof(float) * 3;
		case VET_FLOAT4:
			return Sizeof(float) * 4;
		case VET_USHORT1:
			return Sizeof(UINT16);
		case VET_USHORT2:
			return Sizeof(UINT16) * 2;
		case VET_USHORT4:
			return Sizeof(UINT16) * 4;
		case VET_SHORT1:
			return Sizeof(INT16);
		case VET_SHORT2:
			return Sizeof(INT16) * 2;
		case VET_SHORT4:
			return Sizeof(INT16) * 4;
		case VET_UINT1:
			return Sizeof(UINT32);
		case VET_UINT2:
			return Sizeof(UINT32) * 2;
		case VET_UINT3:
			return Sizeof(UINT32) * 3;
		case VET_UINT4:
			return Sizeof(UINT32) * 4;
		case VET_INT4:
			return Sizeof(INT32) * 4;
		case VET_INT1:
			return Sizeof(INT32);
		case VET_INT2:
			return Sizeof(INT32) * 2;
		case VET_INT3:
			return Sizeof(INT32) * 3;
		case VET_UBYTE4:
			return Sizeof(UINT8) * 4;
		default:
			break;
		}

		return 0;
	}

	unsigned short VertexElement::GetTypeCount(VertexElementType etype)
	{
		switch (etype)
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

		BS_EXCEPT(InvalidParametersException, "Invalid type");
		return 0;
	}

	VertexElementType VertexElement::GetBestColorVertexElementType()
	{
		// Use the current render system to determine if possible
		if (ct::RenderAPI::instancePtr() != nullptr)
		{
			return ct::RenderAPI::instance().getCapabilities(0).vertexColorType;
		}
		else
		{
			// We can't know the specific type right now, so pick a type based on platform
#if BS_PLATFORM == BS_PLATFORM_WIN32
			return VET_COLOR_ARGB; // prefer D3D format on Windows
#else
			return VET_COLOR_ABGR; // prefer GL format on everything else
#endif

		}
	}

	bool VertexElement::operator== (const VertexElement& rhs) const
	{
		if (mType != rhs.mType || mIndex != rhs.mIndex || mOffset != rhs.mOffset ||
			mSemantic != rhs.mSemantic || mSource != rhs.mSource  || mInstanceStepRate != rhs.mInstanceStepRate)
		{
			return false;
		}
		else
			return true;
	}

	bool VertexElement::operator!= (const VertexElement& rhs) const
	{
		return !(*this == rhs);
	}

	size_t VertexElement::GetHash(const VertexElement& element)
	{
		size_t hash = 0;
		bs_hash_combine(hash, element.mType);
		bs_hash_combine(hash, element.mIndex);
		bs_hash_combine(hash, element.mOffset);
		bs_hash_combine(hash, element.mSemantic);
		bs_hash_combine(hash, element.mSource);
		bs_hash_combine(hash, element.mInstanceStepRate);

		return hash;
	}

	VertexDeclarationProperties::VertexDeclarationProperties(const Vector<VertexElement>& elements)
	{
		for (auto& elem : elements)
		{
			VertexElementType type = elem.getType();

			if (elem.getType() == VET_COLOR)
				type = VertexElement::getBestColorVertexElementType();

			mElementList.push_back(VertexElement(elem.getStreamIdx(), elem.getOffset(), type, elem.getSemantic(),
				elem.getSemanticIdx(), elem.getInstanceStepRate()));
		}
	}

	bool VertexDeclarationProperties::operator== (const VertexDeclarationProperties& rhs) const
	{
		if (mElementList.size() != rhs.mElementList.size())
			return false;

		auto myIter = mElementList.begin();
		auto theirIter = rhs.mElementList.begin();

		for (; myIter != mElementList.end() && theirIter != rhs.mElementList.end(); ++myIter, ++theirIter)
		{
			if (!(*myIter == *theirIter))
				return false;
		}

		return true;
	}

	bool VertexDeclarationProperties::operator!= (const VertexDeclarationProperties& rhs) const
	{
		return !(*this == rhs);
	}

	const VertexElement* VertexDeclarationProperties::getElement(UINT16 index) const
	{
		assert(index < mElementList.size() && "Index out of bounds");

		auto iter = mElementList.begin();
		for (UINT16 i = 0; i < index; ++i)
			++iter;

		return &(*iter);

	}
	
	const VertexElement* VertexDeclarationProperties::findElementBySemantic(VertexElementSemantic sem, UINT16 index) const
	{
		for (auto& elem : mElementList)
		{
			if (elem.getSemantic() == sem && elem.getSemanticIdx() == index)
			{
				return &elem;
			}
		}

		return nullptr;
	}

	Vector<VertexElement> VertexDeclarationProperties::FindElementsBySource(UINT16 source) const
	{
		Vector<VertexElement> retList;
		for (auto& elem : mElementList)
		{
			if (elem.getStreamIdx() == source)
				retList.push_back(elem);
		}

		return retList;
	}

	UINT32 VertexDeclarationProperties::GetVertexSize(UINT16 source) const
	{
		UINT32 size = 0;

		for (auto& elem : mElementList)
		{
			if (elem.getStreamIdx() == source)
			{
				size += elem.getSize();
			}
		}

		return size;
	}

	VertexDeclaration::VertexDeclaration(const Vector<VertexElement>& elements)
		:mProperties(elements)
	{

	}

	SPtr<ct::VertexDeclaration> VertexDeclaration::GetCore() const
	{
		return std::static_pointer_cast<ct::VertexDeclaration>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> VertexDeclaration::CreateCore() const
	{
		return ct::HardwareBufferManager::instance().createVertexDeclarationInternal(mProperties.mElementList);
	}

	SPtr<VertexDeclaration> VertexDeclaration::Create(const SPtr<VertexDataDesc>& desc)
	{
		return HardwareBufferManager::Instance().createVertexDeclaration(desc);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* VertexDeclaration::getRTTIStatic()
	{
		return VertexDeclarationRTTI::Instance();
	}

	RTTITypeBase* VertexDeclaration::getRTTI() const
	{
		return GetRTTIStatic();
	}

	String ToString(const VertexElementSemantic& val)
	{
		switch (val)
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

	namespace ct
	{
	UINT32 VertexDeclaration::NextFreeId = 0;

	VertexDeclaration::VertexDeclaration(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask)
		:mProperties(elements)
	{
		
	}

	void VertexDeclaration::Initialize()
	{
		mId = NextFreeId++;
		CoreObject::initialize();
	}

	SPtr<VertexDeclaration> VertexDeclaration::Create(const SPtr<VertexDataDesc>& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::Instance().createVertexDeclaration(desc, deviceMask);
	}

	bool VertexDeclaration::IsCompatible(const SPtr<VertexDeclaration>& shaderDecl)
	{
		const Vector<VertexElement>& shaderElems = shaderDecl->getProperties().getElements();
		const Vector<VertexElement>& bufferElems = getProperties().getElements();

		for (auto shaderIter = shaderElems.begin(); shaderIter != shaderElems.end(); ++shaderIter)
		{
			const VertexElement* foundElement = nullptr;
			for (auto bufferIter = bufferElems.begin(); bufferIter != bufferElems.end(); ++bufferIter)
			{
				if (shaderIter->getSemantic() == bufferIter->getSemantic() && shaderIter->getSemanticIdx() == bufferIter->getSemanticIdx())
				{
					foundElement = &(*bufferIter);
					break;
				}
			}

			if (foundElement == nullptr)
				return false;
		}

		return true;
	}

	Vector<VertexElement> VertexDeclaration::GetMissingElements(const SPtr<VertexDeclaration>& shaderDecl)
	{
		Vector<VertexElement> missingElements;

		const Vector<VertexElement>& shaderElems = shaderDecl->getProperties().getElements();
		const Vector<VertexElement>& bufferElems = getProperties().getElements();

		for (auto shaderIter = shaderElems.begin(); shaderIter != shaderElems.end(); ++shaderIter)
		{
			const VertexElement* foundElement = nullptr;
			for (auto bufferIter = bufferElems.begin(); bufferIter != bufferElems.end(); ++bufferIter)
			{
				if (shaderIter->getSemantic() == bufferIter->getSemantic() && shaderIter->getSemanticIdx() == bufferIter->getSemanticIdx())
				{
					foundElement = &(*bufferIter);
					break;
				}
			}

			if (foundElement == nullptr)
				missingElements.push_back(*shaderIter);
		}

		return missingElements;
	}
	}
}
