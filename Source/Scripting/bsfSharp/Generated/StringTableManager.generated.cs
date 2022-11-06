//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	/// Manages string tables used for localizing text. Allows you to add and remove different tables and change the active 
	/// language.
	/// </summary>
	[ShowInInspector]
	public partial class StringTables : ScriptObject
	{
		private StringTables(bool __dummy0) { }
		protected StringTables() { }

		/// <summary>Determines the currently active language. Any newly created strings will use this value.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static Language ActiveLanguage
		{
			get { return Internal_GetActiveLanguage(); }
			set { Internal_SetActiveLanguage(value); }
		}

		/// <summary>Returns the string table with the specified id. If the table doesn&apos;t exist new one is created.</summary>
		/// <param name="id">Identifier of the string table.</param>
		/// <returns>String table with the specified identifier.</returns>
		public static RRef<StringTable> GetTable(int id)
		{
			return Internal_GetTable(id);
		}

		/// <summary>Removes the string table with the specified id.</summary>
		/// <param name="id">Identifier of the string table.</param>
		public static void RemoveTable(int id)
		{
			Internal_RemoveTable(id);
		}

		/// <summary>Registers a new string table or replaces an old one at the specified id.</summary>
		/// <param name="id">Identifier of the string table.</param>
		/// <param name="table">New string table to assign to the specified identifier.</param>
		public static void SetTable(int id, RRef<StringTable> table)
		{
			Internal_SetTable(id, table);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetActiveLanguage(Language language);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Language Internal_GetActiveLanguage();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<StringTable> Internal_GetTable(int id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveTable(int id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTable(int id, RRef<StringTable> table);
	}

	/** @} */
}
