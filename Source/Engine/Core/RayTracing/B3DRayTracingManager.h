//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d::render
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Normal, tangent and texture coordinates that vertices without them read, in the formats of RayTracingGeometry. Layout of the buffer
	 * RayTracingManager::GetDefaultAttributes() returns.
	 */
	struct RayTracingDefaultAttributes
	{
		u32 Normal = 0xFFFF8080; /**< (0, 0, 1). */
		u32 Tangent = 0xFF8080FF; /**< (1, 0, 0), with handedness 1. */
		float Uv[2] = { 0.0f, 0.0f }; /**< (0, 0). */
	};

	static_assert(sizeof(RayTracingDefaultAttributes) == 16, "RayTracingDefaultAttributes must match the layout of its buffer.");

	/** Ray tracing resources that every RayTracingGeometry of a device shares. */
	class B3D_EXPORT RayTracingManager : public Module<RayTracingManager>
	{
	public:
		/**
		 * Returns a bindless buffer with the normal, tangent and texture coordinates that vertices without them read, creating it on first
		 * use. Its layout is RayTracingDefaultAttributes. Returns null if @p device has no bindless heaps.
		 *
		 * @note	Render thread.
		 */
		const TShared<GpuBuffer>& GetDefaultAttributes(GpuDevice& device);

	protected:
		void OnShutDown() override;

	private:
		UnorderedMap<GpuDevice*, TShared<GpuBuffer>> mDefaultAttributes;
	};

	/** @} */
} // namespace b3d::render
