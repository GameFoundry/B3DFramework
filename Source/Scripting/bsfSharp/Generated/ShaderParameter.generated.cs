//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Contains information about a single shader parameter.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ShaderParameter
	{
		/// <summary>Name of the parameter variable.</summary>
		public string Name;
		/// <summary>Variable identifier of the parameter.</summary>
		public string Identifier;
		/// <summary>Data type of the parameter.</summary>
		public ShaderParameterType Type;
		/// <summary>Flags used to further describe the parameter.</summary>
		public ShaderParameterFlag Flags;
	}

	/** @} */
}
