//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Material
	 *  @{
	 */

	/**	Allows you to specify defines that can control shader compilation. */
	class B3D_EXPORT ShaderDefines
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

	/** Possible types of a variation parameter. */
	enum class ShaderVariationParameterType
	{
		Int,
		UInt,
		Float,
		Bool
	};

	/** Name, type and value of a variation parameter. */
	struct ShaderVariationParameter
	{
		ShaderVariationParameter()
			: SignedInteger(0), Type(ShaderVariationParameterType::Int)
		{}

		ShaderVariationParameter(const String& name, i32 val)
			: SignedInteger(val), Name(name), Type(ShaderVariationParameterType::Int)
		{}

		ShaderVariationParameter(const String& name, u32 val)
			: UnsignedInteger(val), Name(name), Type(ShaderVariationParameterType::Int)
		{}

		ShaderVariationParameter(const String& name, float val)
			: Float(val), Name(name), Type(ShaderVariationParameterType::Float)
		{}

		ShaderVariationParameter(const String& name, bool val)
			: SignedInteger(val ? 1 : 0), Name(name), Type(ShaderVariationParameterType::Bool)
		{}

		union
		{
			i32 SignedInteger;
			u32 UnsignedInteger;
			float Float;
		};

		StringID Name;
		ShaderVariationParameterType Type;
	};

	/**
	 * Contains information about a single variation of a Shader. Each variation can have a separate set of
	 * \#defines that control shader compilation.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ShaderVariationParameters : public IReflectable, public IScriptExportable
	{
	public:
		B3D_SCRIPT_EXPORT()
		ShaderVariationParameters() = default;

		/** Creates a new shader variation with the specified parameters. */
		ShaderVariationParameters(const TInlineArray<ShaderVariationParameter, 4>& params);

		/**
		 * Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be
		 * found.
		 */
		B3D_SCRIPT_EXPORT()
		i32 GetI32(const StringID& name);

		/**
		 * Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be
		 * found.
		 */
		B3D_SCRIPT_EXPORT()
		u32 GetUI32(const StringID& name);

		/** Returns the value of a float parameter with the specified name. Returns 0 if the parameter cannot be found.  */
		B3D_SCRIPT_EXPORT()
		float GetFloat(const StringID& name);

		/**
		 * Returns the value of a boolean parameter with the specified name. Returns false if the parameter cannot be
		 * found.
		 */
		B3D_SCRIPT_EXPORT()
		bool GetBool(const StringID& name);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		B3D_SCRIPT_EXPORT()
		void SetI32(const StringID& name, i32 value);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		B3D_SCRIPT_EXPORT()
		void SetU32(const StringID& name, u32 value);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		B3D_SCRIPT_EXPORT()
		void SetFloat(const StringID& name, float value);

		/**
		 * Sets the value of the parameter for the provided name. Any previous value for a parameter with the same name
		 * will be overwritten.
		 */
		B3D_SCRIPT_EXPORT()
		void SetBool(const StringID& name, bool value);

		/** Registers a new parameter that controls the variation. */
		void AddParameter(const ShaderVariationParameter& parameter);

		/** Removes a parameter with the specified name. */
		B3D_SCRIPT_EXPORT()
		void RemoveParameter(const StringID& parameter);

		/** Checks if the variation has a parameter with the specified name. */
		B3D_SCRIPT_EXPORT()
		bool HasParameter(const StringID& paramName) { return FindParameter(paramName) != nullptr; }

		/** Removes all parameters. */
		B3D_SCRIPT_EXPORT()
		void ClearParameters() { mParams.clear(); }

		/** Attempts to find a parameter with the provided name, or returns null if not found. */
		const ShaderVariationParameter* FindParameter(const StringID& name) const;

		/** Returns a list of names of all registered parameters. */
		B3D_SCRIPT_EXPORT(ExportName(ParamNames), Property(Getter))
		Vector<String> GetParameters() const;

		/** Creates a unique name created from all parameters and their values. */
		String CreateVariationName() const;

		/**
		 * Checks if this variation matches some other variation.
		 *
		 * @param[in]		other		Other variation to compare it to.
		 * @param[in]		exact		When true both variations need to have the exact number of parameters with identical
		 *								contents, equivalent to the equals operator. When false, only the subset of
		 *								parameters present in @p other is used for comparison, while any extra parameters
		 *								present in this object are ignored.
		 */
		bool Matches(const ShaderVariationParameters& other, bool exact = true) const;

		/** Returns all the variation parameters. */
		const TInlineArray<ShaderVariationParameter, 4>& GetParameterList() const { return mParams; }

		bool operator==(const ShaderVariationParameters& rhs) const;

		bool operator!=(const ShaderVariationParameters& rhs) const { return !operator==(rhs); }

		/** Empty variation with no parameters. */
		static const ShaderVariationParameters kEmpty;

		/**
		 * @name Internal
		 * @{
		 */

		/** Converts all the variation parameters in a ShaderDefines object, that may be consumed by the shader compiler. */
		ShaderDefines GetDefines() const;

		/**
		 * Returns a unique index of this variation, relative to all other variations registered in ShaderVariations object.
		 */
		u32 GetIndex() const { return mIndex; }

		/** Assigns a unique index to the variation that can later be used for quick lookup. */
		void SetIndex(u32 idx) const { mIndex = idx; }

		/** @} */
	private:
		friend class ShaderVariations;

		/** Non-const overload of FindParameter() const. */
		ShaderVariationParameter* FindParameter(const StringID& name);

		TInlineArray<ShaderVariationParameter, 4> mParams;
		mutable u32 mIndex = -1;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ShaderVariationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** A container for all variations of a single Shader. */
	class B3D_EXPORT ShaderVariations
	{
	public:
		/** Registers a new variation. */
		void Add(const ShaderVariationParameters& variation);

		/** Returns a variation at the specified index. Variations are indexed sequentially as they are added. */
		const ShaderVariationParameters& Get(u32 idx) { return mVariations[idx]; }

		/**
		 * Scans a list of stored variations and returns an index of a variation that has the same parameters as the
		 * provided one, or -1 if one is not found.
		 */
		u32 Find(const ShaderVariationParameters& variation) const;

		/** Returns a list of all variations. */
		const TInlineArray<ShaderVariationParameters, 4>& GetVariations() const { return mVariations; }

		/** Clears all the shader variations from the set. */
		void Clear()
		{
			mVariations.clear();
			mNextIdx = 0;
		}

		/** Returns true if the object contains zero variations. */
		bool IsEmpty() const { return mVariations.Empty(); }

	private:
		TInlineArray<ShaderVariationParameters, 4> mVariations;
		u32 mNextIdx = 0;
	};

	/** @} */
} // namespace b3d
