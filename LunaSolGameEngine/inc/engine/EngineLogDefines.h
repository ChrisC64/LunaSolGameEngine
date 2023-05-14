#pragma once
import Engine.Logger;

namespace LS::Log
{
#if LS_ENABLE_LOG 
#define LS_LOG_ERROR(x) TraceError(x)
#define LS_LOG_DEBUG(x) TraceDebug(x)
#define LS_LOG_INFO(x) TraceInfo(x)
#define LS_LOG_WARNING(x) TraceWarning(x)
#else
#define LS_LOG_ERROR(x)
#define LS_LOG_DEBUG(x)
#define LS_LOG_INFO(x)
#define LS_LOG_WARNING(x)
#endif
}