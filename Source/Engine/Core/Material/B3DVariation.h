//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "CoreObject/B3DCoreObject.h"
#include "Material/B3DShaderVariation.h"
#include "String/B3DStringID.h"

namespace b3d
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Flags that signal which part of ShaderVariation changed. */
	enum class ShaderVariationDirtyFlag
	{
		Parent = 1 << 0,
		Passes = 1 << 1
	};

	using ShaderVariationDirtyFlags = Flags<ShaderVariationDirtyFlag, u32>;
	B3D_FLAGS_OPERATORS(ShaderVariationDirtyFlags)

	/** Data that may be passed to Variation on creation to initialize it with precompiled set of passes (rather than requiring on-demand compilation). */
	template<bool IsRenderProxy>
	struct TPrecompiledVariationData
	{
		using PassType = CoreVariantType<Pass, IsRenderProxy>;

		TPrecompiledVariationData(const TInlineArray<SPtr<PassType>, 1>& precompiledPasses = {})
			: PrecompiledPasses(precompiledPasses)
		{ }

		TInlineArray<SPtr<PassType>, 1> PrecompiledPasses;
	};

	using PrecompiledVariationData = TPrecompiledVariationData<false>;
	namespace render { using PrecompiledVariationData = TPrecompiledVariationData<true>; }

	/** Base class that is used for implementing both main and render thread versions of Variation. */
	class B3D_EXPORT VariationBase
	{
	public:
		VariationBase(const String& language, const ShaderVariationParameters& variationParameters);
		virtual ~VariationBase() = default;

		/**	Checks if this variation is supported based on current render and other systems. */
		bool IsSupported() const;

		/** Returns a set of preprocessor defines used for compiling this particular variation. */
		const ShaderVariationParameters& GetVariationParameters() const { return mVariationParameters; }

	protected:
		/** Marks the contents as dirty, causing it to sync with the render thread object. */
		virtual void MarkRenderProxyDirty(ShaderVariationDirtyFlags flags) {}

		/** @copydoc CoreObject::SyncToRenderProxy */
		virtual void SyncToRenderProxy() {}

		String mLanguage;
		ShaderVariationParameters mVariationParameters;
	};

	/** Templated class that is used for implementing both main and render thread versions of Variation. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TVariation : public VariationBase
	{
	public:
		using PassType = CoreVariantType<Pass, IsRenderProxy>;
		using ShaderType = CoreVariantType<Shader, IsRenderProxy>;
		using VariationType = CoreVariantType<Variation, IsRenderProxy>;

		TVariation();
		TVariation(const WeakSPtr<ShaderType>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<TPrecompiledVariationData<IsRenderProxy>>& precompiledData);
		virtual ~TVariation() = default;

		/**	Returns a pass with the specified index. */
		SPtr<PassType> GetPass(u32 passIndex) const;

		/**	Returns total number of passes. */
		u32 GetPassCount() const;

		/** Compiles the variation in case it was not initialized with precompiled data. */
		TAsyncOp<bool> Compile();

		/** Returns true if the variation has been fully compiled. */
		bool IsCompiled() const { return mIsCompiled; }

		/**
		 * @name Internal
		 * @{
		 */

		/** Assigns a set of compiled passes to the variation. This should be called only when a variation has not been initialized with precompiled pass data, and compilation for the variation finished. */
		void SetCompiledPassData(TInlineArray<SPtr<PassType>, 1> compiledPasses);

		/** Sets the shader that owns this variation. */
		void SetOwner(const WeakSPtr<ShaderType>& owner);

		/***/

	protected:
		/** Returns a reference to itself using the most derived type. */
		virtual SPtr<VariationType> GetSelf() = 0;

		WeakSPtr<ShaderType> mOwner;
		TInlineArray<SPtr<PassType>, 1> mPasses;
		bool mHasPassData = false;
		bool mIsCompiled = false;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**
	 * Variation is a set of shading passes bindable to the GPU pipeline. Each shader has at least one variation, but many have multiple.
	 * Each variation is typically compiled with different set of preprocessor defines enabling or disabling specific features of the shader.
	 * A shader may also have multiple variations for different rendering backends (e.g. DirectX, OpenGL, Vulkan, etc.).
	 */
	class B3D_EXPORT Variation : public IReflectable, public CoreObject, public TVariation<false>
	{
	public:
		Variation(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData);

		/**
		 * Creates a new variation.
		 *
		 * @param owner					Shader that owns the variation.
		 * @param language				Shading language used by the variation. The engine will not use this variation unless this language is supported by the render backend.
		 * @param variationParameters	Variation parameters used for compiling this variation.
		 * @param precompiledData		Optional set of precompiled variation data. If not provided, you must manually call Compile() on the variation before use.
		 * @return						Newly creted variation.
		 */
		static SPtr<Variation> Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData = {});

	protected:
		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void MarkRenderProxyDirty(ShaderVariationDirtyFlags flags) override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		void SyncToRenderProxy() override;

		SPtr<Variation> GetSelf() override { return std::static_pointer_cast<Variation>(GetShared()); }

		/**	Creates a new variation but doesn't initialize it. */
		static SPtr<Variation> CreateEmpty();

	private:
		struct SyncPacket;
		friend class render::Variation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Serialization only constructor. */
		Variation();

	public:
		friend class VariationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	class VariationRenderProxyRTTI;

	namespace render
	{
		/** @addtogroup Material-Internal
		 *  @{
		 */

		/** Render thread version of b3d::Variation. */
		class B3D_EXPORT Variation : public IReflectable, public RenderProxy, public TVariation<true>
		{
		public:
			Variation(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData);

			/** @copydoc b3d::Variation::Create(const WeakSPtr<Shader>&, const String&, const ShaderVariationParameters&, const Optional<PrecompiledVariationData>&) */
			static SPtr<Variation> Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const TOptional<PrecompiledVariationData>& precompiledData = {});

			/**	Creates a new empty variation. */
			static SPtr<Variation> CreateEmpty();

		protected:
			friend class b3d::Variation;

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
			SPtr<Variation> GetSelf() override { return std::static_pointer_cast<Variation>(GetShared()); }

		private:
			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/

			/** Serialization only constructor. */
			Variation();

		public:
			friend class b3d::VariationRenderProxyRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
