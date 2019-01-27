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
			get { return Internal_getActiveLanguage(); }
			set { Internal_setActiveLanguage(value); }
		}

		/// <summary>Returns the string table with the specified id. If the table doesn't exist new one is created.</summary>
		/// <param name="id">Identifier of the string table.</param>
		/// <returns>String table with the specified identifier.</returns>
		public static RRef<StringTable> GetTable(uint id)
		{
			return Internal_getTable(id);
		}

		/// <summary>Removes the string table with the specified id.</summary>
		/// <param name="id">Identifier of the string table.</param>
		public static void RemoveTable(uint id)
		{
			Internal_removeTable(id);
		}

		/// <summary>Registers a new string table or replaces an old one at the specified id.</summary>
		/// <param name="id">Identifier of the string table.</param>
		/// <param name="table">New string table to assign to the specified identifier.</param>
		public static void SetTable(uint id, RRef<StringTable> table)
		{
			Internal_setTable(id, table);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setActiveLanguage(Language language);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Language Internal_getActiveLanguage();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<StringTable> Internal_getTable(uint id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeTable(uint id);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTable(uint id, RRef<StringTable> table);
	}

	/** @} */
}
