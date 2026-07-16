//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuFrameCapture.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;
	}

	/** Captures Metal commands into Xcode or a standalone GPU trace document. */
	class MetalGpuFrameCapture final : public GpuFrameCapture
	{
	public:
		explicit MetalGpuFrameCapture(render::MetalGpuDevice& device);
		~MetalGpuFrameCapture() override;

		void Start() override;
		void Stop() override;

	private:
		struct Impl;
		render::MetalGpuDevice& mDevice;
		TUnique<Impl> mImpl;
	};
} // namespace b3d
