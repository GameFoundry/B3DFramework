//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <utility>
#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"
#include "Image/BsColor.h"
#include "Math/BsRect2I.h"
#include "Math/BsRect2.h"
#include "Utility/BsEvent.h"

namespace bs 
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Flags that determine which portion of the viewport to clear. */
	enum BS_SCRIPT_EXPORT(n:ClearFlags) class ClearFlagBits
	{
		Empty,
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2
	};

	typedef Flags<ClearFlagBits> ClearFlags;
	BS_FLAGS_OPERATORS(ClearFlagBits)

	/** Common base type used for both sim and core thread variants of Viewport. */
	class BS_CORE_EXPORT ViewportBase
	{
	public:
		virtual ~ViewportBase() = default;

		/** Determines the area that the viewport covers. Coordinates are in normalized [0, 1] range. */
		BS_SCRIPT_EXPORT(n:Area,pr:setter)
		void setArea(const Rect2& area);

		/** @copydoc setArea() */
		BS_SCRIPT_EXPORT(n:Area,pr:getter)
		Rect2 getArea() const { return mNormArea; }

		/**	Returns the area of the render target covered by the viewport, in pixels. */
		BS_SCRIPT_EXPORT(n:PixelArea,pr:getter)
		Rect2I getPixelArea() const;

		/** Determines which portions of the render target should be cleared before rendering to this viewport is performed. */
		BS_SCRIPT_EXPORT(n:ClearFlags,pr:setter)
		void setClearFlags(ClearFlags flags);

		/** @copydoc setClearFlags() */
		BS_SCRIPT_EXPORT(n:ClearFlags,pr:getter)
		ClearFlags getClearFlags() const { return mClearFlags; }

		/**	Sets values to clear color, depth and stencil buffers to. */
		void setClearValues(const Color& clearColor, float clearDepth = 0.0f, UINT16 clearStencil = 0);

		/** Determines the color to clear the viewport to before rendering, if color clear is enabled. */
		BS_SCRIPT_EXPORT(n:ClearColor,pr:setter)
		void setClearColorValue(const Color& color);

		/** @copydoc setClearColorValue() */
		BS_SCRIPT_EXPORT(n:ClearColor,pr:getter)
		const Color& getClearColorValue() const { return mClearColorValue; }

		/** Determines the value to clear the depth buffer to before rendering, if depth clear is enabled. */
		BS_SCRIPT_EXPORT(n:ClearDepth,pr:setter)
		void setClearDepthValue(float depth);

		/** @copydoc setClearDepthValue() */
		BS_SCRIPT_EXPORT(n:ClearDepth,pr:getter)
		float getClearDepthValue() const { return mClearDepthValue; }

		/** Determines the value to clear the stencil buffer to before rendering, if stencil clear is enabled. */
		BS_SCRIPT_EXPORT(n:ClearStencil,pr:setter)
		void setClearStencilValue(UINT16 value);

		/** @copydoc setClearStencilValue() */
		BS_SCRIPT_EXPORT(n:ClearStencil,pr:getter)
		UINT16 getClearStencilValue() const { return mClearStencilValue; }

	protected:
		ViewportBase(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/** 
		 * Marks the core data as dirty. This causes the data from the sim thread object be synced with the core thread
		 * version of the object.
		 */
		virtual void _markCoreDirty() { }

		/** Gets the render target width. */
		virtual UINT32 getTargetWidth() const = 0;

		/**	Gets the render target width. */
		virtual UINT32 getTargetHeight() const = 0;

		Rect2 mNormArea;

		ClearFlags mClearFlags;
		Color mClearColorValue;
		float mClearDepthValue;
		UINT16 mClearStencilValue;

		static const Color DEFAULT_CLEAR_COLOR;
	};

	/** Templated common base type used for both sim and core thread variants of Viewport. */
	template<bool Core>
	class TViewport : public ViewportBase
	{
	public:
		using RenderTargetType = CoreVariantType<RenderTarget, Core>;

		TViewport(SPtr<RenderTargetType> target = nullptr, float x = 0.0f, float y = 0.0f, float width = 1.0f, 
			float height = 1.0f)
			:ViewportBase(x, y, width, height), mTarget(std::move(target))
		{ }
		virtual ~TViewport() = default;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p);

	protected:
		SPtr<RenderTargetType> mTarget;
	};

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Viewport determines to which RenderTarget should rendering be performed. It allows you to render to a sub-region of the
	 * target by specifying the area rectangle, and allows you to set up color/depth/stencil clear values for that specific region.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) Viewport : public IReflectable, public CoreObject, public TViewport<false>
	{
	public:
		/**	Determines the render target the viewport is associated with. */
		BS_SCRIPT_EXPORT(n:Target,pr:setter)
		void setTarget(const SPtr<RenderTarget>& target);

		/** @copydoc setTarget() */
		BS_SCRIPT_EXPORT(n:Target,pr:getter)
		SPtr<RenderTarget> getTarget() const { return mTarget; }

		/**	Retrieves a core implementation of a viewport usable only from the core thread. */
		SPtr<ct::Viewport> getCore() const;

		/**
		 * Creates a new viewport.
		 *
		 * @note	Viewport coordinates are normalized in [0, 1] range.
		 */
		BS_SCRIPT_EXPORT(ec:Viewport)
		static SPtr<Viewport> create(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, 
			float width = 1.0f, float height = 1.0f);

	protected:
		Viewport(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/** @copydoc ViewportBase::_markCoreDirty */
		void _markCoreDirty() override;

		/** @copydoc ViewportBase::getTargetWidth */
		UINT32 getTargetWidth() const override;

		/** @copydoc ViewportBase::getTargetHeight */
		UINT32 getTargetHeight() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		Viewport() = default;

		/** Creates an empty viewport for serialization purposes. */
		static SPtr<Viewport> createEmpty();
	public:
		friend class ViewportRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** @copydoc bs::Viewport */
	class BS_CORE_EXPORT Viewport : public CoreObject, public TViewport<true>
	{
	public:
		/**	Returns the render target the viewport is associated with. */
		SPtr<RenderTarget> getTarget() const { return mTarget; }

		/**	Sets the render target the viewport will be associated with. */
		void setTarget(const SPtr<RenderTarget>& target) { mTarget = target; }

		/** @copydoc bs::Viewport::create() */
		static SPtr<Viewport> create(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, 
			float width = 1.0f, float height = 1.0f);

	protected:
		friend class bs::Viewport;

		Viewport(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/** @copydoc ViewportBase::getTargetWidth */
		UINT32 getTargetWidth() const override;

		/** @copydoc ViewportBase::getTargetHeight */
		UINT32 getTargetHeight() const override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;
	};

	/** @} */
		}
}