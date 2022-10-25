//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLContext.h"

namespace bs::ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	MacOS specific implementation of an OpenGL context. */
	class MacOSContext : public GLContext
	{
		struct Pimpl;

	public:
		/**
		 * Constructs a new OpenGL context.
		 *
		 * @param[in]	depthStencil	True if the framebuffer using the context can have a depth-stencil buffer.
		 * @param[in]	msaaCount		Number of samples the framebuffer using the context is allowed to have.
		 */
		MacOSContext(bool depthStencil, u32 msaaCount);
		virtual ~MacOSContext();

		/** @copydoc GLContext::setCurrent */
		void setCurrent(const RenderWindow& window) override;

		/** @copydoc GLContext::endCurrent */
		void endCurrent() override;

		/** @copydoc GLContext::releaseContext  */
		void releaseContext() override;

		/** Marks the context as dirty and requiring update. Should be called when the drawable changes size or location. */
		void markAsDirty();

		/** Updates the context if dirty. */
		void updateIfDirty();

		/** Enables or disables VSync using the specified interval. Interval of 0 disables VSync. */
		void setVSync(int interval);

		/** Swaps the framebuffer currently attached to this context. */
		void swapBuffers();

		/**
		 * Locks the context so it can safely be used across threads. Should be called before performing any OpenGL
		 * action or direct operation on the context. When done unlock it via unlock().
		 */
		void lock();

		/** Unlocks the context locked via lock(). */
		void unlock();

	private:
		Pimpl* m;
	};

	/** @} */
} // namespace bs::ct
