//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "CoreObject/B3DRenderThread.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class RenderTargetRTTI : public TRTTIType<RenderTarget, IReflectable, RenderTargetRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "RenderTarget";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderTarget;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Unable to instantiate abstract class.");
			return nullptr;
		}

	};

	class RenderTextureRTTI : public TRTTIType<RenderTexture, RenderTarget, RenderTextureRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "RenderTexture";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderTexture;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "This object cannot be instantiated using reflection.");
			return nullptr;
		}

	};

	class RenderWindowRTTI : public TRTTIType<RenderWindow, RenderTarget, RenderWindowRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "RenderWindow";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderWindow;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "This object cannot be instantiated using reflection.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
