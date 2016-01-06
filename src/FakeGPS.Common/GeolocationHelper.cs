namespace FakeGPS.Common
{
    using System;
    using System.Text.RegularExpressions;

    using System.Device.Location;
    using System.Threading;

    /// <summary>
    /// Static class to help with Geolocation operations.
    /// </summary>
    public static class GeolocationHelper
    {
        // http://stackoverflow.com/questions/3518504/regular-expression-for-matching-latitude-longitude-coordinates
        private static Regex regex = new Regex(@"^([-+]?\d{1,2}([.]\d+)?),\s*([-+]?\d{1,3}([.]\d+)?)$");

        /// <summary>
        /// Checks to see if the location string is valid.
        /// </summary>
        /// <param name="latLong">The location string.</param>
        /// <returns>
        /// A value indicating whether the location string was valid.
        /// </returns>
        public static bool IsValid(string latLong)
        {
            // protect IsMatch from null
            if (string.IsNullOrWhiteSpace(latLong))
            {
                return false;
            }

            // will return false if invalid
            return regex.IsMatch(latLong);
        }

        /// <summary>
        /// Convert a location string to a <see cref="LatLong"/>.
        /// </summary>
        /// <param name="latLong">The location string.</param>
        /// <returns>
        /// The <see cref="LatLong"/> instance.
        /// </returns>
        public static LatLong ToLatLong(string latLong)
        {
            if (!IsValid(latLong))
            {
                // TODO: Handle invalid string input.
                throw new NotImplementedException();
            }

            try
            {
                // ok we've got a well formated latLong string
                var splits = latLong.Split(',');

                return new LatLong()
                {
                    Latitude = Convert.ToDouble(splits[0]),
                    Longitude = Convert.ToDouble(splits[1])
                };
            }
            catch (Exception ex)
            {
                // TODO: Handle invalid string parsing.
                throw ex;
            }
        }

        /// <summary>
        /// Get the current location from the Windows location API.
        /// </summary>
        /// <returns>
        /// The <see cref="LatLong"/> instance.
        /// </returns>
        /// <remarks>
        /// This currently includes Thread.Sleep hack to ensure the device is ready.
        /// </remarks>
        public static LatLong Get()
        {
            GeoCoordinateWatcher watcher = new GeoCoordinateWatcher();

            watcher.TryStart(true, TimeSpan.FromMilliseconds(1000));

            // TODO: hack hack hack
            Thread.Sleep(1000);

            GeoCoordinate coord = watcher.Position.Location;

            return new LatLong()
            {
                Latitude = coord.Latitude,
                Longitude = coord.Longitude
            };
        }
    }
}