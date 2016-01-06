namespace FakeGPS.Tests
{
    using System;

    using Xunit;

    using FakeGPS.Common;

    public static class GeolocationHelperTests
    {
        [Theory]
        [InlineData("53.858772,-1.431020", true)]
        [InlineData("90.00,180.00", true)]
        [InlineData("-90,-180", true)]
        [InlineData("200,200", false)]
        [InlineData("-200, -200", false)]
        [InlineData("-90 ,-180", false)]
        [InlineData("-90,,-180", false)]
        [InlineData("", false)]
        [InlineData(",", false)]
        [InlineData(null, false)]
        public static void IsValidTest(string latLong, bool expected)
        {
            Assert.Equal(expected, GeolocationHelper.IsValid(latLong));
        }

        [Theory]
        [InlineData("53.858772,-1.431020", 53.858772, -1.431020)]
        [InlineData("90.00,180.00", 90.00, 180.00)]
        public static void ToLatLongTest(string latLong, double expectedLat, double expectedLong)
        {
            var obj = GeolocationHelper.ToLatLong(latLong);
            Assert.Equal(expectedLat, obj.Latitude);
            Assert.Equal(expectedLong, obj.Longitude);
        }
    }
}
