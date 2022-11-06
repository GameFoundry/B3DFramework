//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>
	/// Contains import options you may use to control how is a mesh imported from some external format into engine format.
	/// </summary>
	[ShowInInspector]
	public partial class MeshImportOptions : ImportOptions
	{
		private MeshImportOptions(bool __dummy0) { }

		/// <summary>Creates a new import options object that allows you to customize how are meshes imported.</summary>
		public MeshImportOptions()
		{
			Internal_Create(this);
		}

		/// <summary>Determines whether the texture data is also stored in CPU memory.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool CpuCached
		{
			get { return Internal_GetCpuCached(mCachedPtr); }
			set { Internal_SetCpuCached(mCachedPtr, value); }
		}

		/// <summary>Determines should mesh normals be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportNormals
		{
			get { return Internal_GetImportNormals(mCachedPtr); }
			set { Internal_SetImportNormals(mCachedPtr, value); }
		}

		/// <summary>Determines should mesh tangents and bitangents be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportTangents
		{
			get { return Internal_GetImportTangents(mCachedPtr); }
			set { Internal_SetImportTangents(mCachedPtr, value); }
		}

		/// <summary>Determines should mesh blend shapes be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportBlendShapes
		{
			get { return Internal_GetImportBlendShapes(mCachedPtr); }
			set { Internal_SetImportBlendShapes(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should mesh skin data like bone weights, indices and bind poses be imported if available.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportSkin
		{
			get { return Internal_GetImportSkin(mCachedPtr); }
			set { Internal_SetImportSkin(mCachedPtr, value); }
		}

		/// <summary>Determines should animation clips be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportAnimation
		{
			get { return Internal_GetImportAnimation(mCachedPtr); }
			set { Internal_SetImportAnimation(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables keyframe reduction. Keyframe reduction will reduce the number of key-frames in an animation clip 
		/// by removing identical keyframes, and therefore reducing the size of the clip.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ReduceKeyFrames
		{
			get { return Internal_GetReduceKeyFrames(mCachedPtr); }
			set { Internal_SetReduceKeyFrames(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables import of root motion curves. When enabled, any animation curves in imported animations affecting 
		/// the root bone will be available through a set of separate curves in AnimationClip, and they won&apos;t be evaluated 
		/// through normal animation process. Instead it is expected that the user evaluates the curves manually and applies them 
		/// as required.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportRootMotion
		{
			get { return Internal_GetImportRootMotion(mCachedPtr); }
			set { Internal_SetImportRootMotion(mCachedPtr, value); }
		}

		/// <summary>Uniformly scales the imported mesh by the specified value.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float ImportScale
		{
			get { return Internal_GetImportScale(mCachedPtr); }
			set { Internal_SetImportScale(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines what type (if any) of collision mesh should be imported. If enabled the collision mesh will be available 
		/// as a sub-resource returned by the importer (along with the normal mesh).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public CollisionMeshType CollisionMeshType
		{
			get { return Internal_GetCollisionMeshType(mCachedPtr); }
			set { Internal_SetCollisionMeshType(mCachedPtr, value); }
		}

		/// <summary>
		/// Animation split infos that determine how will the source animation clip be split. If no splits are present the data 
		/// will be imported as one clip, but if splits are present the data will be split according to the split infos. Split 
		/// infos only affect the primary animation clip, other clips will not be split.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public AnimationSplitInfo[] AnimationSplits
		{
			get { return Internal_GetAnimationSplits(mCachedPtr); }
			set { Internal_SetAnimationSplits(mCachedPtr, value); }
		}

		/// <summary>Set of events that will be added to the animation clip, if animation import is enabled.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ImportedAnimationEvents[] AnimationEvents
		{
			get { return Internal_GetAnimationEvents(mCachedPtr); }
			set { Internal_SetAnimationEvents(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetCpuCached(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCpuCached(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetImportNormals(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportNormals(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetImportTangents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportTangents(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetImportBlendShapes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportBlendShapes(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetImportSkin(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportSkin(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetImportAnimation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportAnimation(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetReduceKeyFrames(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetReduceKeyFrames(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetImportRootMotion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportRootMotion(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetImportScale(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImportScale(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CollisionMeshType Internal_GetCollisionMeshType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCollisionMeshType(IntPtr thisPtr, CollisionMeshType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationSplitInfo[] Internal_GetAnimationSplits(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAnimationSplits(IntPtr thisPtr, AnimationSplitInfo[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ImportedAnimationEvents[] Internal_GetAnimationEvents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAnimationEvents(IntPtr thisPtr, ImportedAnimationEvents[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(MeshImportOptions managedInstance);
	}

	/** @} */
#endif
}
