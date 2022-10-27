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
		void Set(const String& name, float value);

		/** Adds a new define with an integer value. */
		void Set(const String& name, i32 value);

		/** Adds a new define with an integer value. */
		void Set(const String& name, u32 value);

		/** Adds a new define with a string point value. */
		void Set(const String& name, const String& value);

		/**	Returns a list of all defines. */
		UnorderedMap<String, String> GetAll() const { return mDefines; }

		/** Removes all defines. */
		void Clear() { mDefines.clear(); }

	protected:
		UnorderedMap<String, String> mDefines;
	};

	/**
	 * Contains information about a single variation of a Shader. Each variation can have a separate set of
	 * \#defines that control shader compilation.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ShaderVariation : public IReflectable
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
				: I(0), Type(Int)
			{}

			Param(const String& name, i32 val)
				: I(val), Name(name), Type(Int)
			{}

			Param(const String& name, u32 val)
				: Ui(val), Name(name), Type(Int)
			{}

			Param(const String& name, float val)
				: F(val), Name(name), Type(Float)
			{}

			Param(const String& name, bool val)
				: I(val ? 1 : 0), Name(name), Type(Bool)
			{}

			union
			{
				i32 I;
				u32 Ui;
				float F;
			};

			StringID Name;
			ParamType Type;
		};

		BS_SCRIPT_EXPORT()
		ShaderVariation() = default;

		/** Creates a new shader variation with the specified parameters. */
		ShaderVariation(const SmallVector<Param, 4>& params);

		/**
		 * Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be
		 * found.
		 */
		BS_SCRIPT_EXPORT()
		i32 GetInt(const StringID& name);

		/**
		 * Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be
		 * found.
		 */
		BS_SCRIPT_EXPORT()
		u32 GetUInt(const StringID& name);

		/** Returns the value of a float parameter with the specified name. Returns 0 if the parameter cannot be found.  */
		BS_SCRIPT_EXPORT()
		float GetFloat(const StringID& name);

		/**
		 * Returns the value of a boolean parameter with the specified name. Returns false if the parameter cannot be
		 * found.
		 */
		BS_SCRIPT_EXPORT()
		bool GetBool(const StringID& name);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		BS_SCRIPT_EXPORT()
		void SetInt(const StringID& name, i32 value);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		BS_SCRIPT_EXPORT()
		void SetUInt(const StringID& name, u32 value);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		BS_SCRIPT_EXPORT()
		void SetFloat(const StringID& name, float value);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		BS_SCRIPT_EXPORT()
		void SetBool(const StringID& name, bool value);

		/** Registers a new parameter that controls the variation. */
		void AddParam(const Param& param) { mParams[param.Name] = param; }

		/** Removes a parameter with the specified name. */
		BS_SCRIPT_EXPORT()

		void RemoveParam(const StringID& paramName) { mParams.erase(paramName); }

		/** Checks if the variation has a parameter with the specified name. */
		BS_SCRIPT_EXPORT()

		bool HasParam(const StringID& paramName) { return mParams.find(paramName) != mParams.end(); }

		/** Removes all parameters. */
		BS_SCRIPT_EXPORT()

		void ClearParams() { mParams.clear(); }

		/** Returns a list of names of all registered parameters. */
		BS_SCRIPT_EXPORT(ExportName(ParamNames), Property(Getter))
		Vector<String> GetParamNames() const;

		/**
		 * Checks if this variation matches some other variation.
		 *
		 * @param[in]		other		Other variation to compare it to.
		 * @param[in]		exact		When true both variations need to have the exact number of parameters with identical
		 *								contents, equivalent to the equals operator. When false, only the subset of
		 *								parameters present in @p other is used for comparison, while any extra parameters
		 *								present in this object are ignored.
		 */
		bool Matches(const ShaderVariation& other, bool exact = true) const;

		/** Returns all the variation parameters. */
		const UnorderedMap<StringID, Param>& GetParams() const { return mParams; }

		bool operator==(const ShaderVariation& rhs) const;

		/** Empty variation with no parameters. */
		static const ShaderVariation kEmpty;

		/**
		 * @name Internal
		 * @{
		 */

		/** Converts all the variation parameters in a ShaderDefines object, that may be consumed by the shader compiler. */
		ShaderDefines GetDefines() const;

		/**
		 * Returns a unique index of this variation, relative to all other variations registered in ShaderVariations object.
		 */
		u32 GetIdx() const { return mIdx; }

		/** Assigns a unique index to the variation that can later be used for quick lookup. */
		void SetIdx(u32 idx) const { mIdx = idx; }

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P p)
		{
			p(mParams);
		}

		/** @} */
	private:
		friend class ShaderVariations;

		UnorderedMap<StringID, Param> mParams;
		mutable u32 mIdx = -1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ShaderVariationRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** A container for all variations of a single Shader. */
	class BS_CORE_EXPORT ShaderVariations
	{
	public:
		/** Registers a new variation. */
		void Add(const ShaderVariation& variation);

		/** Returns a variation at the specified index. Variations are indexed sequentially as they are added. */
		const ShaderVariation& Get(u32 idx) { return mVariations[idx]; }

		/**
		 * Scans a list of stored variations and returns an index of a variation that has the same parameters as the
		 * provided one, or -1 if one is not found.
		 */
		u32 Find(const ShaderVariation& variation) const;

		/** Returns a list of all variations. */
		const SmallVector<ShaderVariation, 4>& GetVariations() const { return mVariations; }

	private:
		SmallVector<ShaderVariation, 4> mVariations;
		u32 mNextIdx = 0;
	};

	/** @} */
} // namespace bs
