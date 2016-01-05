# FakeGPS

FakeGPS is a Windows driver which allows the user to provide geolocation information without a physical GPS device.

## Why?
Windows 10 includes Cortana, a digital assistant which can help you with location based reminders.
When there is no GPS device the location services in Windows will try to guess where you are geographically using other means like Wi-Fi or IP address.
These methods don't always work, rendering the location-based features of Cortana as useless.

This is especially noticeable on virtual machines which provide geolocation of the cloud infrastructure, not where the user is.

For more information on why you might want this see [Using Cortana with a Fake GPS Driver on Windows 10](http://juliankay.com/development/using-cortana-with-a-fake-gps-driver-on-windows-10/).

## Requirements

1. Windows 8 and up (only tested on Windows 10 x64)
2. Test Mode Enabled (`bcdedit /set testsigning on`)

Note that Test Mode may be incompatible with BitLocker and Secure Boot.

## Usage

* Ensure your system meets the requirements
* Download the latest release binary as a zip and extract to a folder
* Install the driver using "Add Legacy Hardware" in Device Manager
* Confirm you want to install the unsinged driver
    * The driver may say it failed to start on first use, see issue #3
* Open RegEdit and set the Latitude and Longitude
    * The default is 1.31337, 1.31337

Note the settings are currently stored in `HKLM\System\CurrentControlSet\Enum\ROOT\UNKNOWN\0000\Device Parameters\FakeGPS` but this will change, see issue #2

## Questions?

You can contact [@juliankay](https://twitter.com/juliankay) on Twitter, or open issues in this repository.