//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
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
		VET_FLOAT1 = 0, /**< 1D 32-bit floating point value */
		VET_FLOAT2 = 1, /**< 2D 32-bit floating point value */
		VET_FLOAT3 = 2, /**< 3D 32-bit floating point value */
		VET_FLOAT4 = 3, /**< 4D 32-bit floating point value */
		VET_COLOR = 4, /**< Color encoded in 32-bits (8-bits per channel). */
		VET_SHORT1 = 5, /**< 1D 16-bit signed integer value */
		VET_SHORT2 = 6, /**< 2D 16-bit signed integer value */
		VET_SHORT4 = 8, /**< 4D 16-bit signed integer value */
		VET_UBYTE4 = 9, /**< 4D 8-bit unsigned integer value */
		VET_COLOR_ARGB = 10, /**< Color encoded in 32-bits (8-bits per channel) in ARGB order) */
		VET_COLOR_ABGR = 11, /**< Color encoded in 32-bits (8-bits per channel) in ABGR order) */
		VET_UINT4 = 12, /**< 4D 32-bit unsigned integer value */
		VET_INT4 = 13, /**< 4D 32-bit signed integer value */
		VET_USHORT1 = 14, /**< 1D 16-bit unsigned integer value */
		VET_USHORT2 = 15, /**< 2D 16-bit unsigned integer value */
		VET_USHORT4 = 17, /**< 4D 16-bit unsigned integer value */
		VET_INT1 = 18, /**< 1D 32-bit signed integer value */
		VET_INT2 = 19, /**< 2D 32-bit signed integer value */
		VET_INT3 = 20, /**< 3D 32-bit signed integer value */
		VET_UINT1 = 21, /**< 1D 32-bit signed integer value */
		VET_UINT2 = 22, /**< 2D 32-bit signed integer value */
		VET_UINT3 = 23, /**< 3D 32-bit signed integer value */
		VET_UBYTE4_NORM = 24, /**< 4D 8-bit unsigned integer interpreted as a normalized value in [0, 1] range. */
		VET_HALF1 = 25, /**< 1D 16-bit floating point value */
		VET_HALF2 = 26, /**< 2D 16-bit floating point value */
		VET_HALF3 = 27, /**< 3D 16-bit floating point value */
		VET_HALF4 = 28, /**< 3D 16-bit floating point value */
		VET_COUNT, // Keep at end before VET_UNKNOWN
		VET_UNKNOWN = 0xffff
	};

	/**	Describes a single vertex element in a vertex declaration. */
	class B3D_EXPORT VertexElement
	{
	public:
		VertexElement() = default;
		VertexElement(VertexElementType type, VertexElementSemantic semantic, u16 semanticIndex = 0, u32 streamIndex = 0, u32 instanceStepRate = 0, u32 offset = 0);

		bool operator==(const VertexElement& rhs) const;
		bool operator!=(const VertexElement& rhs) const;

		/**	Returns index of the vertex buffer from which this element is stored. */
		u16 GetStreamIndex() const { return mStreamIndex; }

		/**
		 * Returns an offset into the buffer where this vertex is stored. This value might be in bytes but doesn't have to
		 * be, it's likely to be render API specific.
		 */
		u32 GetOffset() const { return mOffset; }

		/** Gets the base data type of this element. */
		VertexElementType GetType() const { return mType; }

		/**	Gets a semantic that describes what this element contains. */
		VertexElementSemantic GetSemantic() const { return mSemantic; }

		/**
		 * Gets an index of this element. Only relevant when you have multiple elements with the same semantic,
		 * for example uv0, uv1.
		 */
		u16 GetSemanticIndex() const { return mIndex; }

		/** Returns the size of this element in bytes. */
		u32 GetSize() const;

		/**
		 * Returns at what rate do the vertex elements advance during instanced rendering. Provide zero for default
		 * behaviour where each vertex receives the next value from the vertex buffer. Provide a value larger than zero
		 * to ensure vertex data is advanced with every instance, instead of every vertex (for example a value of 1 means
		 * each instance will retrieve a new value from the vertex buffer, a value of 2 means each second instance will,
		 * etc.).
		 */
		u32 GetInstanceStepRate() const { return mInstanceStepRate; }

		/**	Returns the size of a base element type. */
		static u32 GetSizeForType(VertexElementType type);

		/** Returns the number of values in the provided base element type. For example float4 has four values. */
		static u16 GetComponentCountForType(VertexElementType type);

		/**	Gets packed color vertex element type used by the active render system. */
		static VertexElementType GetBestColorVertexElementType();

	protected:
		friend class VertexDescription;

		u16 mStreamIndex;
		u32 mOffset;
		VertexElementType mType;
		VertexElementSemantic mSemantic;
		u16 mIndex;
		u32 mInstanceStepRate;
	};

	/**
	 * Contains information about layout of vertices in a buffer.
	 *
	 * @note Thread safe (Immutable).
	 */
	class B3D_EXPORT VertexDescription : public IReflectable
	{
	public:
		VertexDescription(const TArrayView<VertexElement>& elements, bool calculateOffsets = true);

		bool operator==(const VertexDescription& rhs) const;
		bool operator!=(const VertexDescription& rhs) const { return !operator==(rhs); }

		/** Returns a unique identifier of this combination of vertex elements. All vertex descriptions sharing the same set of vertex elements will have the same ID. */
		u32 GetId() const { return mId; }

		/**	Query if we have vertex data for the specified semantic. */
		bool HasElement(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/**	Returns the size in bytes of the vertex element with the specified semantic. */
		u32 GetElementSize(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/**	Returns offset of the vertex from start of the stream in bytes. */
		u32 GetElementOffsetFromStream(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/**	Gets vertex stride in bytes (offset from one vertex to another) in the specified stream. */
		u32 GetVertexStride(u32 streamIndex) const;

		/**	Gets vertex stride in bytes (offset from one vertex to another) in all the streams. */
		u32 GetVertexStride() const;

		/**	Gets offset in bytes from the start of the internal buffer to the start of the specified stream. */
		u32 GetStreamOffset(u32 streamIndex) const;

		/**	Returns the number of vertex elements. */
		u32 GetElementCount() const { return (u32)mVertexElements.size(); }

		/**	Returns the vertex element at the specified index. */
		const VertexElement& GetElement(u32 index) const { return mVertexElements[index]; }

		/**	Returns the vertex element with the specified semantic. */
		const VertexElement* GetElement(VertexElementSemantic semantic, u32 semanticIndex = 0, u32 streamIndex = 0) const;

		/** Returns all the elements of the definition. */
		const TInlineArray<VertexElement, 8>& GetElements() const { return mVertexElements; }

		/* Checks can a vertex buffer declared with the provided vertex elements be bound to a shader defined with the provided vertex element inputs. */
		static bool IsCompatibleWithShaderInputs(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription);

		/** Returns a list of vertex elements that the provided shader expects as inputs, but aren't provided in the list of vertex buffer elements. */
		static TInlineArray<VertexElement, 8> GetMissingElementsForShaderInput(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription);

	private:
		friend class Mesh;
		friend class render::Mesh;

		VertexDescription() = default;

		/**	Returns the largest stream index of all the stored vertex elements. */
		u32 GetLargestStreamIndex() const;

		/**	Checks if any of the vertex elements use the specified stream index. */
		bool HasStream(u32 streamIndex) const;

		/** Calculates offsets of each vertex element, at which they will be stored in the vertex buffer. */
		void CalculateOffsets();

	private:
		TInlineArray<VertexElement, 8> mVertexElements;
		u32 mId;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class VertexDescriptionRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for VertexElement. */
template <>
struct hash<b3d::VertexElement>
{
	size_t operator()(const b3d::VertexElement& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.GetType());
		b3d::B3DCombineHash(hash, value.GetSemantic());
		b3d::B3DCombineHash(hash, value.GetSemanticIndex());
		b3d::B3DCombineHash(hash, value.GetStreamIndex());
		b3d::B3DCombineHash(hash, value.GetInstanceStepRate());
		b3d::B3DCombineHash(hash, value.GetOffset());

		return hash;
	}
};

/** Hash value generator for VertexDescription. */
template <>
struct hash<b3d::VertexDescription>
{
	size_t operator()(const b3d::VertexDescription& value) const
	{
		size_t hash = 0;

		for(const auto& element : value.GetElements())
			b3d::B3DCombineHash(hash, element);

		return hash;
	}
};
} // namespace std

/** @endcond */
