//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Rendering
	 *  @{
	 */

	[ShowInInspector]
	public partial class Material : Resource
	{
		private Material(bool __dummy0) { }

		/// <summary>Creates a new empty material.</summary>
		public Material()
		{
			Internal_Create(this);
		}

		/// <summary>Creates a new material with the specified shader.</summary>
		public Material(RRef<Shader> shader)
		{
			Internal_Create0(this, shader);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Material> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>
		/// Sets a shader that will be used by the material. Material will be initialized using all compatible variations from 
		/// the shader. Shader must be set before doing any other operations with the material.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Shader> Shader
		{
			get { return Internal_GetShader(mCachedPtr); }
			set { Internal_SetShader(mCachedPtr, value); }
		}

		/// <summary>
		/// Set of parameters that determine which subset of variations in the assigned shader should be used. Only the 
		/// variations that have the provided parameters with the provided values will match. This will control which variation 
		/// is considered the default variation and which subset of variations are searched during a call to FindVariation().
		/// </summary>
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ShaderVariationParameters Variation
		{
			get { return Internal_GetVariationParameters(mCachedPtr); }
			set { Internal_SetVariation(mCachedPtr, value); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<Material>(Material x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		/// <summary>Creates a deep copy of the material and returns the new object.</summary>
		public RRef<Material> Clone()
		{
			return Internal_Clone(mCachedPtr);
		}

		/// <summary>
		/// Assigns a float value to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetFloat(string name, float value, int arrayIdx = 0)
		{
			Internal_SetFloat(mCachedPtr, name, value, arrayIdx);
		}

		public void SetFloatCurve(string name, AnimationCurve value, int arrayIdx = 0)
		{
			Internal_SetFloatCurve(mCachedPtr, name, value, arrayIdx);
		}

		/// <summary>
		/// Assigns a color to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetColor(string name, Color value, int arrayIdx = 0)
		{
			Internal_SetColor(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a color gradient to the shader parameter with the specified name. The system will automatically evaluate the 
		/// gradient with the passage of time and apply the evaluated value to the parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetColorGradient(string name, ColorGradientHDR value, int arrayIdx = 0)
		{
			Internal_SetColorGradient(mCachedPtr, name, value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 2D vector to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetVector2(string name, TVector2<float> value, int arrayIdx = 0)
		{
			Internal_SetVec2(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 3D vector to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetVector3(string name, Vector3 value, int arrayIdx = 0)
		{
			Internal_SetVec3(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 4D vector to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetVector4(string name, Vector4 value, int arrayIdx = 0)
		{
			Internal_SetVec4(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 3x3 matrix to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetMatrix3(string name, Matrix3 value, int arrayIdx = 0)
		{
			Internal_SetMat3(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 4x4 matrix to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetMatrix4(string name, Matrix4 value, int arrayIdx = 0)
		{
			Internal_SetMat4(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Returns a float value assigned with the parameter with the specified name. If a curve is assigned to this parameter, 
		/// returns the curve value evaluated at time 0. Use getBoundParamType() to determine the type of the parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public float GetFloat(string name, int arrayIdx = 0)
		{
			return Internal_GetFloat(mCachedPtr, name, arrayIdx);
		}

		/// <summary>
		/// Returns a curve value assigned to the parameter with the specified name. If the parameter has a constant value bound 
		/// instead of a curve then this method returns an empty curve. Use getBoundParamType() to determine the type of the 
		/// parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public AnimationCurve GetFloatCurve(string name, int arrayIdx = 0)
		{
			return Internal_GetFloatCurve(mCachedPtr, name, arrayIdx);
		}

		/// <summary>
		/// Returns a color assigned with the parameter with the specified name. If a color gradient is assigned to this 
		/// parameter, returns the gradient color evaluated at time 0. Use getBoundParamType() to determine the type of the 
		/// parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public Color GetColor(string name, int arrayIdx = 0)
		{
			Color temp;
			Internal_GetColor(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a color gradient assigned with the parameter with the specified name. If the parameter has a constant value 
		/// bound instead of a gradient then this method returns an empty gradient. Use getBoundParamType() to determine the type 
		/// of the parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public ColorGradientHDR GetColorGradient(string name, int arrayIdx = 0)
		{
			return Internal_GetColorGradient(mCachedPtr, name, arrayIdx);
		}

		/// <summary>
		/// Returns a 2D vector assigned with the parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public TVector2<float> GetVector2(string name, int arrayIdx = 0)
		{
			TVector2<float> temp;
			Internal_GetVec2(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a 3D vector assigned with the parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public Vector3 GetVector3(string name, int arrayIdx = 0)
		{
			Vector3 temp;
			Internal_GetVec3(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a 4D vector assigned with the parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public Vector4 GetVector4(string name, int arrayIdx = 0)
		{
			Vector4 temp;
			Internal_GetVec4(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a 3x3 matrix assigned with the parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public Matrix3 GetMatrix3(string name, int arrayIdx = 0)
		{
			Matrix3 temp;
			Internal_GetMat3(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a 4x4 matrix assigned with the parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public Matrix4 GetMatrix4(string name, int arrayIdx = 0)
		{
			Matrix4 temp;
			Internal_GetMat4(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Checks does the data parameter with the specified name currently contains animated data. This could be an animation 
		/// curve or a color gradient.
		/// </summary>
		public bool IsAnimated(string name, int arrayIdx = 0)
		{
			return Internal_IsAnimated(mCachedPtr, name, arrayIdx);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetShader(IntPtr thisPtr, RRef<Shader> shader);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVariation(IntPtr thisPtr, ShaderVariationParameters variation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_Clone(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Shader> Internal_GetShader(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ShaderVariationParameters Internal_GetVariationParameters(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFloat(IntPtr thisPtr, string name, float value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFloatCurve(IntPtr thisPtr, string name, AnimationCurve value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetColor(IntPtr thisPtr, string name, ref Color value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetColorGradient(IntPtr thisPtr, string name, ColorGradientHDR value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVec2(IntPtr thisPtr, string name, ref TVector2<float> value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVec3(IntPtr thisPtr, string name, ref Vector3 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetVec4(IntPtr thisPtr, string name, ref Vector4 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMat3(IntPtr thisPtr, string name, ref Matrix3 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMat4(IntPtr thisPtr, string name, ref Matrix4 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFloat(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve Internal_GetFloatCurve(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetColor(IntPtr thisPtr, string name, int arrayIdx, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradientHDR Internal_GetColorGradient(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetVec2(IntPtr thisPtr, string name, int arrayIdx, out TVector2<float> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetVec3(IntPtr thisPtr, string name, int arrayIdx, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetVec4(IntPtr thisPtr, string name, int arrayIdx, out Vector4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetMat3(IntPtr thisPtr, string name, int arrayIdx, out Matrix3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetMat4(IntPtr thisPtr, string name, int arrayIdx, out Matrix4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsAnimated(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(Material managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(Material managedInstance, RRef<Shader> shader);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTexture(IntPtr thisPtr, string name, RRef<Texture> value, int mipLevel, int numMipLevels, int arraySlice, int numArraySlices);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetTexture(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSpriteImage(IntPtr thisPtr, string name, RRef<SpriteImage> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteImage> Internal_GetSpriteImage(IntPtr thisPtr, string name);
	}

	/** @} */
}
