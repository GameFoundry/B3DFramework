//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Possible types of debug messages.
    /// </summary>
    public enum DebugMessageType // Note: Must match C++ enum DebugChannel
    {
        Info, Warning, Error, CompilerWarning, CompilerError
    }

    /// <summary>
    /// Contains data for a single entry in a call stack associated with a log entry.
    /// </summary>
    public class CallStackEntry
    {
        public string method;
        public string file;
        public int line;
    }

    /// <summary>
    /// Contains data for a single log entry. Contained data was parsed from the message string
    /// to better organize the provided information.
    /// </summary>
    public class ParsedLogEntry
    {
        public string message;
        public CallStackEntry[] callstack;
    }

    /// <summary>
    /// Contains data for a single log entry.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct LogEntry // Note: Must match C++ struct ScriptLogEntryData
    {
        public DebugMessageType type;
        public string message;
    }

    /// <summary>
    /// Utility class providing various debug functionality.
    /// </summary>
    public sealed class Debug
    {
        /// <summary>
        /// Triggered when a new message is added to the debug log.
        /// </summary>
        public static Action<DebugMessageType, string> OnAdded;

        /// <summary>
        /// Returns a list of all messages in the debug log.
        /// </summary>
        public static LogEntry[] Messages
        {
            get { return Internal_GetMessages(); }
        }

        /// <summary>
        /// Logs a new informative message to the global debug log.
        /// </summary>
        /// <param name="message">Message to log.</param>
        public static void Log(object message)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine(message.ToString());
            sb.Append(GetStackTrace(1));

            Internal_Log(sb.ToString());
        }

        /// <summary>
        /// Logs a new warning message to the global debug log.
        /// </summary>
        /// <param name="message">Message to log.</param>
        public static void LogWarning(object message)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine(message.ToString());
            sb.Append(GetStackTrace(1));

            Internal_LogWarning(sb.ToString());
        }

        /// <summary>
        /// Logs a new error message to the global debug log.
        /// </summary>
        /// <param name="message">Message to log.</param>
        public static void LogError(object message)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine(message.ToString());
            sb.Append(GetStackTrace(1));

            Internal_LogError(sb.ToString());
        }

        /// <summary>
        /// Logs a new message to the global debug log using the provided type.
        /// </summary>
        /// <param name="message">Message to log.</param>
        /// <param name="type">Type of the message to log.</param>
        internal static void LogMessage(object message, DebugMessageType type)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine(message.ToString());

            Internal_LogMessage(sb.ToString(), type);
        }

        /// <summary>
        /// Clears all messages from the debug log.
        /// </summary>
        internal static void Clear()
        {
            Internal_Clear();
        }

        /// <summary>
        /// Clears all messages of the specified type from the debug log.
        /// </summary>
        internal static void Clear(DebugMessageType type)
        {
            Internal_ClearType(type);
        }

        /// <summary>
        /// Returns the stack trace of the current point in code.
        /// </summary>
        /// <param name="ignoreFirstN">Determines how many of the top-level entries should be ignored.</param>
        /// <returns>String containing the stack trace.</returns>
        public static string GetStackTrace(int ignoreFirstN = 0)
        {
            StackTrace stackTrace = new StackTrace(1, true);

            StackFrame[] frames = stackTrace.GetFrames();
            if (frames == null)
                return "";

            StringBuilder sb = new StringBuilder();

            for(int i = 0; i < frames.Length; i++)
            {
                if (i < ignoreFirstN)
                    continue;

                StackFrame frame = frames[i];

                MethodBase method = frame.GetMethod();
                if (method == null)
                    continue;

                Type parentType = method.DeclaringType;
                if (parentType == null)
                    continue;

                sb.Append("\tat " + parentType.Name + "." + method.Name + "(");

                ParameterInfo[] methodParams = method.GetParameters();
                for(int j = 0; j < methodParams.Length; j++)
                {
                    if (j > 0)
                        sb.Append(", ");

                    sb.Append(methodParams[j].ParameterType.Name);
                }

                sb.Append(")");

                string ns = parentType.Namespace;
                string fileName = frame.GetFileName();
                if (!string.IsNullOrEmpty(fileName))
                {
                    int line = frame.GetFileLineNumber();
                    int column = frame.GetFileColumnNumber();

                    sb.Append(" in " + fileName + ", line " + line + ", column " + column + ", namespace " + ns);
                }
                else
                {
                    if (!string.IsNullOrEmpty(ns))
                        sb.Append(" in namespace " + ns);
                }

                sb.AppendLine();
            }

            return sb.ToString();
        }

        /// <summary>
        /// Parses a log message and outputs a data object with a separate message and callstack entries.
        /// </summary>
        /// <param name="message">Message to parse.</param>
        /// <returns>Parsed log message.</returns>
        public static ParsedLogEntry ParseLogMessage(string message)
        {
            // Note: If you are modifying GetStackTrace method make sure to also update this one to match the formattting
            int firstMatchIdx = -1;
            Regex regex = new Regex(@"\tat (.*) in (.*), line (\d*), column .*, namespace .*");
            var matches = regex.Matches(message);

            ParsedLogEntry newEntry = new ParsedLogEntry();
            newEntry.callstack = new CallStackEntry[matches.Count];
            for (int i = 0; i < matches.Count; i++)
            {
                CallStackEntry callstackEntry = new CallStackEntry();
                callstackEntry.method = matches[i].Groups[1].Value;
                callstackEntry.file = matches[i].Groups[2].Value;
                int.TryParse(matches[i].Groups[3].Value, out callstackEntry.line);

                newEntry.callstack[i] = callstackEntry;

                if (firstMatchIdx == -1)
                    firstMatchIdx = matches[i].Index;
            }

            if (firstMatchIdx != -1)
                newEntry.message = message.Substring(0, firstMatchIdx);
            else
                newEntry.message = message;

            return newEntry;
        }

        /// <summary>
        /// Parses a managed exception message and outputs a data object with a separate message and callstack entries.
        /// </summary>
        /// <param name="message">Message to parse.</param>
        /// <returns>Parsed log message.</returns>
        public static ParsedLogEntry ParseExceptionMessage(string message)
        {
            Regex headerRegex = new Regex(@"Managed exception: (.*)\n");
            var headerMatch = headerRegex.Match(message);

            if (!headerMatch.Success)
                return null;

            Regex regex = new Regex(@"  at (.*) \[.*\] in (.*):(\d*)");
            var matches = regex.Matches(message);

            ParsedLogEntry newEntry = new ParsedLogEntry();
            newEntry.callstack = new CallStackEntry[matches.Count];
            for (int i = 0; i < matches.Count; i++)
            {
                CallStackEntry callstackEntry = new CallStackEntry();
                callstackEntry.method = matches[i].Groups[1].Value;
                callstackEntry.file = matches[i].Groups[2].Value;
                int.TryParse(matches[i].Groups[3].Value, out callstackEntry.line);

                newEntry.callstack[i] = callstackEntry;
            }

            newEntry.message = headerMatch.Groups[1].Value;

            return newEntry;
        }

        /// <summary>
        /// Triggered by the runtime when a new message is added to the debug log.
        /// </summary>
        /// <param name="type">Type of the message that was added.</param>
        /// <param name="message">Text of the message.</param>
        private static void Internal_OnAdded(DebugMessageType type, string message)
        {
            if (OnAdded != null)
                OnAdded(type, message);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_Log(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_LogWarning(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_LogError(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_LogMessage(string message, DebugMessageType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_Clear();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_ClearType(DebugMessageType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern LogEntry[] Internal_GetMessages();
    }

    /** @} */
}
