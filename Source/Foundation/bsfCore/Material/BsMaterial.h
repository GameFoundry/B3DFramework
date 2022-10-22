//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Resources/BsIResourceListener.h"
#include "Material/BsMaterialParam.h"
#include "Material/BsMaterialParams.h"
#include "Material/BsTechnique.h"
#include "Animation/BsAnimationCurve.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsMatrix3.h"
#include "Math/BsMatrix4.h"

namespace bs
{
	template<class T>
	class TAnimationCurve;
	class ColorGradient;

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Flags that signal in what way did the Material change. */
	enum class MaterialDirtyFlags
	{
		/** Material parameter changed. */
		Param				= 1 << 0,
		/** Dependant resource has been loaded and/or changed. (e.g. a texture assigned to a parameter. */
		ParamResource		= 1 << 1,
		/** Material shader has changed. */
		Shader				= 2 << 2
	};

	/** Structure used when searching for a specific technique in a Material. */
	struct FIND_TECHNIQUE_DESC
	{
		static constexpr u32 MAX_NUM_TAGS = 10;

		/** A set of tags that the technique must have. */
		StringID Tags[MAX_NUM_TAGS];

		/** Number of valid tags in the @p tags array. */
		u32 NumTags = 0;

		/** Specified variation of the technique. Parameters not specified in the variation are assumed to be irrelevant. */
		const ShaderVariation* Variation = nullptr;

		/**
		 * Determines should the parameters in @p variation override any parameters that might have been defined on the
		 * Material itself. If false then you are guaranteed to search only over the subset of techniques that match the
		 * Material's internal variaton parameters. If true then you can search outside that range by setting a variation
		 * parameter to some different value. Overriding can be useful for renderers which might need to override the user's
		 * choice of variation.
		 */
		bool Override = false;

		/** Registers a new tag to look for when searching for the technique. */
		void AddTag(const StringID& tag)
		{
			BS_ASSERT(NumTags < MAX_NUM_TAGS);

			Tags[NumTags] = tag;
			NumTags++;
		}
	};

	/**
	 * Material that controls how objects are rendered. It is represented by a shader and parameters used to set up that
	 * shader. It provides a simple interface for manipulating the parameters.
	 */
	class BS_CORE_EXPORT MaterialBase
	{
	public:
		/** Data used to describe a structure defined within a shader. */
		struct StructData
		{
			StructData()
				:Data(nullptr), Size(0)
			{ }

			StructData(u32 _size)
				:Size(_size)
			{
				Data = std::shared_ptr<void>(bs_alloc(_size), (void(*)(void*))&bs_free);
			}

			/**
			 * Writes the specified data to the internal buffer. Caller must ensure size of the provided buffer is correct.
			 */
			void Write(void* _data)
			{
				memcpy(Data.get(), _data, Size);
			}

			SPtr<void> Data;
			u32 Size;
		};

		MaterialBase() = default;
		virtual ~MaterialBase() = default;

		/** @name Internal
		 *  @{
		 */

		/** Marks the contents of the sim thread object as dirty, causing it to sync with its core thread counterpart. */
		virtual void MarkCoreDirtyInternal(MaterialDirtyFlags flags = MaterialDirtyFlags::Param) { }

		/** @copydoc CoreObject::markDependenciesDirty */
		virtual void MarkDependenciesDirtyInternal() { }

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		virtual void MarkResourcesDirtyInternal() { }

		/** @} */
	};

	/** @copydoc MaterialBase */
	template<bool Core>
	class BS_CORE_EXPORT TMaterial : public MaterialBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, Core>;
		using SpriteTextureType = CoreVariantHandleType<SpriteTexture, Core>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;
		using SamplerStateType = SPtr<CoreVariantType<SamplerState, Core>>;
		using GpuProgramPtrType = SPtr<CoreVariantType<GpuProgram, Core>>;
		using PassType = CoreVariantType<Pass, Core>;
		using TechniqueType = CoreVariantType<Technique, Core>;
		using ShaderType = CoreVariantHandleType<Shader, Core>;
		using GpuParamsSetType = CoreVariantType<GpuParamsSet, Core>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;

		TMaterial() = default;
		virtual ~TMaterial() = default;

		/** Returns the currently active shader. */
		BS_SCRIPT_EXPORT(n:Shader,pr:getter)
		ShaderType GetShader() const { return mShader; }

		/**
		 * Set of parameters that determine which subset of techniques in the assigned shader should be used. Only the
		 * techniques that have the provided parameters with the provided values will match. This will control which
		 * technique is considered the default technique and which subset of techniques are searched during a call to
		 * findTechnique().
		 */
		BS_SCRIPT_EXPORT(n:Variation,pr:getter)
		const ShaderVariation& GetVariation() const { return mVariation; }

		/** Returns the total number of techniques supported by this material. */
		u32 GetNumTechniques() const { return (u32)mTechniques.size(); }

		/** Returns the technique at the specified index. */
		const SPtr<TechniqueType>& GetTechnique(u32 idx) const { return mTechniques[idx]; }

		/**
		 * Attempts to find a technique matching the specified variation and tags among the supported techniques.
		 *
		 * @param[in]	desc				Object containing an optional set of tags and a set of variation parameters to
		 *									look for.
		 * @return							First technique that matches the tags & variation parameters specified in
		 *									@p desc.
		 */
		u32 FindTechnique(const FIND_TECHNIQUE_DESC& desc) const;

		/**
		 * Finds the index of the default (primary) technique to use. This will be the first technique that matches the
		 * currently set variation parameters (if any).
		 */
		u32 GetDefaultTechnique() const;

		/**
		 * Returns the number of passes that are used by the technique at the specified index.
		 *
		 * @param[in]	techniqueIdx	Index of the technique to retrieve the number of passes for. 0 is always guaranteed
		 *								to be the default technique.
		 * @return						Number of passes used by the technique.
		 */
		u32 GetNumPasses(u32 techniqueIdx = 0) const;

		/**
		 * Retrieves a specific shader pass from the provided technique.
		 *
		 * @param[in]	passIdx			Sequential index of the pass to retrieve.
		 * @param[in]	techniqueIdx	Index of the technique to retrieve the pass for. 0 is always guaranteed to be
		 *								the default technique.
		 * @return						Pass if found, null otherwise.
		 */
		SPtr<PassType> GetPass(u32 passIdx = 0, u32 techniqueIdx = 0) const;

		/**
		 * Creates a set of GpuParams that may be used for binding material parameters to the GPU. The expected behaviour
		 * is to create a set of GpuParams per-technique once, and then before binding them to the GPU call
		 * UpdateParamsSet() to ensure any dirty parameters are transfered from the material to GpuParams. You may also
		 * use the parameter set to manually modify parameters on a per-program basis, in which case no further updates from
		 * the material are necessary.
		 */
		SPtr<GpuParamsSetType> CreateParamsSet(u32 techniqueIdx = 0);

		/**
		 * Copies internal material parameter data to the provided params set.
		 *
		 * @param[in]	paramsSet		Parameter set to update.
		 * @param[in]	t				Time to evaluate animated parameters at (if any are present).
		 * @param[in]	updateAll		Normally the system will track dirty parameters since the last call to this method
		 *								(on a per-set basis), and only update the dirty ones. Set this to true if you want
		 *								to force all parameters to update, regardless of their dirty state.
		 */
		void UpdateParamsSet(const SPtr<GpuParamsSetType>& paramsSet, float t = 0.0f, bool updateAll = false);

		/**
		 * Assigns a float value to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT()
		void SetFloat(const String& name, float value, u32 arrayIdx = 0)	{ return GetParamFloat(name).Set(value, arrayIdx); }

		/*
		 * Assigns a curve to the the float shader parameter with the specified name. The system will automatically
		 * evaluate the curve with the passage of time and apply the evaluated value to the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT()
		void SetFloatCurve(const String& name, TAnimationCurve<float> value, u32 arrayIdx = 0)
		{ return GetParamFloatCurve(name).Set(std::move(value), arrayIdx); }

		/**
		 * Assigns a color to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT()
		void SetColor(const String& name, const Color& value, u32 arrayIdx = 0) { return GetParamColor(name).Set(value, arrayIdx); }

		/**
		 * Assigns a color gradient to the shader parameter with the specified name. The system will automatically
		 * evaluate the gradient with the passage of time and apply the evaluated value to the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT()
		void SetColorGradient(const String& name, const ColorGradientHDR& value, u32 arrayIdx = 0)
		{ return GetParamColorGradient(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 2D vector to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetVector2)
		void SetVec2(const String& name, const Vector2& value, u32 arrayIdx = 0)	{ return GetParamVec2(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 3D vector to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetVector3)
		void SetVec3(const String& name, const Vector3& value, u32 arrayIdx = 0)	{ return GetParamVec3(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 4D vector to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetVector4)
		void SetVec4(const String& name, const Vector4& value, u32 arrayIdx = 0)	{ return GetParamVec4(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 3x3 matrix to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetMatrix3)
		void SetMat3(const String& name, const Matrix3& value, u32 arrayIdx = 0)	{ return GetParamMat3(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 4x4 matrix to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetMatrix4)
		void SetMat4(const String& name, const Matrix4& value, u32 arrayIdx = 0)	{ return GetParamMat4(name).Set(value, arrayIdx); }

		/**
		 * Assigns a structure to the shader parameter with the specified name.
		 *  		
		 * Structure is provided as a raw buffer and caller must ensure structure in buffer matches what the shader expects.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void SetStructData(const String& name, void* value, u32 size, u32 arrayIdx = 0) { return GetParamStruct(name).Set(value, size, arrayIdx); }

		/** Assigns a texture to the shader parameter with the specified name. */
		void SetTexture(const String& name, const TextureType& value, const TextureSurface& surface = TextureSurface::COMPLETE)
		{
			return GetParamTexture(name).Set(value, surface);
		}

		/**
		 * Assigns a sprite texture to the shader parameter with the specified name. If the sprite texture contains
		 * animation it will be automatically evaluated every frame.
		 *
		 * @note
		 * In order for the sprite sub-image to be properly applied the shader needs to have a 4D vector parameter marked
		 * with the SpriteUV attribute referencing this parameter. This vector will then receive the necessary UV offset
		 * and size which should be utilized by the shader code to render a subset of the texture as defined in the sprite
		 * texture.
		 */
		void SetSpriteTexture(const String& name, const SpriteTextureType& value)
		{
			return GetParamSpriteTexture(name).Set(value);
		}

		/** Assigns a texture to be used for random load/store operations to the shader parameter with the specified name. */
		void SetLoadStoreTexture(const String& name, const TextureType& value, const TextureSurface& surface)
		{
			return GetParamLoadStoreTexture(name).Set(value, surface);
		}

		/** Assigns a buffer to the shader parameter with the specified name. */
		void SetBuffer(const String& name, const BufferType& value) { return GetParamBuffer(name).Set(value); }

		/** Assigns a sampler state to the shader parameter with the specified name. */
		void SetSamplerState(const String& name, const SamplerStateType& value) { return GetParamSamplerState(name).Set(value); }

		/**
		 * Returns a float value assigned with the parameter with the specified name. If a curve is assigned to this
		 * parameter, returns the curve value evaluated at time 0. Use getBoundParamType() to determine
		 * the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT()
		float GetFloat(const String& name, u32 arrayIdx = 0) const { return GetParamFloat(name).Get(arrayIdx); }

		/**
		 * Returns a curve value assigned to the parameter with the specified name. If the parameter has a constant
		 * value bound instead of a curve then this method returns an empty curve. Use getBoundParamType() to determine
		 * the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT()
		const TAnimationCurve<float>& GetFloatCurve(const String& name, u32 arrayIdx = 0) const
		{ return GetParamFloatCurve(name).Get(arrayIdx); }

		/**
		 * Returns a color assigned with the parameter with the specified name. If a color gradient is assigned to this
		 * parameter, returns the gradient color evaluated at time 0. Use getBoundParamType() to determine
		 * the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT()
		Color GetColor(const String& name, u32 arrayIdx = 0) const { return GetParamColor(name).Get(arrayIdx); }

		/**
		 * Returns a color gradient assigned with the parameter with the specified name. If the parameter has a constant
		 * value bound instead of a gradient then this method returns an empty gradient. Use getBoundParamType() to
		 * determine the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT()
		const ColorGradientHDR& GetColorGradient(const String& name, u32 arrayIdx = 0) const
		{ return GetParamColorGradient(name).Get(arrayIdx); }

		/**
		 * Returns a 2D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetVector2)
		Vector2 GetVec2(const String& name, u32 arrayIdx = 0) const { return GetParamVec2(name).Get(arrayIdx); }

		/**
		 * Returns a 3D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetVector3)
		Vector3 GetVec3(const String& name, u32 arrayIdx = 0) const { return GetParamVec3(name).Get(arrayIdx); }

		/**
		 * Returns a 4D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetVector4)
		Vector4 GetVec4(const String& name, u32 arrayIdx = 0) const { return GetParamVec4(name).Get(arrayIdx); }

		/**
		 * Returns a 3x3 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetMatrix3)
		Matrix3 GetMat3(const String& name, u32 arrayIdx = 0) const { return GetParamMat3(name).Get(arrayIdx); }

		/**
		 * Returns a 4x4 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetMatrix4)
		Matrix4 GetMat4(const String& name, u32 arrayIdx = 0) const { return GetParamMat4(name).Get(arrayIdx); }

		/**
		 * Checks does the data parameter with the specified name currently contains animated data. This could be
		 * an animation curve or a color gradient.
		 */
		BS_SCRIPT_EXPORT()
		bool IsAnimated(const String& name, u32 arrayIdx = 0);

		/** Returns a texture assigned with the parameter with the specified name. */
		TextureType GetTexture(const String& name) const { return GetParamTexture(name).Get(); }

		/**
		 * Returns a sprite texture assigned to the parameter with the specified name. If the parameter has a regular
		 * texture attached instead of a sprite texture, null will be returned. Use getBoundParamType() to determine
		 * the type of the parameter.
		 */
		SpriteTextureType GetSpriteTexture(const String& name) const { return GetParamSpriteTexture(name).Get(); }

		/** Returns a sampler state assigned with the parameter with the specified name. */
		SamplerStateType GetSamplerState(const String& name) const	{ return GetParamSamplerState(name).Get(); }

		/**
		 * Returns a buffer representing a structure assigned to the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		MaterialBase::StructData GetStructData(const String& name, u32 arrayIdx = 0) const
		{
			TMaterialParamStruct<Core> structParam = GetParamStruct(name);

			MaterialBase::StructData data(structParam.GetElementSize());
			structParam.Get(data.Data.get(), structParam.GetElementSize(), arrayIdx);

			return data;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a floating point parameter. This handle
		 * may be used for more efficiently getting/setting GPU parameter values than calling
		 * Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<float, Core> GetParamFloat(const String& name) const
		{
			TMaterialDataParam<float, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a time-varying curve to a floating point parameter. This
		 * handle may be used for more efficiently getting/setting GPU parameter values than calling
		 * Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialCurveParam<float, Core> GetParamFloatCurve(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a constant value to a color parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::get* / Material::set*
		 * methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material,
		 * and then use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Color, Core> GetParamColor(const String& name) const
		{
			TMaterialDataParam<Color, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a time-varying gradient to a color parameter. This handle
		 * may be used for more efficiently getting/setting GPU parameter values than calling
		 * Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material,
		 * and then use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialColorGradientParam<Core> GetParamColorGradient(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a constant value to a 2D vector parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::get* / Material::set*
		 * methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Vector2, Core> GetParamVec2(const String& name) const
		{
			TMaterialDataParam<Vector2, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 3D vector parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::get* / Material::set*
		 * methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Vector3, Core> GetParamVec3(const String& name) const
		{
			TMaterialDataParam<Vector3, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 4D vector parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::get* / Material::set*
		 * methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Vector4, Core> GetParamVec4(const String& name) const
		{
			TMaterialDataParam<Vector4, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 3x3 matrix parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::get* / Material::set*
		 * methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Matrix3, Core> GetParamMat3(const String& name) const
		{
			TMaterialDataParam<Matrix3, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 4x4 matrix parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::get* / Material::set*
		 * methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Matrix4, Core> GetParamMat4(const String& name) const
		{
			TMaterialDataParam<Matrix4, Core> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a structure GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamStruct<Core> GetParamStruct(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a texture GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamTexture<Core> GetParamTexture(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a sprite texture GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamSpriteTexture<Core> GetParamSpriteTexture(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a load-store texture GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamLoadStoreTexture<Core> GetParamLoadStoreTexture(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a buffer GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamBuffer<Core> GetParamBuffer(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a sampler state GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::get* / Material::set* methods.
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamSampState<Core> GetParamSamplerState(const String& name) const;

		/**
		 * Allows you to retrieve a handle to a parameter that you can then use for quickly setting and retrieving parameter
		 * data. This allows you to set/get parameter data without all the cost of extra lookups otherwise required.
		 * 			
		 * @note	
		 * All of these handles will be invalidated if material shader ever changes. It is up to the caller to keep track
		 * of that.
		 */
		template <typename T>
		void GetParam(const String& name, TMaterialDataParam<T, Core>& output) const;

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Returns an object containg all of material's parameters. Allows the caller to manipulate the parameters more
		 * directly.
		 */
		SPtr<MaterialParamsType> GetInternalParamsInternal() const { return mParams; }

		/** @} */
	protected:
		/**
		 * Assigns a value from a raw buffer to the parameter with the specified name. Buffer must be of sizeof(T) *
		 * numElements size and initialized.
		 *
		 * @note	Provided parameter must exist, no checking is done.
		 */
		template <typename T>
		void SetParamValue(const String& name, u8* buffer, u32 numElements);

		/**
		 * Initializes the material by using the compatible techniques from the currently set shader. Shader must contain
		 * the techniques that matches the current renderer and render system.
		 */
		void InitializeTechniques();

		/** Assigns all the default parameters specified in the shader to the material. */
		void InitDefaultParameters();

		/** Throw an exception if no shader is set, or no acceptable technique was found. */
		void ThrowIfNotInitialized() const;

		ShaderType mShader;
		SPtr<MaterialParamsType> mParams;
		Vector<SPtr<TechniqueType>> mTechniques;
		ShaderVariation mVariation;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Material: public Resource, public TMaterial<false>, public IResourceListener
	{
	public:
		~Material() = default;

		/**
		 * Sets a shader that will be used by the material. Material will be initialized using all compatible techniques
		 * from the shader. Shader must be set before doing any other operations with the material.
		 */
		BS_SCRIPT_EXPORT(n:Shader,pr:setter)
		void SetShader(const HShader& shader);

		/** @copydoc TMaterial<Core>::GetVariation() const */
		BS_SCRIPT_EXPORT(n:Variation,pr:setter,hide)
		void SetVariation(const ShaderVariation& variation);

		/** Retrieves an implementation of a material usable only from the core thread. */
		SPtr<ct::Material> GetCore() const;

		/** @copydoc CoreObject::initialize */
		void Initialize() override;

		/** Creates a deep copy of the material and returns the new object. */
		BS_SCRIPT_EXPORT(n:Clone)
		HMaterial Clone();

		/**
		 * Creates a new empty material.
		 * 			
		 * @note	Make sure you call Material::setShader before using it.
		 */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(Material))
		static HMaterial Create();

		/** Creates a new material with the specified shader. */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(Material))
		static HMaterial Create(const HShader& shader);

		/**
		 * Creates a new material with the specified shader, and a set of parameters that determine which subset of
		 * techniques in the shader should the material use.
		 */
		static HMaterial Create(const HShader& shader, const ShaderVariation& variation);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Marks the core data as dirty. This causes the syncToCore() method to trigger the next time objects are synced
		 * between core and sim threads.
		 */
		void MarkCoreDirtyInternal(MaterialDirtyFlags flags = MaterialDirtyFlags::Param) override;

		/** @copydoc CoreObject::markDependenciesDirty */
		void MarkDependenciesDirtyInternal() override;

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		void MarkResourcesDirtyInternal() override;

		/** @} */
	private:
		Material();
		Material(const HShader& shader, const ShaderVariation& variation);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> CreateCore() const ;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) ;

		/** @copydoc IResourceListener::getListenerResources */
		void GetListenerResources(Vector<HResource>& resources) ;

		/** @copydoc IResourceListener::notifyResourceLoaded */
		void NotifyResourceLoaded(const HResource& resource) override;

		/** @copydoc IResourceListener::notifyResourceChanged */
		void NotifyResourceChanged(const HResource& resource) override;

		/**	Performs material initialization when all resources are ready. */
		void InitializeIfLoaded();

		/**
		 * Uses the provided list of parameters to try to set every parameter in this material. Parameter whose name, type
		 * or size don't match are ignored and will not be set.
		 */
		void SetParams(const SPtr<MaterialParams>& params);

		/**	Creates a new empty material but doesn't initialize it. */
		static SPtr<Material> CreateEmpty();

		u32 mLoadFlags;
		
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
	public:
		friend class MaterialRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class BS_CORE_EXPORT Material : public CoreObject, public TMaterial<true>
	{
	public:
		~Material() = default;

		/** @copydoc bs::Material::setShader */
		void SetShader(const SPtr<Shader>& shader);

		/**
		 * Set of parameters that determine which subset of techniques in the assigned shader should be used. Only the
		 * techniques that have the provided parameters with the provided values will match. This will control which
		 * technique is considered the default technique and which subset of techniques are searched during a call to
		 * findTechnique().
		 */
		void SetVariation(const ShaderVariation& variation);

		/** Creates a new material with the specified shader. */
		static SPtr<Material> Create(const SPtr<Shader>& shader);
	private:
		friend class bs::Material;

		Material() = default;
		Material(const SPtr<Shader>& shader, const ShaderVariation& variation);
		Material(const SPtr<Shader>& shader, const Vector<SPtr<Technique>>& techniques,
			const SPtr<MaterialParams>& materialParams, const ShaderVariation& variation);

		/** @copydoc CoreObject::syncToCore */
		void SyncToCore(const CoreSyncData& data) override;
	};

	/** @} */	
	}
}
