//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "BsCoreApplication.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsRenderTexture.h"
#include "RenderAPI/BsRenderWindow.h"
#include "CoreObject/BsRenderThread.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class RenderTargetRTTI : public RTTIType<RenderTarget, IReflectable, RenderTargetRTTI>
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

		u32 GetRttiId() override
		{
			return TID_RenderTarget;
		}
	};

	class RenderTextureRTTI : public RTTIType<RenderTexture, RenderTarget, RenderTextureRTTI>
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

		u32 GetRttiId() override
		{
			return TID_RenderTexture;
		}
	};

	class RenderWindowRTTI : public RTTIType<RenderWindow, RenderTarget, RenderWindowRTTI>
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

		u32 GetRttiId() override
		{
			return TID_RenderWindow;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
