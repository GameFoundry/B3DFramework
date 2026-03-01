//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"
#include "Resources/B3DIResourceListener.h"
#include "Material/B3DMaterialParam.h"
#include "Material/B3DMaterialParameters.h"
#include "Material/B3DVariation.h"
#include "Animation/B3DAnimationCurve.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DMatrix4.h"

namespace b3d
{
	template <class T>
	class TAnimationCurve;
	class ColorGradient;

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Flags that signal in what way did the Material change. */
	enum class MaterialDirtyFlags
	{
		/** Material parameter changed. */
		Parameter = 1 << 0,
		/** Dependant resource has been loaded and/or changed. (e.g. a texture assigned to a parameter. */
		ParameterResource = 1 << 1,
		/** Material shader has changed. */
		Shader = 2 << 2
	};

	/** Structure used when searching for a specific variation in a Material. */
	struct FindVariationInformation
	{
		/** Specified variation parameters. Parameters not specified are assumed to be irrelevant. */
		const ShaderVariationParameters* VariationParameters = nullptr;

		/**
		 * Determines should the parameters in @p VariationParameters override any parameters that might have been defined on the
		 * Material itself. If false then you are guaranteed to search only over the subset of variations that match the
		 * Material's internal variation parameters. If true then you can search outside that range by setting a variation
		 * parameter to some different value. Overriding can be useful for renderers which might need to override the user's
		 * choice of variation.
		 */
		bool Override = false;
	};

	/**
	 * Material that controls how objects are rendered. It is represented by a shader and parameters used to set up that
	 * shader. It provides a simple interface for manipulating the parameters.
	 */
	class B3D_EXPORT MaterialBase
	{
	public:
		/** Data used to describe a structure defined within a shader. */
		struct StructData
		{
			StructData()
				: Data(nullptr), Size(0)
			{}

			StructData(u32 _size)
				: Size(_size)
			{
				Data = std::shared_ptr<void>(B3DAllocate(_size), (void (*)(void*)) & B3DFree);
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

		/** Marks the contents of the main thread object as dirty, causing it to sync with the render proxy. */
		virtual void MarkRenderProxyDataDirtyInternal(MaterialDirtyFlags flags = MaterialDirtyFlags::Parameter) {}

		/** @copydoc CoreObject::MarkDependenciesDirty */
		virtual void MarkDependenciesDirtyInternal() {}

		/** @copydoc IResourceListener::MarkListenerResourcesDirty */
		virtual void MarkResourcesDirtyInternal() {}

		/** @} */
	};

	/** @copydoc MaterialBase */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterial : public MaterialBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using SpriteImageType = CoreVariantHandleType<SpriteImage, IsRenderProxy>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;
		using PassType = CoreVariantType<Pass, IsRenderProxy>;
		using VariationType = CoreVariantType<Variation, IsRenderProxy>;
		using ShaderType = CoreVariantHandleType<Shader, IsRenderProxy>;
		using MaterialParameterAdapterType = CoreVariantType<MaterialParameterAdapter, IsRenderProxy>;
		using MaterialParametersType = CoreVariantType<MaterialParameters, IsRenderProxy>;

		TMaterial() = default;
		virtual ~TMaterial() = default;

		/** Returns the currently active shader. */
		B3D_SCRIPT_EXPORT(ExportName(Shader), Property(Getter))
		ShaderType GetShader() const { return mShader; }

		/**
		 * Set of parameters that determine which subset of variations in the assigned shader should be used. Only the
		 * variations that have the provided parameters with the provided values will match. This will control which
		 * variation is considered the default variation and which subset of variations are searched during a call to
		 * FindVariation().
		 */
		B3D_SCRIPT_EXPORT(ExportName(Variation), Property(Getter))
		const ShaderVariationParameters& GetVariationParameters() const { return mVariationParameters; }

		/** Returns the total number of variations supported by this material. */
		u32 GetVariationCount() const { return (u32)mVariations.size(); }

		/** Returns the variation at the specified index. */
		const SPtr<VariationType>& GetVariation(u32 index) const { return mVariations[index]; }

		/**
		 * Attempts to find a variation matching the specified variation and tags among the supported variations.
		 *
		 * @param		information		Object containing an optional set of tags and a set of variation parameters to look for.
		 * @return						First variation that matches the variation parameters specified in @p information.
		 */
		u32 FindVariation(const FindVariationInformation& information) const;

		/**
		 * Finds the index of the default (primary) variation to use. This will be the first variation that matches the
		 * currently set variation parameters (if any).
		 */
		u32 GetDefaultVariation() const;

		/**
		 * Returns the number of passes that are used by the variation at the specified index.
		 *
		 * @param	variationIndex		Index of the variation to retrieve the number of passes for. 0 is always guaranteed to be the default variation.
		 * @return						Number of passes used by the variation.
		 */
		u32 GetPassCount(u32 variationIndex = 0) const;

		/**
		 * Retrieves a specific shader pass from the provided variation.
		 *
		 * @param	passIndex			Sequential index of the pass to retrieve.
		 * @param	variationIndex		Index of the variation to retrieve the pass for. 0 is always guaranteed to be the default variation.
		 * @return						Pass if found, null otherwise.
		 */
		SPtr<PassType> GetPass(u32 passIndex = 0, u32 variationIndex = 0) const;

		/**
		 * Creates an adapter that can be used for transferring parameters from the material to GpuParameterSet objects.
		 * The adapter will take care of tracking when material parameters change and can be used to update the underlying
		 * GpuParameterSet objects. Adapter is only valid for a particular material variation, you will need to create a
		 * different adapter for each variation.
		 */
		SPtr<MaterialParameterAdapterType> CreateParameterAdapter(u32 variationIndex = 0);

		/**
		 * Assigns a float value to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT()
		void SetFloat(const String& name, float value, u32 arrayIdx = 0) { return GetParamFloat(name).Set(value, arrayIdx); }

		/*
		 * Assigns a curve to the the float shader parameter with the specified name. The system will automatically
		 * evaluate the curve with the passage of time and apply the evaluated value to the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT()
		void SetFloatCurve(const String& name, TAnimationCurve<float> value, u32 arrayIdx = 0)
		{
			return GetParamFloatCurve(name).Set(std::move(value), arrayIdx);
		}

		/**
		 * Assigns a color to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT()
		void SetColor(const String& name, const Color& value, u32 arrayIdx = 0) { return GetParamColor(name).Set(value, arrayIdx); }

		/**
		 * Assigns a color gradient to the shader parameter with the specified name. The system will automatically
		 * evaluate the gradient with the passage of time and apply the evaluated value to the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT()
		void SetColorGradient(const String& name, const ColorGradientHDR& value, u32 arrayIdx = 0)
		{
			return GetParamColorGradient(name).Set(value, arrayIdx);
		}

		/**
		 * Assigns a 2D vector to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetVector2))
		void SetVec2(const String& name, const Vector2& value, u32 arrayIdx = 0) { return GetParamVec2(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 3D vector to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetVector3))
		void SetVec3(const String& name, const Vector3& value, u32 arrayIdx = 0) { return GetParamVec3(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 4D vector to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetVector4))
		void SetVec4(const String& name, const Vector4& value, u32 arrayIdx = 0) { return GetParamVec4(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 3x3 matrix to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMatrix3))
		void SetMat3(const String& name, const Matrix3& value, u32 arrayIdx = 0) { return GetParamMat3(name).Set(value, arrayIdx); }

		/**
		 * Assigns a 4x4 matrix to the shader parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMatrix4))
		void SetMat4(const String& name, const Matrix4& value, u32 arrayIdx = 0) { return GetParamMat4(name).Set(value, arrayIdx); }

		/**
		 * Assigns a structure to the shader parameter with the specified name.
		 *
		 * Structure is provided as a raw buffer and caller must ensure structure in buffer matches what the shader expects.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void SetStructData(const String& name, void* value, u32 size, u32 arrayIdx = 0) { return GetParamStruct(name).Set(value, size, arrayIdx); }

		/** Assigns a texture to the shader parameter with the specified name. */
		void SetTexture(const String& name, const TextureType& value, const TextureSurface& surface = TextureSurface::kComplete)
		{
			return GetParamTexture(name).Set(value, surface);
		}

		/**
		 * Assigns a sprite image to the shader parameter with the specified name. If the sprite image contains
		 * animation it will be automatically evaluated every frame.
		 *
		 * @note
		 * In order for the sprite sub-image to be properly applied the shader needs to have a 4D vector parameter marked
		 * with the SpriteUV attribute referencing this parameter. This vector will then receive the necessary UV offset
		 * and size which should be utilized by the shader code to render a subset of the texture as defined in the sprite
		 * texture.
		 */
		void SetSpriteImage(const String& name, const SpriteImageType& value)
		{
			return GetParamSpriteImage(name).Set(value);
		}

		/** Assigns a texture to be used for random load/store operations to the shader parameter with the specified name. */
		void SetLoadStoreTexture(const String& name, const TextureType& value, const TextureSurface& surface)
		{
			return GetParamLoadStoreTexture(name).Set(value, surface);
		}

		/** Assigns a buffer to the shader parameter with the specified name. */
		void SetBuffer(const String& name, const BufferType& value) { return GetParamBuffer(name).Set(value); }

		/** Assigns a sampler state to the shader parameter with the specified name. */
		void SetSamplerState(const String& name, const SPtr<SamplerState>& value) { return GetParamSamplerState(name).Set(value); }

		/**
		 * Returns a float value assigned with the parameter with the specified name. If a curve is assigned to this
		 * parameter, returns the curve value evaluated at time 0. Use getBoundParamType() to determine
		 * the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT()
		float GetFloat(const String& name, u32 arrayIdx = 0) const { return GetParamFloat(name).Get(arrayIdx); }

		/**
		 * Returns a curve value assigned to the parameter with the specified name. If the parameter has a constant
		 * value bound instead of a curve then this method returns an empty curve. Use getBoundParamType() to determine
		 * the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT()
		const TAnimationCurve<float>& GetFloatCurve(const String& name, u32 arrayIdx = 0) const
		{
			return GetParamFloatCurve(name).Get(arrayIdx);
		}

		/**
		 * Returns a color assigned with the parameter with the specified name. If a color gradient is assigned to this
		 * parameter, returns the gradient color evaluated at time 0. Use getBoundParamType() to determine
		 * the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT()
		Color GetColor(const String& name, u32 arrayIdx = 0) const { return GetParamColor(name).Get(arrayIdx); }

		/**
		 * Returns a color gradient assigned with the parameter with the specified name. If the parameter has a constant
		 * value bound instead of a gradient then this method returns an empty gradient. Use getBoundParamType() to
		 * determine the type of the parameter.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT()
		const ColorGradientHDR& GetColorGradient(const String& name, u32 arrayIdx = 0) const
		{
			return GetParamColorGradient(name).Get(arrayIdx);
		}

		/**
		 * Returns a 2D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetVector2))
		Vector2 GetVec2(const String& name, u32 arrayIdx = 0) const { return GetParamVec2(name).Get(arrayIdx); }

		/**
		 * Returns a 3D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetVector3))
		Vector3 GetVec3(const String& name, u32 arrayIdx = 0) const { return GetParamVec3(name).Get(arrayIdx); }

		/**
		 * Returns a 4D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetVector4))
		Vector4 GetVec4(const String& name, u32 arrayIdx = 0) const { return GetParamVec4(name).Get(arrayIdx); }

		/**
		 * Returns a 3x3 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetMatrix3))
		Matrix3 GetMat3(const String& name, u32 arrayIdx = 0) const { return GetParamMat3(name).Get(arrayIdx); }

		/**
		 * Returns a 4x4 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetMatrix4))
		Matrix4 GetMat4(const String& name, u32 arrayIdx = 0) const { return GetParamMat4(name).Get(arrayIdx); }

		/**
		 * Checks does the data parameter with the specified name currently contains animated data. This could be
		 * an animation curve or a color gradient.
		 */
		B3D_SCRIPT_EXPORT()
		bool IsAnimated(const String& name, u32 arrayIdx = 0);

		/** Returns a texture assigned with the parameter with the specified name. */
		TextureType GetTexture(const String& name) const { return GetParamTexture(name).Get(); }

		/**
		 * Returns a sprite image assigned to the parameter with the specified name. If the parameter has a regular
		 * texture attached instead of a sprite image, null will be returned. Use getBoundParamType() to determine
		 * the type of the parameter.
		 */
		SpriteImageType GetSpriteImage(const String& name) const { return GetParamSpriteImage(name).Get(); }

		/** Returns a sampler state assigned with the parameter with the specified name. */
		SPtr<SamplerState> GetSamplerState(const String& name) const { return GetParamSamplerState(name).Get(); }

		/**
		 * Returns a buffer representing a structure assigned to the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		MaterialBase::StructData GetStructData(const String& name, u32 arrayIdx = 0) const
		{
			TMaterialParameterStruct<IsRenderProxy> structParam = GetParamStruct(name);

			MaterialBase::StructData data(structParam.GetElementSize());
			structParam.Get(data.Data.get(), structParam.GetElementSize(), arrayIdx);

			return data;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a floating point parameter. This handle
		 * may be used for more efficiently getting/setting GPU parameter values than calling
		 * Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<float, IsRenderProxy> GetParamFloat(const String& name) const
		{
			TMaterialParameterPrimitive<float, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a time-varying curve to a floating point parameter. This
		 * handle may be used for more efficiently getting/setting GPU parameter values than calling
		 * Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterCurve<float, IsRenderProxy> GetParamFloatCurve(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a constant value to a color parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set*
		 * methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material,
		 * and then use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<Color, IsRenderProxy> GetParamColor(const String& name) const
		{
			TMaterialParameterPrimitive<Color, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a time-varying gradient to a color parameter. This handle
		 * may be used for more efficiently getting/setting GPU parameter values than calling
		 * Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material,
		 * and then use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterColorGradient<IsRenderProxy> GetParamColorGradient(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a constant value to a 2D vector parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set*
		 * methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<Vector2, IsRenderProxy> GetParamVec2(const String& name) const
		{
			TMaterialParameterPrimitive<Vector2, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 3D vector parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set*
		 * methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<Vector3, IsRenderProxy> GetParamVec3(const String& name) const
		{
			TMaterialParameterPrimitive<Vector3, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 4D vector parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set*
		 * methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<Vector4, IsRenderProxy> GetParamVec4(const String& name) const
		{
			TMaterialParameterPrimitive<Vector4, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 3x3 matrix parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set*
		 * methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<Matrix3, IsRenderProxy> GetParamMat3(const String& name) const
		{
			TMaterialParameterPrimitive<Matrix3, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a constant value to a 4x4 matrix parameter. This handle may be
		 * used for more efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set*
		 * methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterPrimitive<Matrix4, IsRenderProxy> GetParamMat4(const String& name) const
		{
			TMaterialParameterPrimitive<Matrix4, IsRenderProxy> gpuParam;
			GetParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a handle that allows you to assign a structure GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterStruct<IsRenderProxy> GetParamStruct(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a texture GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterSampledTexture<IsRenderProxy> GetParamTexture(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a sprite texture GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamSpriteImage<IsRenderProxy> GetParamSpriteImage(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a load-store texture GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterStorageTexture<IsRenderProxy> GetParamLoadStoreTexture(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a buffer GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterBuffer<IsRenderProxy> GetParamBuffer(const String& name) const;

		/**
		 * Returns a handle that allows you to assign a sampler state GPU parameter. This handle may be used for more
		 * efficiently getting/setting GPU parameter values than calling Material::Get* / Material::Set* methods.
		 *
		 * @note
		 * Expected behavior is that you would retrieve this handle when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParameterSampler<IsRenderProxy> GetParamSamplerState(const String& name) const;

		/**
		 * Allows you to retrieve a handle to a parameter that you can then use for quickly setting and retrieving parameter
		 * data. This allows you to set/get parameter data without all the cost of extra lookups otherwise required.
		 *
		 * @note
		 * All of these handles will be invalidated if material shader ever changes. It is up to the caller to keep track
		 * of that.
		 */
		template <typename T>
		void GetParam(const String& name, TMaterialParameterPrimitive<T, IsRenderProxy>& output) const;

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Returns an object containg all of material's parameters. Allows the caller to manipulate the parameters more
		 * directly.
		 */
		SPtr<MaterialParametersType> GetMaterialParameters() const { return mParameters; }

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
		 * Initializes the material by using the compatible variations from the currently set shader. Shader must contain
		 * the variations that matches the current renderer and GPU backend.
		 */
		void InitializeVariations();

		/** Assigns all the default parameters specified in the shader to the material. */
		void InitializeDefaultParameters();

		/** Reports an error if no shader is set, or no acceptable variation was found. */
		void ReportIfNotInitialized() const;

		ShaderType mShader;
		SPtr<MaterialParametersType> mParameters;
		Vector<SPtr<VariationType>> mVariations;
		ShaderVariationParameters mVariationParameters;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Material : public Resource, public TMaterial<false>, public IResourceListener
	{
	public:
		~Material() = default;

		/**
		 * Sets a shader that will be used by the material. Material will be initialized using all compatible variations
		 * from the shader. Shader must be set before doing any other operations with the material.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Shader), Property(Setter))
		void SetShader(const HShader& shader);

		/** @copydoc TMaterial<Core>::GetVariation() const */
		B3D_SCRIPT_EXPORT(ExportName(Variation), Property(Setter), UI(Hide))
		void SetVariation(const ShaderVariationParameters& variation);

		void Initialize() override;

		/** Creates a deep copy of the material and returns the new object. */
		B3D_SCRIPT_EXPORT(ExportName(Clone))
		HMaterial Clone();

		/**
		 * Creates a new empty material.
		 *
		 * @note	Make sure you call Material::setShader before using it.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Material))
		static HMaterial Create();

		/** Creates a new material with the specified shader. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Material))
		static HMaterial Create(const HShader& shader);

		/**
		 * Creates a new material with the specified shader, and a set of parameters that determine which subset of
		 * variations in the shader should the material use.
		 */
		static HMaterial Create(const HShader& shader, const ShaderVariationParameters& variation);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Marks the render proxy data as dirty. This causes the syncToCore() method to trigger the next time objects are synced
		 * between render and main threads.
		 */
		void MarkRenderProxyDataDirtyInternal(MaterialDirtyFlags flags = MaterialDirtyFlags::Parameter) override;

		void MarkDependenciesDirtyInternal() override;
		void MarkResourcesDirtyInternal() override;

		/** @} */
	private:
		struct SyncPacket;
		friend class render::Material;

		Material();
		Material(const HShader& shader, const ShaderVariationParameters& variation);

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		void GetListenerResources(Vector<HResource>& resources) override;

		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;

		/**	Performs material initialization when all resources are ready. */
		void InitializeIfLoaded();

		/**
		 * Uses the provided list of parameters to try to set every parameter in this material. Parameter whose name, type
		 * or size don't match are ignored and will not be set.
		 */
		void SetParams(const SPtr<MaterialParameters>& params);

		/**	Creates a new empty material but doesn't initialize it. */
		static SPtr<Material> CreateEmpty();

		u32 mLoadFlags;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class MaterialRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Material-Internal
		 *  @{
		 */

		/** @copydoc MaterialBase */
		class B3D_EXPORT Material : public RenderProxy, public TMaterial<true>
		{
		public:
			~Material() = default;

			void Initialize() override;

			/** @copydoc b3d::Material::SetShader */
			void SetShader(const SPtr<Shader>& shader);

			/**
			 * Set of parameters that determine which subset of variations in the assigned shader should be used. Only the
			 * variations that have the provided parameters with the provided values will match. This will control which
			 * variation is considered the default variation and which subset of variations are searched during a call to
			 * FindVariation().
			 */
			void SetVariation(const ShaderVariationParameters& variation);

			/** Creates a new material with the specified shader. */
			static SPtr<Material> Create(const SPtr<Shader>& shader);

		private:
			friend class b3d::Material;

			Material() = default;
			Material(const SPtr<Shader>& shader, const ShaderVariationParameters& variation);
			Material(const SPtr<Shader>& shader, const Vector<SPtr<Variation>>& variations, const SPtr<MaterialParameters>& materialParameters, const ShaderVariationParameters& variation);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
