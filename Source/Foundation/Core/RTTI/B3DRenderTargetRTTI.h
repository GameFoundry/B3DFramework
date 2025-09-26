//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "RenderAPI/B3DRenderWindow.h"
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
		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InvalidStateException, "Unable to instantiate abstract class.");
			return nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "RenderTarget";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderTarget;
		}
	};

	class RenderTextureRTTI : public TRTTIType<RenderTexture, RenderTarget, RenderTextureRTTI>
	{
	public:
		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InvalidStateException, "This object cannot be instantiated using reflection.");
			return nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "RenderTexture";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderTexture;
		}
	};

	class RenderWindowRTTI : public TRTTIType<RenderWindow, RenderTarget, RenderWindowRTTI>
	{
	public:
		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InvalidStateException, "This object cannot be instantiated using reflection.");
			return nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "RenderWindow";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderWindow;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
