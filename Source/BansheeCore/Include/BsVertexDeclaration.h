//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsColor.h"
#include "BsCoreObject.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Semantics that are used for identifying the meaning of vertex buffer elements. */
	enum VertexElementSemantic 
	{
		VES_POSITION = 1, /**< Position */
		VES_BLEND_WEIGHTS = 2, /**< Blend weights */
        VES_BLEND_INDICES = 3, /**< Blend indices */
		VES_NORMAL = 4, /**< Normal */
		VES_COLOR = 5, /**< Color */
		VES_TEXCOORD = 6, /**< UV coordinate */
        VES_BITANGENT = 7, /**< Bitangent */
        VES_TANGENT = 8, /**< Tangent */
		VES_POSITIONT = 9, /**< Transformed position */
		VES_PSIZE = 10 /**< Point size */
	};

	/**	Types used to identify base types of vertex element contents. */
    enum VertexElementType
    {
        VET_FLOAT1 = 0, /**< 1D floating point value */
        VET_FLOAT2 = 1, /**< 2D floating point value */
        VET_FLOAT3 = 2, /**< 3D floating point value */
        VET_FLOAT4 = 3, /**< 4D floating point value */
		VET_COLOR = 4, /**< Color encoded in 32-bits (8-bits per channel). */
		VET_SHORT1 = 5, /**< 1D 16-bit signed integer value */
		VET_SHORT2 = 6, /**< 2D 16-bit signed integer value */
		VET_SHORT4 = 8, /**< 4D 16-bit signed integer value */
        VET_UBYTE4 = 9, /**< 4D 8-bit unsigned integer value */
        VET_COLOR_ARGB = 10, /**< Color encoded in 32-bits (8-bits per channel) in ARGB order) */
        VET_COLOR_ABGR = 11, /**< Color encoded in 32-bits (8-bits per channel) in ABGR order) */
		VET_UINT4 = 12, /**< 4D 32-bit unsigned integer value */
		VET_INT4 = 13,  /**< 4D 32-bit signed integer value */
		VET_USHORT1 = 14, /**< 1D 16-bit unsigned integer value */
		VET_USHORT2 = 15, /**< 2D 16-bit unsigned integer value */
		VET_USHORT4 = 17, /**< 4D 16-bit unsigned integer value */
		VET_INT1 = 18,  /**< 1D 32-bit signed integer value */
		VET_INT2 = 19,  /**< 2D 32-bit signed integer value */
		VET_INT3 = 20,  /**< 3D 32-bit signed integer value */
		VET_UINT1 = 21,  /**< 1D 32-bit signed integer value */
		VET_UINT2 = 22,  /**< 2D 32-bit signed integer value */
		VET_UINT3 = 23,  /**< 3D 32-bit signed integer value */
		VET_UBYTE4_NORM = 24, /**< 4D 8-bit unsigned integer interpreted as a normalized value in [0, 1] range. */
		VET_COUNT // Keep at end
    };

	/**	Describes a single vertex element in a vertex declaration. */
	class BS_CORE_EXPORT VertexElement
    {
    public:
		VertexElement() {}
        VertexElement(UINT16 source, UINT32 offset, VertexElementType theType,
			VertexElementSemantic semantic, UINT16 index = 0, UINT32 instanceStepRate = 0);

		bool operator== (const VertexElement& rhs) const;
		bool operator!= (const VertexElement& rhs) const;

		/**	Returns index of the vertex buffer from which this element is stored. */
		UINT16 getStreamIdx() const { return mSource; }

		/**
		 * Returns an offset into the buffer where this vertex is stored. This value might be in bytes but doesn't have to
		 * be, it's likely to be render API specific.
		 */
        UINT32 getOffset() const { return mOffset; }

		/** Gets the base data type of this element. */
        VertexElementType getType() const { return mType; }

		/**	Gets a semantic that describes what this element contains. */
        VertexElementSemantic getSemantic() const { return mSemantic; }

		/**
		 * Gets an index of this element. Only relevant when you have multiple elements with the same semantic, 
		 * for example uv0, uv1.
		 */
		UINT16 getSemanticIdx() const { return mIndex; }

		/** Returns the size of this element in bytes. */
		UINT32 getSize() const;

		/** 
		 * Returns at what rate do the vertex elements advance during instanced rendering. Provide zero for default 
		 * behaviour where each vertex receives the next value from the vertex buffer. Provide a value larger than zero
		 * to ensure vertex data is advanced with every instance, instead of every vertex (for example a value of 1 means
		 * each instance will retrieve a new value from the vertex buffer, a value of 2 means each second instance will, 
		 * etc.).
		 */
		UINT32 getInstanceStepRate() const { return mInstanceStepRate; }

		/**	Returns the size of a base element type. */
		static UINT32 getTypeSize(VertexElementType etype);

		/** Returns the number of values in the provided base element type. For example float4 has four values. */
		static UINT16 getTypeCount(VertexElementType etype);

		/**	Gets packed color vertex element type used by the active render system. */
		static VertexElementType getBestColorVertexElementType();

	protected:
		UINT16 mSource;
		UINT32 mOffset;
		VertexElementType mType;
		VertexElementSemantic mSemantic;
		UINT16 mIndex;
		UINT32 mInstanceStepRate;
    };

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(VertexElement);
	/** @endcond */

	/**	Contains information about a vertex declaration. */
	class BS_CORE_EXPORT VertexDeclarationProperties
	{
	public:
		VertexDeclarationProperties(const List<VertexElement>& elements);

		bool operator== (const VertexDeclarationProperties& rhs) const;
		bool operator!= (const VertexDeclarationProperties& rhs) const;

		/**	Get the number of elements in the declaration. */
        UINT32 getElementCount() const { return (UINT32)mElementList.size(); }
        
		/**	Returns a list of vertex elements in the declaration. */
		const List<VertexElement>& getElements() const { return mElementList; }

		/**	Returns a single vertex element at the specified index. */
		const VertexElement* getElement(UINT16 index) const;

		/**
		 * Attempts to find an element by the given semantic and semantic index. If no element can be found null is returned.
		 */
		virtual const VertexElement* findElementBySemantic(VertexElementSemantic sem, UINT16 index = 0) const;

		/**	Returns a list of all elements that use the provided source index. */
		virtual List<VertexElement> findElementsBySource(UINT16 source) const;

		/**	Returns the total size of all vertex elements using the provided source index. */
		virtual UINT32 getVertexSize(UINT16 source) const;

	protected:
		friend class VertexDeclaration;
		friend class VertexDeclarationRTTI;

		List<VertexElement> mElementList;
	};

	/**
	 * Describes a set of vertex elements, used for describing contents of a vertex buffer or inputs to a vertex GPU program.
	 *
	 * @note	Sim thread.
	 */
	class BS_CORE_EXPORT VertexDeclaration : public IReflectable, public CoreObject
    {
	public:
		virtual ~VertexDeclaration() { }

		/** Returns properties describing the vertex declaration. */
		const VertexDeclarationProperties& getProperties() const { return mProperties; }

		/**	Retrieves a core implementation of a vertex declaration usable only from the core thread. */
		SPtr<VertexDeclarationCore> getCore() const;

		/** @copydoc HardwareBufferManager::createVertexDeclaration */
		static SPtr<VertexDeclaration> create(const SPtr<VertexDataDesc>& desc);

    protected:
		friend class HardwareBufferManager;

		VertexDeclaration(const List<VertexElement>& elements);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

	protected:
		VertexDeclarationProperties mProperties;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class VertexDeclarationRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
    };

	/**	Converts a vertex semantic enum to a readable name. */
	BS_CORE_EXPORT String toString(const VertexElementSemantic& val);

	/**	Converts a vertex semantic enum to a readable name. */
	BS_CORE_EXPORT WString toWString(const VertexElementSemantic& val);

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread portion of a VertexDeclaration.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT VertexDeclarationCore : public CoreObjectCore
    {
	public:
		virtual ~VertexDeclarationCore() { }

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Returns properties describing the vertex declaration. */
		const VertexDeclarationProperties& getProperties() const { return mProperties; }

		/**	Returns an ID unique to this declaration. */
		UINT32 getId() const { return mId; }

		/**
		 * Checks can a vertex buffer declared with this declaration be bound to a shader defined with the provided 
		 * declaration.
		 */
		bool isCompatible(const SPtr<VertexDeclarationCore>& shaderDecl);

		/**
		 * Returns a list of vertex elements that the provided shader's vertex declaration expects but aren't present in 
		 * this vertex declaration.
		 */
		Vector<VertexElement> getMissingElements(const SPtr<VertexDeclarationCore>& shaderDecl);

		/** @copydoc HardwareBufferCoreManager::createVertexDeclaration */
		static SPtr<VertexDeclarationCore> create(const SPtr<VertexDataDesc>& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

    protected:
		friend class HardwareBufferCoreManager;

		VertexDeclarationCore(const List<VertexElement>& elements, GpuDeviceFlags deviceMask);

		VertexDeclarationProperties mProperties;
		UINT32 mId;

		static UINT32 NextFreeId;
    };

	/** @} */
}