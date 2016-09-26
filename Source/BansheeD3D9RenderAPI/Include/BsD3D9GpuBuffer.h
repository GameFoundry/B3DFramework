//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsGpuBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup D3D9
	 *  @{
	 */

	/**
	 * Represents a generic GPU buffer in DX9. This class is just a dummy in order to conform to the interface as DX9
	 * supports no such buffers.
	 */
	class BS_D3D9_EXPORT D3D9GpuBufferCore : public GpuBufferCore
	{
	public:
		~D3D9GpuBufferCore();

		/** @copydoc GpuBufferCore::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc GpuBufferCore::unlock */
		void unlock() override;

		/** @copydoc GpuBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* pDest) override;

		/** @copydoc GpuBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* pSource,
				BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/** @copydoc GpuBufferCore::copyData */
		void copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset,
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false) override;

	protected:
		friend class D3D9HardwareBufferCoreManager;

		D3D9GpuBufferCore(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferFormat format, 
			GpuBufferUsage usage, bool randomGpuWrite = false, bool useCounter = false);

		/** @copydoc GpuBufferCore::createView */
		GpuBufferView* createView() override;

		/** @copydoc GpuBufferCore::destroyView */
		void destroyView(GpuBufferView* view) override;

		/** @copydoc GpuBufferCore::initialize */
		void initialize() override;	
	};

	/** @} */
}