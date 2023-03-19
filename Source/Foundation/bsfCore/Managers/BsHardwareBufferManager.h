//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Utility/BsModule.h"
#include "RenderAPI/BsVertexDeclaration.h"

namespace bs
{
	struct GenericGpuBufferCreateInformation;
	struct GPU_PARAMS_DESC;

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Handles creation of various hardware buffers.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
	{
	public:
		HardwareBufferManager() = default;
		virtual ~HardwareBufferManager() = default;

		/**
		 * Creates a new vertex declaration from a list of vertex elements.
		 *
		 * @param[in]	desc	Description of the object to create.
		 */
		SPtr<VertexDeclaration> CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc);

		/** @copydoc GpuParams::Create(const SPtr<GpuPipelineParamInfo>&) */
		SPtr<GpuParams> CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo);
	};

	namespace ct
	{

		/**
		 * Handles creation of various hardware buffers.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
		{
		public:
			virtual ~HardwareBufferManager() {}

			/**
			 * @copydoc bs::HardwareBufferManager::CreateVertexDeclaration
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexDeclaration> CreateVertexDeclaration(const SPtr<VertexDataDesc>& createInformation, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Creates a new vertex declaration from a list of vertex elements.
			 *
			 * @param[in]	elements		List of elements to initialize the declaration with.
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexDeclaration> CreateVertexDeclaration(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuParams::Create(const SPtr<GpuPipelineParamInfo>&, GpuDeviceFlags) */
			SPtr<GpuParams> CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class bs::VertexDeclaration;
			friend class bs::GpuBuffer;

			/** Key for use in the vertex declaration map. */
			struct VertexDeclarationKey
			{
				VertexDeclarationKey(const Vector<VertexElement>& elements);

				class HashFunction
				{
				public:
					size_t operator()(const VertexDeclarationKey& key) const;
				};

				class EqualFunction
				{
				public:
					bool operator()(const VertexDeclarationKey& lhs, const VertexDeclarationKey& rhs) const;
				};

				Vector<VertexElement> Elements;
			};

			/** @copydoc CreateVertexDeclaration(const Vector<VertexElement>&, GpuDeviceFlags) */
			virtual SPtr<VertexDeclaration> CreateVertexDeclarationInternal(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc CreateGpuParams */
			virtual SPtr<GpuParams> CreateGpuParamsInternal(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			typedef UnorderedMap<VertexDeclarationKey, SPtr<VertexDeclaration>, VertexDeclarationKey::HashFunction, VertexDeclarationKey::EqualFunction> DeclarationMap;

			DeclarationMap mCachedDeclarations;
		};
	} // namespace ct

	/** @} */
} // namespace bs
