//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**	Allows you to specify defines that can control shader compilation. */
	class BS_CORE_EXPORT ShaderDefines
	{
	public:
		/** Adds a new define with a floating point value. */
		void set(const String& name, float value);

		/** Adds a new define with an integer value. */
		void set(const String& name, INT32 value);

		/** Adds a new define with an integer value. */
		void set(const String& name, UINT32 value);

		/** Adds a new define with a string point value. */
		void set(const String& name, const String& value);

		/**	Returns a list of all defines. */
		UnorderedMap<String, String> getAll() const { return mDefines; }

		/** Removes all defines. */
		void clear() { mDefines.clear(); }

	protected:
		UnorderedMap<String, String> mDefines;
	};

	/** 
	 * Contains information about a single variation of a Shader. Each variation can have a separate set of
	 * \#defines that control shader compilation.
	 */
	class BS_CORE_EXPORT ShaderVariation : public IReflectable
	{
	public:
		/** Possible types of a variation parameter. */
		enum ParamType
		{
			Int,
			UInt,
			Float,
			Bool
		};

		/** Name, type and value of a variation parameter. */
		struct Param
		{
			Param()
				:i(0), type(Int)
			{ }

			Param(const String& name, INT32 val)
				:i(val), name(name), type(Int)
			{ }

			Param(const String& name, UINT32 val)
				:ui(val), name(name), type(Int)
			{ }

			Param(const String& name, float val)
				:f(val), name(name), type(Float)
			{ }

			Param(const String& name, bool val)
				:i(val ? 1 : 0), name(name), type(Bool)
			{ }

			union
			{
				INT32 i;
				UINT32 ui;
				float f;
			};

			StringID name;
			ParamType type;
		};

		ShaderVariation() = default;
		
		/** Creates a new shader variation with the specified parameters. */
		ShaderVariation(const SmallVector<Param, 4>& params);

		/** 
		 * Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be 
		 * found.
		 */
		INT32 getInt(const StringID& name);

		/** 
		 * Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be
		 * found.
		 */
		UINT32 getUInt(const StringID& name);

		/** Returns the value of a float parameter with the specified name. Returns 0 if the parameter cannot be found.  */
		float getFloat(const StringID& name);

		/** 
		 * Returns the value of a boolean parameter with the specified name. Returns false if the parameter cannot be 
		 * found.
		 */
		bool getBool(const StringID& name);

		/** Registers a new parameter that controls the variation. */
		void addParam(const Param& param) { mParams[param.name] = param; }

		/** Removes a parameter with the specified name. */
		void removeParam(const StringID& paramName) { mParams.erase(paramName); }

		/** Removes all parameters. */
		void clearParams() { mParams.clear(); }

		/** 
		 * Checks if this variation matches some other variation. 
		 * 
		 * @param[in]		other		Other variation to compare it to.
		 * @param[in]		exact		When true both variations need to have the exact number of parameters with identical
		 *								contents, equivalent to the equals operator. When false, only the subset of 
		 *								parameters present in @p other is used for comparison, while any extra parameters 
		 *								present in this object are ignored.
		 */
		bool matches(const ShaderVariation& other, bool exact = true) const;

		/** Returns all the variation parameters. */
		const UnorderedMap<StringID, Param>& getParams() const { return mParams; }

		bool operator==(const ShaderVariation& rhs) const;

		/** Empty variation with no parameters. */
		static const ShaderVariation EMPTY;

		/**
		 * @name Internal
		 * @{
		 */

		/** Converts all the variation parameters in a ShaderDefines object, that may be consumed by the shader compiler. */
		ShaderDefines getDefines() const;

		/** 
		 * Returns a unique index of this variation, relative to all other variations registered in ShaderVariations object.
		 */
		UINT32 getIdx() const { return mIdx;  }

		/** Assigns a unique index to the variation that can later be used for quick lookup. */
		void setIdx(UINT32 idx) const { mIdx = idx; }

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p)
		{
			p(mParams);
		}

		/** @} */
	private:
		friend class ShaderVariations;

		UnorderedMap<StringID, Param> mParams;
		mutable UINT32 mIdx = -1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
	public:
		friend class ShaderVariationRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;	
	};

	/** A container for all variations of a single Shader. */
	class BS_CORE_EXPORT ShaderVariations
	{
	public:
		/** Registers a new variation. */
		void add(const ShaderVariation& variation);

		/** Returns a variation at the specified index. Variations are indexed sequentially as they are added. */
		const ShaderVariation& get(UINT32 idx) { return mVariations[idx]; }

		/** 
		 * Scans a list of stored variations and returns an index of a variation that has the same parameters as the
		 * provided one, or -1 if one is not found.
		 */
		UINT32 find(const ShaderVariation& variation) const;

		/** Returns a list of all variations. */
		const SmallVector<ShaderVariation, 4>& getVariations() const { return mVariations; }

	private:
		SmallVector<ShaderVariation, 4> mVariations;
		UINT32 mNextIdx = 0;
	};

	/** @} */
}
