#pragma
#include <minitest.h>

#ifdef _WIN32
#ifdef shared_lib_EXPORTS
#define SHARED_LIB_EXPORT __declspec(dllexport)
#else
#define SHARED_LIB_EXPORT __declspec(dllimport)
#endif // *_EXPORTS
#else
#define SHARED_LIB_EXPORT
#endif // _WIN32
namespace minitest
{
namespace shared_lib
{
SHARED_LIB_EXPORT void shared_lib_dummy_func();
} // namespace shared_lib
} // namespace minitest