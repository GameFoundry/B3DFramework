//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
			Internal_create(this);
		}

		/// <summary>Determines whether the texture data is also stored in CPU memory.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool CpuCached
		{
			get { return Internal_getcpuCached(mCachedPtr); }
			set { Internal_setcpuCached(mCachedPtr, value); }
		}

		/// <summary>Determines should mesh normals be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportNormals
		{
			get { return Internal_getimportNormals(mCachedPtr); }
			set { Internal_setimportNormals(mCachedPtr, value); }
		}

		/// <summary>Determines should mesh tangents and bitangents be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportTangents
		{
			get { return Internal_getimportTangents(mCachedPtr); }
			set { Internal_setimportTangents(mCachedPtr, value); }
		}

		/// <summary>Determines should mesh blend shapes be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportBlendShapes
		{
			get { return Internal_getimportBlendShapes(mCachedPtr); }
			set { Internal_setimportBlendShapes(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines should mesh skin data like bone weights, indices and bind poses be imported if available.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportSkin
		{
			get { return Internal_getimportSkin(mCachedPtr); }
			set { Internal_setimportSkin(mCachedPtr, value); }
		}

		/// <summary>Determines should animation clips be imported if available.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportAnimation
		{
			get { return Internal_getimportAnimation(mCachedPtr); }
			set { Internal_setimportAnimation(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables keyframe reduction. Keyframe reduction will reduce the number of key-frames in an animation clip 
		/// by removing identical keyframes, and therefore reducing the size of the clip.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ReduceKeyFrames
		{
			get { return Internal_getreduceKeyFrames(mCachedPtr); }
			set { Internal_setreduceKeyFrames(mCachedPtr, value); }
		}

		/// <summary>
		/// Enables or disables import of root motion curves. When enabled, any animation curves in imported animations  
		/// affecting the root bone will be available through a set of separate curves in AnimationClip, and they won't be 
		/// evaluated through normal animation process. Instead it is expected that the user evaluates the curves manually and 
		/// applies them as required.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool ImportRootMotion
		{
			get { return Internal_getimportRootMotion(mCachedPtr); }
			set { Internal_setimportRootMotion(mCachedPtr, value); }
		}

		/// <summary>Uniformly scales the imported mesh by the specified value.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float ImportScale
		{
			get { return Internal_getimportScale(mCachedPtr); }
			set { Internal_setimportScale(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines what type (if any) of collision mesh should be imported. If enabled the collision mesh will be available 
		/// as a sub-resource returned by the importer (along with the normal mesh).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public CollisionMeshType CollisionMeshType
		{
			get { return Internal_getcollisionMeshType(mCachedPtr); }
			set { Internal_setcollisionMeshType(mCachedPtr, value); }
		}

		/// <summary>
		/// Animation split infos that determine how will the source animation clip be split. If no splits are present the  data 
		/// will be imported as one clip, but if splits are present the data will be split according to the split infos. Split 
		/// infos only affect the primary animation clip, other clips will not be split.
		/// </summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public AnimationSplitInfo[] AnimationSplits
		{
			get { return Internal_getanimationSplits(mCachedPtr); }
			set { Internal_setanimationSplits(mCachedPtr, value); }
		}

		/// <summary>Set of events that will be added to the animation clip, if animation import is enabled.</summary>
		[ShowInInspector]
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public ImportedAnimationEvents[] AnimationEvents
		{
			get { return Internal_getanimationEvents(mCachedPtr); }
			set { Internal_setanimationEvents(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getcpuCached(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcpuCached(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getimportNormals(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportNormals(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getimportTangents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportTangents(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getimportBlendShapes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportBlendShapes(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getimportSkin(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportSkin(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getimportAnimation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportAnimation(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getreduceKeyFrames(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setreduceKeyFrames(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getimportRootMotion(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportRootMotion(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getimportScale(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setimportScale(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CollisionMeshType Internal_getcollisionMeshType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcollisionMeshType(IntPtr thisPtr, CollisionMeshType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationSplitInfo[] Internal_getanimationSplits(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setanimationSplits(IntPtr thisPtr, AnimationSplitInfo[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ImportedAnimationEvents[] Internal_getanimationEvents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setanimationEvents(IntPtr thisPtr, ImportedAnimationEvents[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(MeshImportOptions managedInstance);
	}

	/** @} */
#endif
}
