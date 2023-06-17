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
	enum class B3D_SCRIPT_EXPORT(ExportName(ClearFlags)) ClearFlagBits
	{
		Empty,
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2
	};

	typedef Flags<ClearFlagBits> ClearFlags;
	B3D_FLAGS_OPERATORS(ClearFlagBits)

	/** Common base type used for both sim and core thread variants of Viewport. */
	class B3D_CORE_EXPORT ViewportBase
	{
	public:
		virtual ~ViewportBase() = default;

		/** Determines the area that the viewport covers. Coordinates are in normalized [0, 1] range. */
		B3D_SCRIPT_EXPORT(ExportName(Area), Property(Setter))
		void SetArea(const Rect2& area);

		/** @copydoc SetArea() */
		B3D_SCRIPT_EXPORT(ExportName(Area), Property(Getter))
		Rect2 GetArea() const { return mNormArea; }

		/**	Returns the area of the render target covered by the viewport, in pixels. */
		B3D_SCRIPT_EXPORT(ExportName(PixelArea), Property(Getter))
		Rect2I GetPixelArea() const;

		/** Determines which portions of the render target should be cleared before rendering to this viewport is performed. */
		B3D_SCRIPT_EXPORT(ExportName(ClearFlags), Property(Setter))
		void SetClearFlags(ClearFlags flags);

		/** @copydoc SetClearFlags() */
		B3D_SCRIPT_EXPORT(ExportName(ClearFlags), Property(Getter))
		ClearFlags GetClearFlags() const { return mClearFlags; }

		/**	Sets values to clear color, depth and stencil buffers to. */
		void SetClearValues(const Color& clearColor, float clearDepth = 0.0f, u16 clearStencil = 0);

		/** Determines the color to clear the viewport to before rendering, if color clear is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(ClearColor), Property(Setter))
		void SetClearColorValue(const Color& color);

		/** @copydoc SetClearColorValue() */
		B3D_SCRIPT_EXPORT(ExportName(ClearColor), Property(Getter))
		const Color& GetClearColorValue() const { return mClearColorValue; }

		/** Determines the value to clear the depth buffer to before rendering, if depth clear is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(ClearDepth), Property(Setter))
		void SetClearDepthValue(float depth);

		/** @copydoc SetClearDepthValue() */
		B3D_SCRIPT_EXPORT(ExportName(ClearDepth), Property(Getter))
		float GetClearDepthValue() const { return mClearDepthValue; }

		/** Determines the value to clear the stencil buffer to before rendering, if stencil clear is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(ClearStencil), Property(Setter))
		void SetClearStencilValue(u16 value);

		/** @copydoc SetClearStencilValue() */
		B3D_SCRIPT_EXPORT(ExportName(ClearStencil), Property(Getter))
		u16 GetClearStencilValue() const { return mClearStencilValue; }

	protected:
		ViewportBase(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		/**
		 * Marks the core data as dirty. This causes the data from the sim thread object be synced with the core thread
		 * version of the object.
		 */
		virtual void MarkCoreDirtyInternal() {}

		/** Gets the render target width. */
		virtual u32 GetTargetWidth() const = 0;

		/**	Gets the render target width. */
		virtual u32 GetTargetHeight() const = 0;

		Rect2 mNormArea;

		ClearFlags mClearFlags;
		Color mClearColorValue;
		float mClearDepthValue;
		u16 mClearStencilValue;

		static const Color kDefaultClearColor;
	};

	/** Templated common base type used for both sim and core thread variants of Viewport. */
	template <bool Core>
	class TViewport : public ViewportBase
	{
	public:
		using RenderTargetType = CoreVariantType<RenderTarget, Core>;

		TViewport(SPtr<RenderTargetType> target = nullptr, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f)
			: ViewportBase(x, y, width, height), mTarget(std::move(target))
		{}

		virtual ~TViewport() = default;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P p);

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
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Viewport : public IReflectable, public CoreObject, public TViewport<false>
	{
	public:
		/**	Determines the render target the viewport is associated with. */
		B3D_SCRIPT_EXPORT(ExportName(Target), Property(Setter))
		void SetTarget(const SPtr<RenderTarget>& target);

		/** @copydoc setTarget() */
		B3D_SCRIPT_EXPORT(ExportName(Target), Property(Getter))

		SPtr<RenderTarget> GetTarget() const { return mTarget; }

		/**	Retrieves a core implementation of a viewport usable only from the core thread. */
		SPtr<ct::Viewport> GetCore() const;

		/**
		 * Creates a new viewport.
		 *
		 * @note	Viewport coordinates are normalized in [0, 1] range.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Viewport))
		static SPtr<Viewport> Create(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

	protected:
		Viewport(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		void MarkCoreDirtyInternal() override;
		u32 GetTargetWidth() const override;
		u32 GetTargetHeight() const override;

		CoreSyncData SyncToCore(FrameAlloc* allocator) override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		SPtr<ct::CoreObject> CreateCore() const override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		Viewport() = default;

		/** Creates an empty viewport for serialization purposes. */
		static SPtr<Viewport> CreateEmpty();

	public:
		friend class ViewportRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/** @copydoc bs::Viewport */
		class B3D_CORE_EXPORT Viewport : public CoreObject, public TViewport<true>
		{
		public:
			/**	Returns the render target the viewport is associated with. */
			SPtr<RenderTarget> GetTarget() const { return mTarget; }

			/**	Sets the render target the viewport will be associated with. */
			void SetTarget(const SPtr<RenderTarget>& target) { mTarget = target; }

			/** @copydoc bs::Viewport::Create() */
			static SPtr<Viewport> Create(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

		protected:
			friend class bs::Viewport;

			Viewport(const SPtr<RenderTarget>& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);

			u32 GetTargetWidth() const override;
			u32 GetTargetHeight() const override;

			void SyncToCore(const CoreSyncData& data, FrameAlloc& allocator) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
