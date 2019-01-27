//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>
	/// Special physics controller meant to be used for game characters. Uses the "slide-and-collide" physics instead of of 
	/// the standard physics model to handle various issues with manually moving kinematic objects. Uses a capsule to 
	/// represent the character's bounds.
	/// </summary>
	[ShowInInspector]
	public partial class CharacterController : Component
	{
		private CharacterController(bool __dummy0) { }
		protected CharacterController() { }

		/// <summary>
		/// Determines the position of the bottom of the controller. Position takes contact offset into account. Changing this 
		/// will teleport the character to the location. Use move() for movement that includes physics.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 FootPosition
		{
			get
			{
				Vector3 temp;
				Internal_getFootPosition(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setFootPosition(mCachedPtr, ref value); }
		}

		/// <summary>Determines the radius of the controller capsule.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_getRadius(mCachedPtr); }
			set { Internal_setRadius(mCachedPtr, value); }
		}

		/// <summary>Determines the height between the centers of the two spheres of the controller capsule.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Height
		{
			get { return Internal_getHeight(mCachedPtr); }
			set { Internal_setHeight(mCachedPtr, value); }
		}

		/// <summary>Determines the up direction of capsule. Determines capsule orientation.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Up
		{
			get
			{
				Vector3 temp;
				Internal_getUp(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setUp(mCachedPtr, ref value); }
		}

		/// <summary>Controls what happens when character encounters a height higher than its step offset.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CharacterClimbingMode ClimbingMode
		{
			get { return Internal_getClimbingMode(mCachedPtr); }
			set { Internal_setClimbingMode(mCachedPtr, value); }
		}

		/// <summary>Controls what happens when character encounters a slope higher than its slope offset.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CharacterNonWalkableMode NonWalkableMode
		{
			get { return Internal_getNonWalkableMode(mCachedPtr); }
			set { Internal_setNonWalkableMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Represents minimum distance that the character will move during a call to move(). This is used to stop the recursive 
		/// motion algorithm when the remaining distance is too small.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MinMoveDistance
		{
			get { return Internal_getMinMoveDistance(mCachedPtr); }
			set { Internal_setMinMoveDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Contact offset specifies a skin around the object within which contacts will be generated. It should be a small 
		/// positive non-zero value.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float ContactOffset
		{
			get { return Internal_getContactOffset(mCachedPtr); }
			set { Internal_setContactOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls which obstacles will the character be able to automatically step over without being stopped. This is the 
		/// height of the maximum obstacle that will be stepped over (with exceptions, see climbingMode).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float StepOffset
		{
			get { return Internal_getStepOffset(mCachedPtr); }
			set { Internal_setStepOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls which slopes should the character consider too steep and won't be able to move over. See nonWalkableMode for 
		/// more information.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Radian SlopeLimit
		{
			get
			{
				Radian temp;
				Internal_getSlopeLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setSlopeLimit(mCachedPtr, ref value); }
		}

		/// <summary>Determines the layer that controls what can the controller collide with.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_getLayer(mCachedPtr); }
			set { Internal_setLayer(mCachedPtr, value); }
		}

		/// <summary>Triggered when the controller hits a collider.</summary>
		public event Action<ControllerColliderCollision> OnColliderHit;

		/// <summary>Triggered when the controller hits another character controller.</summary>
		public event Action<ControllerControllerCollision> OnControllerHit;

		/// <summary>
		/// Moves the controller in the specified direction by the specified amount, while interacting with surrounding geometry. 
		/// Returns flags signaling where collision occurred after the movement.
		///
		/// Does not account for gravity, you must apply it manually.
		/// </summary>
		public CharacterCollisionFlag Move(Vector3 displacement)
		{
			return Internal_move(mCachedPtr, ref displacement);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharacterCollisionFlag Internal_move(IntPtr thisPtr, ref Vector3 displacement);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getFootPosition(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setFootPosition(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setHeight(IntPtr thisPtr, float height);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getUp(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setUp(IntPtr thisPtr, ref Vector3 up);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharacterClimbingMode Internal_getClimbingMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setClimbingMode(IntPtr thisPtr, CharacterClimbingMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharacterNonWalkableMode Internal_getNonWalkableMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setNonWalkableMode(IntPtr thisPtr, CharacterNonWalkableMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMinMoveDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMinMoveDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getContactOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setContactOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getStepOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setStepOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getSlopeLimit(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSlopeLimit(IntPtr thisPtr, ref Radian value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_getLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayer(IntPtr thisPtr, ulong layer);
		private void Internal_onColliderHit(ref ControllerColliderCollision p0)
		{
			OnColliderHit?.Invoke(p0);
		}
		private void Internal_onControllerHit(ref ControllerControllerCollision p0)
		{
			OnControllerHit?.Invoke(p0);
		}
	}

	/** @} */
}
