//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Localization
	 *  @{
	 */

	/// <summary>
	/// String handle. Provides a wrapper around an Unicode string, primarily for localization purposes.
	///
	/// Actual value for this string is looked up in a global string table based on the provided identifier string and  
	/// currently active language. If such value doesn&apos;t exist then the identifier is used as is.
	///
	/// Use {0}, {1}, etc. in the string value for values that might change dynamically.
	/// </summary>
	[ShowInInspector]
	public partial class LocString : ScriptObject
	{
		private LocString(bool __dummy0) { }

		/// <summary>
		/// Creates a new localized string with the specified identifier. If the identifier doesn&apos;t previously exist in the  
		/// string table, identifier value will also be used for initializing the default language version of the string.
		/// </summary>
		/// <param name="identifier">String you can use for later referencing the localized string.</param>
		/// <param name="stringTableId">Unique identifier of the string table to retrieve the string from.</param>
		public LocString(string identifier, int stringTableId = 0)
		{
			Internal_HString(this, identifier, stringTableId);
		}

		/// <summary>
		/// Creates a new localized string with the specified identifier and sets the default language version of the  string. If 
		/// a string with that identifier already exists default language string will be updated.
		/// </summary>
		/// <param name="identifier">String you can use for later referencing the localized string.</param>
		/// <param name="defaultString">
		/// Default string to assign to the specified identifier. Language to which it will be  assigned depends on the 
		/// StringTable::DEFAULT_LANGUAGE value.
		/// </param>
		/// <param name="stringTableId">Unique identifier of the string table to retrieve the string from.</param>
		public LocString(string identifier, string defaultString, int stringTableId = 0)
		{
			Internal_HString0(this, identifier, defaultString, stringTableId);
		}

		/// <summary>Creates a new empty localized string.</summary>
		/// <param name="stringTableId">Unique identifier of the string table to retrieve the string from.</param>
		public LocString(int stringTableId)
		{
			Internal_HString1(this, stringTableId);
		}

		/// <summary>Creates a new empty localized string.</summary>
		public LocString()
		{
			Internal_HString2(this);
		}

		/// <summary>
		/// Sets a value of a string parameter. Parameters are specified as bracketed values within the string itself  (for 
		/// example {0}, {1}) etc. Use ^ as an escape character.
		/// </summary>
		public void SetParameter(int idx, string value)
		{
			Internal_setParameter(mCachedPtr, idx, value);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_HString(LocString managedInstance, string identifier, int stringTableId);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_HString0(LocString managedInstance, string identifier, string defaultString, int stringTableId);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_HString1(LocString managedInstance, int stringTableId);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_HString2(LocString managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getValue(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setParameter(IntPtr thisPtr, int idx, string value);
	}

	/** @} */
}
