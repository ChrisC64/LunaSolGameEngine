#pragma once
import Engine.Logger;
//TODO: Not working as expected, because I suck at preprocessor stuff. Okay, so it seems
// the definitions of Trace___ are not included. I'll work this out later. 
namespace LS::Log
{
#if LS_ENABLE_LOG 
#define LS_LOG_ERROR(x) TraceError(x)
#define LS_LOG_DEBUG(x) TraceDebug(x)
#define LS_LOG_INFO(x) TraceInfo(x)
#define LS_LOG_WARNING(x) TraceWarn(x)
#else
#define LS_LOG_ERROR(x)
#define LS_LOG_DEBUG(x)
#define LS_LOG_INFO(x)
#define LS_LOG_WARNING(x)
#endif
}