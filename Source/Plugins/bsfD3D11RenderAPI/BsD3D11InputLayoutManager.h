//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsVertexDeclaration.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Handles creation and caching of DirectX 11 input layout objects. */
		class D3D11InputLayoutManager
		{
		public:
			/**	Key uniquely identifying vertex declaration and vertex shader combination. */
			struct VertexDeclarationKey
			{
				u64 VertxDeclId;
				u32 VertexProgramId;
			};

			/**	Creates a hash from vertex declaration key. */
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

			/**	Contains data about a single instance of DX11 input layout object. */
			struct InputLayoutEntry
			{
				InputLayoutEntry() {}

				ID3D11InputLayout* InputLayout;
				u32 LastUsedIdx;
			};

		public:
			D3D11InputLayoutManager() = default;
			~D3D11InputLayoutManager();

			/**
			 * Finds an existing or creates a new D3D11 input layout. Input layout maps a vertex declaration
			 * from a vertex buffer to vertex program input declaration
			 *
			 * @param[in]	vertexShaderDecl	Vertex declaration describing vertex program input parameters.
			 * @param[in]	vertexBufferDecl	Vertex declaration describing structure of a vertex buffer to be bound as input
			 *									to the GPU program.
			 * @param[in]	vertexProgram		Instance of the vertex program we are creating input layout for.
			 *
			 * @note	Error will be thrown if the vertex buffer doesn't provide all the necessary data that the shader expects.
			 */
			ID3D11InputLayout* RetrieveInputLayout(const SPtr<VertexDeclaration>& vertexShaderDecl, const SPtr<VertexDeclaration>& vertexBufferDecl, D3D11GpuProgram& vertexProgram);

		private:
			/**	Creates a new input layout using the specified parameters and stores it in the input layout map. */
			void AddNewInputLayout(const SPtr<VertexDeclaration>& vertexShaderDecl, const SPtr<VertexDeclaration>& vertexBufferDecl, D3D11GpuProgram& vertexProgram);

			/**	Destroys least used input layout. */
			void RemoveLeastUsed();

		private:
			static const int DECLARATION_BUFFER_SIZE = 1024;
			static const int NUM_ELEMENTS_TO_PRUNE = 64;

			UnorderedMap<VertexDeclarationKey, InputLayoutEntry*, HashFunc, EqualFunc> mInputLayoutMap;

			bool mWarningShown = false;
			u32 mLastUsedCounter = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
