//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Wrapper around the buffer that holds a D3D12 acceleration structure and the view shaders read it through, which manages their lifetime. */
		class D3D12AccelerationStructure : public D3D12Resource
		{
		public:
			/**
			 * @param	owner	Manager that takes care of tracking and releasing of this object.
			 * @param	buffer	Buffer that stores the structure, created with the acceleration structure resource flag. The structure takes
			 *					ownership of it.
			 * @param	name	Name of the structure, for debugging.
			 */
			D3D12AccelerationStructure(D3D12ResourceManager* owner, D3D12Buffer& buffer, const StringView& name);
			~D3D12AccelerationStructure() override;

			/** Returns the address builds and top level instances refer to the structure by. */
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mGpuVirtualAddress; }

			/** Returns the buffer that stores the structure. Barriers of the structure apply to it. */
			D3D12Buffer& GetBuffer() const { return *mBuffer; }

			/** Returns a CPU descriptor of the shader resource view that shaders trace rays against the structure through. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle() const { return mSrv; }

			/**
			 * Converts @p inputs into the native inputs of a size query or a build.
			 *
			 * @param	inputs			Contents of the structure.
			 * @param	outInputs		Native inputs. Points into @p outGeometries.
			 * @param	outGeometries	Native geometries of a bottom level structure. Must stay alive while @p outInputs is used.
			 */
			static void GetBuildInputs(const GpuAccelerationStructureInputs& inputs, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& outInputs,
				FrameVector<D3D12_RAYTRACING_GEOMETRY_DESC>& outGeometries);

		private:
			D3D12Buffer* mBuffer;
			D3D12_GPU_VIRTUAL_ADDRESS mGpuVirtualAddress = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE mSrv{};
		};

		/** D3D12 implementation of an acceleration structure. */
		class D3D12GpuAccelerationStructure : public GpuAccelerationStructure
		{
		public:
			/**
			 * @param	device					Device that created the structure.
			 * @param	createInformation		Description of the structure, which the device validated.
			 * @param	accelerationStructure	Native structure, which the base class destroys.
			 */
			D3D12GpuAccelerationStructure(D3D12GpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation, D3D12AccelerationStructure& accelerationStructure);

			/** Returns the native structure. */
			D3D12AccelerationStructure* GetD3D12Resource() const { return mAccelerationStructure; }

		private:
			D3D12AccelerationStructure* mAccelerationStructure;
		};

		/** @} */
	} // namespace render
} // namespace b3d
