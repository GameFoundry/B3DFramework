//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "CoreThread/BsCoreObject.h"
#include "Material/BsShaderVariation.h"
#include "String/BsStringID.h"

namespace bs
{
	/** @addtogroup Implementation
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

	/** Data that may be passed to Technique on creation to initialize it with precompiled set of passes (rather than requiring on-demand compilation). */
	template<bool Core>
	struct TPrecompiledVariationData
	{
		using PassType = CoreVariantType<Pass, Core>;

		TPrecompiledVariationData(const TInlineArray<SPtr<PassType>, 1>& precompiledPasses = {})
			: PrecompiledPasses(precompiledPasses)
		{ }

		TInlineArray<SPtr<PassType>, 1> PrecompiledPasses;
	};

	using PrecompiledVariationData = TPrecompiledVariationData<false>;
	namespace ct { using PrecompiledVariationData = TPrecompiledVariationData<true>; }

	/** Base class that is used for implementing both sim and core versions of Technique. */
	class B3D_CORE_EXPORT TechniqueBase
	{
	public:
		TechniqueBase(const String& language, const ShaderVariationParameters& variationParameters);
		virtual ~TechniqueBase() = default;

		/**	Checks if this technique is supported based on current render and other systems. */
		bool IsSupported() const;

		/** Returns a set of preprocessor defines used for compiling this particular technique. */
		const ShaderVariationParameters& GetVariationParameters() const { return mVariationParameters; }

	protected:
		/** Marks the contents as dirty, causing it to sync with the render thread object. */
		virtual void MarkCoreDirty(ShaderVariationDirtyFlags flag) {}

		/** @copydoc CoreObject::SyncToCore */
		virtual void SyncToCore() {};

		String mLanguage;
		ShaderVariationParameters mVariationParameters;
	};

	/** Templated class that is used for implementing both sim and core versions of Technique. */
	template <bool Core>
	class B3D_CORE_EXPORT TTechnique : public TechniqueBase
	{
	public:
		using PassType = CoreVariantType<Pass, Core>;
		using ShaderType = CoreVariantType<Shader, Core>;
		using TechniqueType = CoreVariantType<Technique, Core>;

		TTechnique();
		TTechnique(const WeakSPtr<ShaderType>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<TPrecompiledVariationData<Core>>& precompiledData);
		virtual ~TTechnique() = default;

		/**	Returns a pass with the specified index. */
		SPtr<PassType> GetPass(u32 index) const;

		/**	Returns total number of passes. */
		u32 GetPassCount() const;

		/** Compiles the variation in case it was not initialized with precompiled data. */
		TAsyncOp<bool> Compile();

		/** Returns true if the technique has been fully compiled. */
		bool IsCompiled() const { return mIsCompiled; }

		/**
		 * @name Internal
		 * @{
		 */

		/** Assigns a set of compiled passes to the technique. This should be called only when a technique has not been initialized with precompiled pass data, and compilation for the technique finished. */
		void SetCompiledPassData(TInlineArray<SPtr<PassType>, 1> compiledPasses);

		/** Sets the shader that owns this variation. */
		void SetOwner(const WeakSPtr<ShaderType>& owner);

		/***/

	protected:
		/** Returns a reference to itself using the most derived type. */
		virtual SPtr<TechniqueType> GetSelf() = 0;

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
	 * Technique is a set of shading passes bindable to the GPU pipeline. Each technique can also have a set of properties
	 * that help the engine to determine which technique should be used under which circumstances (if more than one
	 * technique is available).
	 *
	 * @note
	 * Normally you want to have a separate technique for every render system and renderer your application supports.
	 * For example, if you are supporting DirectX11 and OpenGL you will want to have two techniques, one using HLSL based
	 * GPU programs, other using GLSL. Those techniques should try to mirror each other's end results.
	 */
	class B3D_CORE_EXPORT Technique : public IReflectable, public CoreObject, public TTechnique<false>
	{
	public:
		Technique(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData);

		/** Retrieves an implementation of a technique usable only from the core thread. */
		SPtr<ct::Technique> GetCore() const;

		/**
		 * Creates a new variation.
		 *
		 * @param		owner				Shader that owns the variation.
		 * @param		language			Shading language used by the variation. The engine will not use this variation unless this language is supported by the render backend.
		 * @param		variationParameters	Variation parameters used for compiling this variation.
		 * @param		precompiledData		Optional set of precompiled variation data. If not provided, you must manually call Compile() on the variation before use.
		 * @return							Newly creted variation.
		 */
		static SPtr<Technique> Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData = {});

	protected:
		SPtr<ct::CoreObject> CreateCore() const override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void MarkCoreDirty(ShaderVariationDirtyFlags flag) override;
		CoreSyncPacket* CreateSyncPacket(FrameAlloc& allocator, u32 flags) override;
		void SyncToCore() override;

		SPtr<Technique> GetSelf() override { return std::static_pointer_cast<Technique>(GetShared()); }

		/**	Creates a new technique but doesn't initialize it. */
		static SPtr<Technique> CreateEmpty();

	private:
		struct SyncPacket;
		friend class ct::Technique;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Serialization only constructor. */
		Technique();

	public:
		friend class TechniqueRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	class CoreTechniqueRTTI;

	namespace ct
	{
		/** @addtogroup Material-Internal
		 *  @{
		 */

		/** Core thread version of bs::Technique. */
		class B3D_CORE_EXPORT Technique : public IReflectable, public CoreObject, public TTechnique<true>
		{
		public:
			Technique(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData);

			/** @copydoc bs::Technique::Create(const WeakSPtr<Shader>&, const String&, const ShaderVariationParameters&, const Optional<PrecompiledVariationData>&) */
			static SPtr<Technique> Create(const WeakSPtr<Shader>& owner, const String& language, const ShaderVariationParameters& variationParameters, const Optional<PrecompiledVariationData>& precompiledData = {});

			/**	Creates a new empty technique. */
			static SPtr<Technique> CreateEmpty();

		protected:
			friend class bs::Technique;

			void SyncToCore(const CoreSyncData& data, FrameAlloc& allocator) override;
			SPtr<Technique> GetSelf() override { return std::static_pointer_cast<Technique>(GetShared()); }

		private:
			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/

			/** Serialization only constructor. */
			Technique();

		public:
			friend class bs::CoreTechniqueRTTI;
			static RTTITypeBase* GetRttiStatic();
			RTTITypeBase* GetRtti() const override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
