// 2016 OK

/*++

Module:

    Trace.h

Description:

    This module contains the tracing information for the driver.

--*/

#ifndef _TRACE_H_
#define _TRACE_H_

// Hacks around IntelliSense - someone smarter may know how best to handle this.
#if defined(__INTELLISENSE__)// && defined(Trace)
#define TRACE_LEVEL_NONE        0
#define TRACE_LEVEL_CRITICAL    1
#define TRACE_LEVEL_FATAL       1
#define TRACE_LEVEL_ERROR       2
#define TRACE_LEVEL_WARNING     3
#define TRACE_LEVEL_INFORMATION 4
#define TRACE_LEVEL_VERBOSE     5
#define TRACE_LEVEL_RESERVED6   6
#define TRACE_LEVEL_RESERVED7   7
#define TRACE_LEVEL_RESERVED8   8
#define TRACE_LEVEL_RESERVED9   9
#undef Trace
#define Trace(a,b,...)
#define TRACE_DRIVER
#endif
#pragma warning(disable:6387)


// Tracing Information
#define MYDRIVER_TRACING_ID L"FakeGPS"

// Tracing Definitions:
//
// Control GUID:
// (a9de1ebf-dead-beef-b3c7-5edb2f1f7b15)
#define WPP_CONTROL_GUIDS                             \
    WPP_DEFINE_CONTROL_GUID(                          \
        FakeGPSTraceGuid,                             \
        (a9de1ebf,dead,beef,b3c7,5edb2f1f7b15),       \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)             \
        )

#define WPP_FLAG_LEVEL_LOGGER(flag, level)            \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)           \
    (WPP_LEVEL_ENABLED(flag) &&                       \
        WPP_CONTROL(WPP_BIT_ ## flag).Level >= level  \
        )

// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC FuncEntry{FLAG=MYDRIVER_ALL_INFO, LEVEL=TRACE_LEVEL_VERBOSE}(...);
// FUNC FuncExit{FLAG=MYDRIVER_ALL_INFO, LEVEL=TRACE_LEVEL_VERBOSE}(...);
// USEPREFIX(Trace, "%!STDPREFIX! [%!FUNC!] ");
// USEPREFIX(FuncEntry, "%!STDPREFIX! [%!FUNC!] --> entry");
// USEPREFIX(FuncExit, "%!STDPREFIX! [%!FUNC!] <--");
// end_wpp

#endif // _TRACE_H_
