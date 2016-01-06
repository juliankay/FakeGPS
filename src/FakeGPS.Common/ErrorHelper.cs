namespace FakeGPS.Common
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;

    /// <summary>
    /// Error Helper Class
    /// </summary>
    public static class ErrorHelper
    {
        /// <summary>
        /// Invalid Arguments
        /// </summary>
        /// <param name="format">The message</param>
        /// <param name="args">The format arguments</param>
        /// <returns>The Exception</returns>
        public static ArgumentException InvalidArguments(string format = "Invalid Arguments", params object[] args)
        {
            var message = Format(format, args);

            Trace.TraceError(message);

            return new ArgumentException(message);
        }

        /// <summary>
        /// Format a string with the Current Culture.
        /// </summary>
        /// <param name="format">
        /// The format string.
        /// </param>
        /// <param name="args">
        /// The format arguments.
        /// </param>
        /// <returns>
        /// The formatted <see cref="string"/>.
        /// </returns>
        private static string Format(string format, params object[] args)
        {
            return string.Format(CultureInfo.CurrentCulture, format, args);
        }
    }
}
