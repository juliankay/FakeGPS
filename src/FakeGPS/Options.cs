namespace FakeGPS
{
    using CommandLine;

    /// <summary>
    /// Command Line Options class.
    /// </summary>
    internal class Options
    {
        /// <summary>
        /// Gets or sets the value for setting the location.
        /// </summary>
        /// <remarks>
        /// Included if -s or --set has been supplied.
        /// </remarks>
        [Option('s', "set", HelpText = "Set the location.")]
        public string LatLong { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether to get the location.
        /// </summary>
        [Option('g', "get", HelpText = "Get the location.")]
        public bool Get { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether to show the help text.
        /// </summary>
        [Option('h', "help", HelpText = "Help text please.")]
        public bool Help { get; set; }
    }
}