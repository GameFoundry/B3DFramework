//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Script-exportable variant of LogEntry.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct LogEntry
	{
		public string Message;
		public LogVerbosity Verbosity;
		public string CategoryName;
	}
}
