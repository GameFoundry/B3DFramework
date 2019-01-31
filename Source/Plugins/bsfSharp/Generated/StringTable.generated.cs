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
	/** @addtogroup Localization
	 *  @{
	 */

	/// <summary>Used for string localization. Stores strings and their translations in various languages.</summary>
	[ShowInInspector]
	public partial class StringTable : Resource
	{
		private StringTable(bool __dummy0) { }

		/// <summary>Creates a new empty string table resource.</summary>
		public StringTable()
		{
			Internal_create(this);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<StringTable> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a total number of strings in the table.</summary>
		[NativeWrapper]
		public uint NumStrings
		{
			get { return Internal_getNumStrings(mCachedPtr); }
		}

		/// <summary>Returns all identifiers that the string table contains localized strings for.</summary>
		[NativeWrapper]
		public string[] Identifiers
		{
			get { return Internal_getIdentifiers(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<StringTable>(StringTable x)
		{ return Internal_GetRef(x.mCachedPtr); }

		/// <summary>Checks does the string table contain the provided identifier.</summary>
		/// <param name="identifier">Identifier to look for.</param>
		/// <returns>True if the identifier exists in the table, false otherwise.</returns>
		public bool Contains(string identifier)
		{
			return Internal_contains(mCachedPtr, identifier);
		}

		/// <summary>Adds or modifies string translation for the specified language.</summary>
		public void SetString(string identifier, Language language, string value)
		{
			Internal_setString(mCachedPtr, identifier, language, value);
		}

		/// <summary>
		/// Returns a string translation for the specified language. Returns the identifier itself if one doesn't exist.
		/// </summary>
		public string GetString(string identifier, Language language)
		{
			return Internal_getString(mCachedPtr, identifier, language);
		}

		/// <summary>Removes the string described by identifier, from all languages.</summary>
		public void RemoveString(string identifier)
		{
			Internal_removeString(mCachedPtr, identifier);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<StringTable> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_contains(IntPtr thisPtr, string identifier);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getNumStrings(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string[] Internal_getIdentifiers(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setString(IntPtr thisPtr, string identifier, Language language, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getString(IntPtr thisPtr, string identifier, Language language);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeString(IntPtr thisPtr, string identifier);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(StringTable managedInstance);
	}

	/** @} */
}
