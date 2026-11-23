#include "version.h"

#if __has_include("build_info.h")
#include "build_info.h"
#else
#define APP_BUILD_ID "unknown"
#endif

/**
 * @brief Returns the generated source-control build identifier.
 * @return Null-terminated build identifier.
 */
const char *get_app_build_id()
{
  return APP_BUILD_ID;
}

/**
 * @brief Returns the complete user-facing application version.
 * @return Null-terminated version string.
 */
const char *get_app_version_string()
{
#if defined(_DEBUG) || defined(_RELEASE)
  return APP_VERSION "-dev+" APP_BUILD_ID;
#else
  return APP_VERSION;
#endif
}
