namespace FakeGPS.Common
{
    using System;
    using System.Diagnostics;
    using Microsoft.Win32;

    /// <summary>
    /// Static class to help with Registry operations.
    /// </summary>
    public static class RegistryHelper
    {
        // TODO: Fix hard coded to the first device instance.
        private const string Path = @"SYSTEM\CurrentControlSet\Enum\ROOT\UNKNOWN\0000\Device Parameters\FakeGPS";
        private const string LatitudeProperty = @"SENSOR_PROPERTY_LATITUDE";
        private const string LongitudeProperty = @"SENSOR_PROPERTY_LONGITUDE";

        /// <summary>
        /// Set the Latitude and Longitude to the Registry.
        /// </summary>
        /// <param name="latLong">The <see cref="LatLong"/>.</param>
        public static void SetLatLong(LatLong latLong)
        {
            try
            {
                var key = Registry.LocalMachine.OpenSubKey(Path, true);

                key.SetValue(LatitudeProperty, latLong.Latitude);
                key.SetValue(LongitudeProperty, latLong.Longitude);

                key.Close();
            }
            catch (Exception ex)
            {
                if (Debugger.IsAttached)
                {
                    Debugger.Break();
                }

                // just re-throw for now
                // potentially catch specific exceptions for better handling
                throw ex;
            }
        }
    }
}