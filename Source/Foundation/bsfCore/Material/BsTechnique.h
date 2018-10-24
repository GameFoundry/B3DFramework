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

	/** Base class that is used for implementing both sim and core versions of Technique. */
	class BS_CORE_EXPORT TechniqueBase
	{
	public:
		TechniqueBase(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation);
		virtual ~TechniqueBase() = default;

		/**	Checks if this technique is supported based on current render and other systems. */
		bool isSupported() const;

		/** Checks if the technique has the specified tag. */
		bool hasTag(const StringID& tag);

		/** Checks if the technique has any tags. */
		UINT32 hasTags() const { return !mTags.empty(); }

		/** Returns a set of preprocessor defines used for compiling this particular technique. */
		const ShaderVariation& getVariation() const { return mVariation; }

	protected:
		String mLanguage;
		Vector<StringID> mTags;
		ShaderVariation mVariation;
	};

	/** Templated class that is used for implementing both sim and core versions of Technique. */
	template<bool Core>
	class BS_CORE_EXPORT TTechnique : public TechniqueBase
	{
	public:
		using PassType = CoreVariantType<Pass, Core>;
		
		TTechnique();
		TTechnique(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, 
			const Vector<SPtr<PassType>>& passes);
		virtual ~TTechnique() = default;

		/**	Returns a pass with the specified index. */
		SPtr<PassType> getPass(UINT32 idx) const;

		/**	Returns total number of passes. */
		UINT32 getNumPasses() const { return (UINT32)mPasses.size(); }

		/** Compiles all the passes in a technique. @see Pass::compile. */
		void compile();

	protected:
		Vector<SPtr<PassType>> mPasses;
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
	class BS_CORE_EXPORT Technique : public IReflectable, public CoreObject, public TTechnique<false>
	{
	public:
		Technique(const String& language, const Vector<StringID>& tags, const ShaderVariation& variation, 
			const Vector<SPtr<Pass>>& passes);

		/** Retrieves an implementation of a technique usable only from the core thread. */
		SPtr<ct::Technique> getCore() const;

		/** 
		 * Creates a new technique. 
		 *
		 * @param[in]	language	Shading language used by the technique. The engine will not use this technique unless
		 *							this language is supported by the render API.
		 * @param[in]	passes		A set of passes that define the technique.
		 * @return					Newly creted technique.
		 */
		static SPtr<Technique> create(const String& language, const Vector<SPtr<Pass>>& passes);

		/** 
		 * Creates a new technique. 
		 *
		 * @param[in]	language	Shading language used by the technique. The engine will not use this technique unless
		 *							this language is supported by the render API.
		 * @param[in]	tags		An optional set of tags that can be used for further identifying under which 
		 *							circumstances should a technique be used.
		 * @param[in]	variation	A set of preprocessor directives that were used for compiling this particular technique.
		 *							Used for shaders that have multiple variations.
		 * @param[in]	passes		A set of passes that define the technique.
		 * @return					Newly creted technique.
		 */
		static SPtr<Technique> create(const String& language, const Vector<StringID>& tags,
			const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes);

	protected:
		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/**	Creates a new technique but doesn't initialize it. */
		static SPtr<Technique> createEmpty();

	private:
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
		/** Serialization only constructor. */
		Technique();

	public:
		friend class TechniqueRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;	
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Core thread version of bs::Technique. */
	class BS_CORE_EXPORT Technique : public CoreObject, public TTechnique<true>
	{
	public:
		Technique(const String& language, const Vector<StringID>& tags,
			const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes);

		/** @copydoc bs::Technique::create(const String&, const Vector<SPtr<Pass>>&) */
		static SPtr<Technique> create(const String& language, const Vector<SPtr<Pass>>& passes);

		/** 
		 * @copydoc bs::Technique::create(const String&, const Vector<StringID>&, const ShaderVariation&, const Vector<SPtr<Pass>>&) 
		 */
		static SPtr<Technique> create(const String& language, const Vector<StringID>& tags,
			const ShaderVariation& variation, const Vector<SPtr<Pass>>& passes);
	};

	/** @} */
	}
}
