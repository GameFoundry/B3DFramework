//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** A single vertex shader input resolved against the elements of a vertex buffer layout. */
		struct GpuVertexInputAttribute
		{
			/** Shader input the attribute feeds. */
			const VertexElement* ShaderInput = nullptr;

			/** Vertex buffer element providing the attribute's data, or null when the layout provides none. */
			const VertexElement* BufferElement = nullptr;

			/** Vertex-buffer stream the attribute fetches from (the null stream when there is no matching buffer element). */
			u32 StreamIndex = 0;

			/** True when the attribute advances once per instance rather than once per vertex. */
			bool SteppedPerInstance = false;
		};

		/**
		 * A vertex buffer layout resolved against the inputs of a vertex shader, pairing each shader input with the
		 * buffer element that feeds it. Serves as backend-agnostic input for building API-specific vertex input
		 * descriptions.
		 */
		struct B3D_EXPORT GpuVertexInputLayout
		{
			/** Value of NullStreamIndex when every shader input has a matching vertex buffer element. */
			static constexpr u32 kNoNullStream = ~0u;

			/** One resolved attribute per shader input element, in shader input declaration order. */
			TInlineArray<GpuVertexInputAttribute, 8> Attributes;

			/**
			 * Number of vertex-buffer streams the attributes reference (contiguous, starting at stream 0), including
			 * the null stream when present.
			 */
			u32 StreamCount = 0;

			/** Bitmask of streams fetched by attributes with a matching vertex buffer element. */
			u32 UsedStreamMask = 0;

			/** Stream reserved for shader inputs with no matching vertex buffer element, or kNoNullStream if all matched. */
			u32 NullStreamIndex = kNoNullStream;

			/**
			 * Resolves the provided shader inputs against the provided vertex buffer elements, matching them by
			 * semantic and semantic index. Unmatched shader inputs are assigned to an extra null stream, which the
			 * backend is expected to serve with an empty vertex buffer so such inputs read zero.
			 */
			void Resolve(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription);
		};

		/**
		 * Builds and caches backend vertex input objects describing how the elements of a vertex buffer layout map to
		 * the inputs of a vertex shader, so the mapping for a combination is built once and reused afterwards. Least
		 * recently used entries are evicted once the cache fills up.
		 *
		 * @note	Thread safe.
		 *
		 * @tparam	TDerived		Backend manager deriving from this class. Must provide:
		 *							- TVertexInput CreateVertexInput(const TShared<VertexDescription>& vertexBufferDescription,
		 *							  const TShared<VertexDescription>& shaderInputDescription, const GpuVertexInputLayout& layout)
		 *							  building the backend vertex input object; may return null on failure.
		 *							- void DestroyVertexInput(TVertexInput& vertexInput) releasing an object evicted from
		 *							  the cache. The derived destructor must call ReleaseAll() so eviction of the remaining
		 *							  entries happens while the derived class is still alive.
		 * @tparam	TVertexInput	Backend vertex input handle (e.g. a shared or raw pointer). Must be contextually
		 *							convertible to bool, where false means an empty handle.
		 */
		template<class TDerived, class TVertexInput>
		class TGpuVertexInputManager
		{
		public:
			/**
			 * Returns an object mapping vertex buffer elements to vertex shader inputs, building and caching it on
			 * first use. Returns null if the backend could not build it.
			 *
			 * @param	vertexBufferDescription		Describes the structure of a single vertex in a vertex buffer.
			 * @param	shaderInputDescription		Describes the vertex element inputs expected by a vertex shader.
			 */
			TVertexInput GetVertexInput(const TShared<VertexDescription>& vertexBufferDescription, const TShared<VertexDescription>& shaderInputDescription);

		protected:
			~TGpuVertexInputManager() = default;

			/** Evicts all cached entries. Call from the derived class' destructor. */
			void ReleaseAll();

		private:
			/**	Key uniquely identifying buffer and shader vertex declarations. */
			struct VertexDeclarationKey
			{
				u32 BufferDeclarationId;
				u32 ShaderDeclarationId;
			};

			/**	Creates a hash from a vertex declaration key. */
			class HashFunc
			{
			public:
				::std::size_t operator()(const VertexDeclarationKey& key) const;
			};

			/**	Compares two vertex declaration keys. */
			class EqualFunc
			{
			public:
				bool operator()(const VertexDeclarationKey& a, const VertexDeclarationKey& b) const;
			};

			/**	Contains data about a single cached vertex input object. */
			struct VertexInputEntry
			{
				TVertexInput VertexInput{};
				u32 LastUsedIndex = 0;
			};

			/**	Evicts the least recently used vertex inputs. Caller must hold the manager mutex. */
			void RemoveLeastUsed();

			static constexpr u32 kDeclarationBufferSize = 1024;
			static constexpr u32 kElementCountToPrune = 64;

			UnorderedMap<VertexDeclarationKey, VertexInputEntry, HashFunc, EqualFunc> mVertexInputMap;

			bool mWarningShown = false;
			u32 mLastUsedCounter = 0;

			Mutex mMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
