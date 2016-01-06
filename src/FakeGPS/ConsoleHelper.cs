namespace FakeGPS
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Reflection;

    using FakeGPS.Common;

    /// <summary>
    /// Console Helper Class
    /// </summary>
    public static class ConsoleHelper
    {
        /// <summary>
        /// The executing assembly.
        /// </summary>
        private static readonly Assembly Assembly = Assembly.GetExecutingAssembly();

        public static void WriteLatLong(LatLong latLong)
        {
            Console.WriteLine("Lat:\t{0}", latLong.Latitude);
            Console.WriteLine("Long:\t{0}", latLong.Longitude);
        }

        /// <summary>
        /// Read Line
        /// </summary>
        /// <returns>The Line</returns>
        public static string ReadLine()
        {
            return Console.ReadLine();
        }

        /// <summary>
        /// Write Line
        /// </summary>
        public static void WriteLine()
        {
            Console.WriteLine();
        }

        /// <summary>
        /// Write Line
        /// </summary>
        /// <param name="value">The Line</param>
        public static void WriteLine(string value)
        {
            Console.WriteLine(value);
        }

        /// <summary>
        /// Write Line
        /// </summary>
        /// <param name="format">Format</param>
        /// <param name="arg">Arguments</param>
        public static void WriteLine(string format, params object[] arg)
        {
            Console.WriteLine(format, arg);
        }

        /// <summary>
        /// Write the Header to the Console
        /// </summary>
        public static void WriteHeader()
        {
            Console.WriteLine("FakeGPS CLI version 1.0.0");
        }

        /// <summary>
        /// Write attach debugger message to the Console
        /// </summary>
        /// <remarks>
        /// DEBUG only. Message shown if debugger is not attached.
        /// </remarks>
        [Conditional("DEBUG")]
        public static void WriteDebugAttach()
        {
            // if the debugger is already attached, don't ask.
            if (Debugger.IsAttached)
            {
                return;
            }

            // this is helpful if you want to attach your debugger before running
            Console.WriteLine("Attach debugger and press return...");
            Console.ReadLine();
        }

        /// <summary>
        /// Write exit message to the Console
        /// </summary>
        /// <remarks>
        /// DEBUG only. Message shown if debugger is attached.
        /// </remarks>
        [Conditional("DEBUG")]
        public static void WriteDebugExit()
        {
            // if the debugger is not attached, just exit anyway.
            if (!Debugger.IsAttached)
            {
                return;
            }

            // this stops the application exiting right away on execution.
            Console.WriteLine("All your base to exit...");
            Console.ReadLine();
        }

        /// <summary>
        /// Write an Error to the Console in Red then change the color back
        /// </summary>
        /// <param name="message">The error message</param>
        public static void WriteError(string message)
        {
            var color = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("ZOMG, we got this error:");
            Console.WriteLine(message);
            Console.WriteLine("Send feedback plz!");
            Console.ForegroundColor = color;
        }

        /// <summary>
        /// Write the Help file to the Console
        /// </summary>
        public static void WriteHelp()
        {
            var help = ReadHelpResource();
            foreach (var line in help)
            {
                Console.WriteLine(line);
            }
        }

        /// <summary>
        /// Read the Help File
        /// </summary>
        /// <returns>Each line of the help file</returns>
        private static IEnumerable<string> ReadHelpResource()
        {
            var stream = Assembly.GetManifestResourceStream("FakeGPS.Resources.Help.txt");

            Debug.Assert(stream != null, "stream was null");

            if (stream == null)
            {
                yield break;
            }

            using (var r = new StreamReader(stream))
            {
                string line;
                while ((line = r.ReadLine()) != null)
                {
                    yield return line;
                }
            }
        }
    }
}
