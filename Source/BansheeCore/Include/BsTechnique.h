//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class that is used for implementing both sim and core versions of Technique. */
	class BS_CORE_EXPORT TechniqueBase
	{
	public:
		TechniqueBase(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags);
		virtual ~TechniqueBase() { }

		/**	Checks if this technique is supported based on current render and other systems. */
		bool isSupported() const;

		/** Checks if the technique has the specified tag. */
		bool hasTag(const StringID& tag);

	protected:
		StringID mRenderAPI;
		StringID mRenderer;
		Vector<StringID> mTags;
	};

	template<bool Core> struct TPassType { };
	template<> struct TPassType < false > { typedef Pass Type; };
	template<> struct TPassType < true > { typedef PassCore Type; };

	template<bool Core> struct TTechniqueType {};
	template<> struct TTechniqueType < false > { typedef Technique Type; };
	template<> struct TTechniqueType < true > { typedef TechniqueCore Type; };

	/** Templated class that is used for implementing both sim and core versions of Technique. */
	template<bool Core>
	class BS_CORE_EXPORT TTechnique : public TechniqueBase
	{
	public:
		typedef typename TPassType<Core>::Type PassType;
		
		TTechnique();
		TTechnique(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
			const Vector<SPtr<PassType>>& passes);
		virtual ~TTechnique() { }

		/**	Returns a pass with the specified index. */
		SPtr<PassType> getPass(UINT32 idx) const;

		/**	Returns total number of passes. */
		UINT32 getNumPasses() const { return (UINT32)mPasses.size(); }

	protected:
		Vector<SPtr<PassType>> mPasses;
	};

	/** @} */

	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Core thread version of Technique. */
	class BS_CORE_EXPORT TechniqueCore : public CoreObjectCore, public TTechnique<true>
	{
	public:
		TechniqueCore(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
			const Vector<SPtr<PassCore>>& passes);

		/** @copydoc Technique::create(const StringID&, const StringID&, const Vector<SPtr<Pass>>&) */
		static SPtr<TechniqueCore> create(const StringID& renderAPI, const StringID& renderer, 
			const Vector<SPtr<PassCore>>& passes);

		/** @copydoc Technique::create(const StringID&, const StringID&, const Vector<StringID>&, const Vector<SPtr<Pass>>&) */
		static SPtr<TechniqueCore> create(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags,
			const Vector<SPtr<PassCore>>& passes);
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
		Technique(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
			const Vector<SPtr<Pass>>& passes);

		/** Retrieves an implementation of a technique usable only from the core thread. */
		SPtr<TechniqueCore> getCore() const;

		/** 
		 * Creates a new technique. 
		 *
		 * @param[in]	renderAPI	Render API the technique supports. Under normal circumstances the engine will not use
		 *							this technique unless this API is enabled.
		 * @param[in]	renderer	Renderer the technique supports. Under normal circumstances the engine will not use
		 *							this technique unless this renderer is enabled.
		 * @param[in]	passes		A set of passes that define the technique.
		 * @return					Newly creted technique.
		 */
		static SPtr<Technique> create(const StringID& renderAPI, const StringID& renderer, const Vector<SPtr<Pass>>& passes);

		/** 
		 * Creates a new technique. 
		 *
		 * @param[in]	renderAPI	Render API the technique supports. Under normal circumstances the engine will not use
		 *							this technique unless this API is enabled.
		 * @param[in]	renderer	Renderer the technique supports. Under normal circumstances the engine will not use
		 *							this technique unless this renderer is enabled.
		 * @param[in]	tags		An optional set of tags that can be used for further identifying under which 
		 *							circumstances should a technique be used.
		 * @param[in]	passes		A set of passes that define the technique.
		 * @return					Newly creted technique.
		 */
		static SPtr<Technique> create(const StringID& renderAPI, const StringID& renderer, const Vector<StringID>& tags, 
			const Vector<SPtr<Pass>>& passes);

	protected:
		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

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
}