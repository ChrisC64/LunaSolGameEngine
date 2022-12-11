#pragma once

#ifdef LSE_API
#define LSE_EXPORT __declspec(dllexport)
#else
#define LSE_EXPORT __declspec(dllimport)
#endif


// Typedefs //
// Reference Pointers //
template <class T, typename Deleter = std::default_delete<T>>
using Ref = std::unique_ptr<T, Deleter>;
template <class S>
using SharedRef = std::shared_ptr<S>;
template<class T>
using LSOptional = std::optional<T>;

// DEFINES //
#ifdef _DEBUG
#define TRACE(x) {      \
std::stringstream ss;	\
ss << x;				\
OutputDebugStringA(ss.str().c_str());\
}
#define TRACE_W(x) {						\
std::wstringstream ws;					\
 ws << x;								\
 OutputDebugStringW(ws.str().c_str()); }

#define TRACE_ERR(x) std::cerr << "ERROR: [" << __FILE__<< "]" << " [" << __func__ << "] " << "[Line: " << __LINE__ << "]" << x;
#else
#define TRACE(x) // Does nothing (should output to a file maybe?)
#define TRACE_W(x)// Does nothing (maybe output to file?)
#define TRACE_ERR(x)//Should really output to file perhapse
#endif _DEBUG
