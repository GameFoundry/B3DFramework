//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "BsCorePrerequisites.h"
#include "BsGpuBackend.h"
#include "BsGpuDevice.h"
#include "RenderAPI/BsSamplerState.h"
#include "CoreThread/BsCommandQueue.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsRenderTexture.h"
#include "RenderAPI/BsRenderWindow.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsVertexDescription.h"
#include "RenderAPI/BsGpuDeviceCapabilities.h"
#include "Math/BsPlane.h"
#include "Utility/BsModule.h"
#include "Utility/BsEvent.h"

namespace bs
{
	class RenderAPIManager;

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Provides low-level API access to rendering commands (internally wrapping DirectX/OpenGL/Vulkan or similar).
		 *
		 * Methods that accept a CommandBuffer parameter get queued in the provided command buffer, and don't get executed until
		 * executeCommands() method is called. User is allowed to populate command buffers from non-core threads, but they all
		 * must get executed from the core thread.
		 *
		 * If a command buffer is not provivided to such methods, they execute immediately. Without a command buffer the methods
		 * are only allowed to be called from the core thread.
		 *
		 * @note	Accessible on any thread for methods accepting a CommandBuffer. Otherwise core thread unless specifically
		 *			noted otherwise on per-method basis.
		 */
		class B3D_CORE_EXPORT RenderAPI : public Module<RenderAPI>
		{
		public:
			RenderAPI();
			virtual ~RenderAPI();

			/**
			 * Returns the name of the rendering system.
			 *
			 * @note	Thread safe.
			 */
			virtual const StringID& GetName() const = 0;

			/**
			 * Prepares the backend for rendering a single frame. A frame involves a set of submitted command buffers followed by any optional present operations on the window surfaces.
			 * Must be followed by an EndFrame() call after all command buffers and present operations have been submitted.
			 */
			virtual void BeginFrame() {}

			/** Notifies the backend the rendering for the current frame has ended. See BeginFrame(). */
			virtual void EndFrame() {}

			/** Returns the primary GPU on which to perform rendering. */
			virtual SPtr<GpuDevice> GetPrimaryGpuDevice() const = 0;

			/************************************************************************/
			/* 							INTERNAL METHODS				        	*/
			/************************************************************************/
		protected:
			friend class RenderAPIManager;

			/**
			 * Initializes the render API system and creates a primary render window.
			 *
			 * @note Sim thread only.
			 */
			SPtr<bs::RenderWindow> Initialize(const RENDER_WINDOW_DESC& primaryWindowDesc);

			/** Initializes the render API system. Called before the primary render window is created. */
			virtual void Initialize();

			/**
			 * Performs (optional) secondary initialization of the render API system. Called after the render window is
			 * created.
			 */
			virtual void InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow);

			/**
			 * Shuts down the render API system and cleans up all resources.
			 *
			 * @note	Sim thread.
			 */
			void Destroy();

			/** Performs render API system shutdown on the core thread. */
			virtual void DestroyCore();

			/** Converts the number of vertices to number of primitives based on the specified draw operation. */
			u32 VertexCountToPrimCount(DrawOperationType type, u32 elementCount);
		};

		/** Shorthand for GpuDevice::GetCapabilities(). */
		inline const GpuDeviceCapabilities& GetGpuDeviceCapabilities(u32 deviceIndex = 0)
		{
			return GpuBackend::Instance().GetDevice(deviceIndex)->GetCapabilities();
		}

		/**	Provides easy access to render API. */
		inline RenderAPI& GetRenderAPI()
		{
			return RenderAPI::Instance();
		}

		/** @} */
	} // namespace ct
} // namespace bs
