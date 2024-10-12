//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains data about a single member (field or property) in a managed object (class or struct).</summary>
	[ShowInInspector]
	public partial class ManagedMemberInfo : ScriptObject
	{
		private ManagedMemberInfo(bool __dummy0) { }
		protected ManagedMemberInfo() { }

		/// <summary>Determines should the member be serialized when serializing the parent object.</summary>
		[NativeWrapper]
		public bool IsSerializable
		{
			get { return Internal_IsSerializable(mCachedPtr); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public string Name
		{
			get { return Internal_GetName(mCachedPtr); }
			set { Internal_SetName(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfo TypeInfo
		{
			get { return Internal_GetTypeInfo(mCachedPtr); }
			set { Internal_SetTypeInfo(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ManagedFieldMetaDataFlag MetaDataFlags
		{
			get { return Internal_GetMetaDataFlags(mCachedPtr); }
			set { Internal_SetMetaDataFlags(mCachedPtr, value); }
		}

		/// <summary>
		/// Parses style attributes for this members and returns a structure holding all the relevant style information.
		/// </summary>
		public ManagedMemberStyle ParseStyle()
		{
			ManagedMemberStyle temp;
			Internal_ParseStyle(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>Returns a boxed value contained in the member in the specified object instance.</summary>
		/// <param name="instance">Object instance to access the member on.</param>
		/// <returns>A boxed value of the member.</returns>
		public object GetValue(object instance)
		{
			return Internal_GetValue(mCachedPtr, instance);
		}

		/// <summary>Sets a value of the member in the specified object instance.</summary>
		/// <param name="instance">Object instance to access the member on.</param>
		/// <param name="value">Boxed value to set on the member.</param>
		public void SetValue(object instance, object value)
		{
			Internal_SetValue(mCachedPtr, instance, value);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsSerializable(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ParseStyle(IntPtr thisPtr, out ManagedMemberStyle __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_GetValue(IntPtr thisPtr, object instance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetValue(IntPtr thisPtr, object instance, object value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetName(IntPtr thisPtr, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetTypeInfo(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTypeInfo(IntPtr thisPtr, ManagedTypeInfo value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedFieldMetaDataFlag Internal_GetMetaDataFlags(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMetaDataFlags(IntPtr thisPtr, ManagedFieldMetaDataFlag value);
	}
}
