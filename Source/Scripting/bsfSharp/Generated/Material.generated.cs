//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Material that controls how objects are rendered. It is represented by a shader and parameters used to set up that 
	/// shader. It provides a simple interface for manipulating the parameters.
	/// </summary>
	[ShowInInspector]
	public partial class Material : Resource
	{
		private Material(bool __dummy0) { }

		/// <summary>Creates a new empty material.</summary>
		public Material()
		{
			Internal_create(this);
		}

		/// <summary>Creates a new material with the specified shader.</summary>
		public Material(RRef<Shader> shader)
		{
			Internal_create0(this, shader);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<Material> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>
		/// Sets a shader that will be used by the material. Material will be initialized using all compatible techniques from 
		/// the shader. Shader must be set before doing any other operations with the material.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Shader> Shader
		{
			get { return Internal_getShader(mCachedPtr); }
			set { Internal_setShader(mCachedPtr, value); }
		}

		/// <summary>
		/// Set of parameters that determine which subset of techniques in the assigned shader should be used. Only the 
		/// techniques that have the provided parameters with the provided values will match. This will control which technique 
		/// is considered the default technique and which subset of techniques are searched during a call to findTechnique().
		/// </summary>
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ShaderVariation Variation
		{
			get { return Internal_getVariation(mCachedPtr); }
			set { Internal_setVariation(mCachedPtr, value); }
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
			return Internal_clone(mCachedPtr);
		}

		/// <summary>
		/// Assigns a float value to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetFloat(string name, float value, int arrayIdx = 0)
		{
			Internal_setFloat(mCachedPtr, name, value, arrayIdx);
		}

		public void SetFloatCurve(string name, AnimationCurve value, int arrayIdx = 0)
		{
			Internal_setFloatCurve(mCachedPtr, name, value, arrayIdx);
		}

		/// <summary>
		/// Assigns a color to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetColor(string name, Color value, int arrayIdx = 0)
		{
			Internal_setColor(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a color gradient to the shader parameter with the specified name. The system will automatically evaluate the 
		/// gradient with the passage of time and apply the evaluated value to the parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetColorGradient(string name, ColorGradientHDR value, int arrayIdx = 0)
		{
			Internal_setColorGradient(mCachedPtr, name, value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 2D vector to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetVector2(string name, Vector2 value, int arrayIdx = 0)
		{
			Internal_setVec2(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 3D vector to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetVector3(string name, Vector3 value, int arrayIdx = 0)
		{
			Internal_setVec3(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 4D vector to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetVector4(string name, Vector4 value, int arrayIdx = 0)
		{
			Internal_setVec4(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 3x3 matrix to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetMatrix3(string name, Matrix3 value, int arrayIdx = 0)
		{
			Internal_setMat3(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Assigns a 4x4 matrix to the shader parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index to assign the value to.
		/// </summary>
		public void SetMatrix4(string name, Matrix4 value, int arrayIdx = 0)
		{
			Internal_setMat4(mCachedPtr, name, ref value, arrayIdx);
		}

		/// <summary>
		/// Returns a float value assigned with the parameter with the specified name. If a curve is assigned to this parameter, 
		/// returns the curve value evaluated at time 0. Use getBoundParamType() to determine the type of the parameter.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public float GetFloat(string name, int arrayIdx = 0)
		{
			return Internal_getFloat(mCachedPtr, name, arrayIdx);
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
			return Internal_getFloatCurve(mCachedPtr, name, arrayIdx);
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
			Internal_getColor(mCachedPtr, name, arrayIdx, out temp);
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
			return Internal_getColorGradient(mCachedPtr, name, arrayIdx);
		}

		/// <summary>
		/// Returns a 2D vector assigned with the parameter with the specified name.
		///
		/// Optionally if the parameter is an array you may provide an array index you which to retrieve.
		/// </summary>
		public Vector2 GetVector2(string name, int arrayIdx = 0)
		{
			Vector2 temp;
			Internal_getVec2(mCachedPtr, name, arrayIdx, out temp);
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
			Internal_getVec3(mCachedPtr, name, arrayIdx, out temp);
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
			Internal_getVec4(mCachedPtr, name, arrayIdx, out temp);
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
			Internal_getMat3(mCachedPtr, name, arrayIdx, out temp);
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
			Internal_getMat4(mCachedPtr, name, arrayIdx, out temp);
			return temp;
		}

		/// <summary>
		/// Checks does the data parameter with the specified name currently contains animated data. This could be an animation 
		/// curve or a color gradient.
		/// </summary>
		public bool IsAnimated(string name, int arrayIdx = 0)
		{
			return Internal_isAnimated(mCachedPtr, name, arrayIdx);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setShader(IntPtr thisPtr, RRef<Shader> shader);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVariation(IntPtr thisPtr, ShaderVariation variation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Material> Internal_clone(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Shader> Internal_getShader(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ShaderVariation Internal_getVariation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFloat(IntPtr thisPtr, string name, float value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFloatCurve(IntPtr thisPtr, string name, AnimationCurve value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setColor(IntPtr thisPtr, string name, ref Color value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setColorGradient(IntPtr thisPtr, string name, ColorGradientHDR value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVec2(IntPtr thisPtr, string name, ref Vector2 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVec3(IntPtr thisPtr, string name, ref Vector3 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setVec4(IntPtr thisPtr, string name, ref Vector4 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMat3(IntPtr thisPtr, string name, ref Matrix3 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMat4(IntPtr thisPtr, string name, ref Matrix4 value, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getFloat(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve Internal_getFloatCurve(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getColor(IntPtr thisPtr, string name, int arrayIdx, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradientHDR Internal_getColorGradient(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getVec2(IntPtr thisPtr, string name, int arrayIdx, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getVec3(IntPtr thisPtr, string name, int arrayIdx, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getVec4(IntPtr thisPtr, string name, int arrayIdx, out Vector4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMat3(IntPtr thisPtr, string name, int arrayIdx, out Matrix3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMat4(IntPtr thisPtr, string name, int arrayIdx, out Matrix4 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isAnimated(IntPtr thisPtr, string name, int arrayIdx);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(Material managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(Material managedInstance, RRef<Shader> shader);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTexture(IntPtr thisPtr, string name, RRef<Texture> value, int mipLevel, int numMipLevels, int arraySlice, int numArraySlices);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_getTexture(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSpriteTexture(IntPtr thisPtr, string name, RRef<SpriteTexture> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteTexture> Internal_getSpriteTexture(IntPtr thisPtr, string name);
	}

	/** @} */
}
