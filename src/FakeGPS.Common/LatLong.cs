namespace FakeGPS.Common
{
    /// <summary>
    /// Container class for the two double values of Latitude and Longitude
    /// </summary>
    public class LatLong
    {
        // replace with https://msdn.microsoft.com/en-us/library/system.device.location.geocoordinate.aspx
        // if we end up using more than just lat/long

        /// <summary>
        /// Gets or sets the Latitude.
        /// </summary>
        public double Latitude { get; set; }

        /// <summary>
        /// Gets or sets the Longitude.
        /// </summary>
        public double Longitude { get; set; }
    }
}
