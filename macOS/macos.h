#if __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#include <libgen.h>
#include <mach-o/dyld.h>

/*
  Use this instead:
    - https://wiki.libsdl.org/SDL2/SDL_GetPrefPath
    - https://wiki.libsdl.org/SDL2/SDL_GetBasePath
*/

static int getExecutablePath(std::string &outputPath)
{
  outputPath.clear();

#if __APPLE__
  char exePath[2048];
  uint32_t bufSize = sizeof(exePath);
  int result = _NSGetExecutablePath(exePath, &bufSize);

  if (result == 0)
  {
    char *dir;
    exePath[sizeof(exePath) - 1] = 0;
    // printf("EXE Path: '%s' \n", exePath );

    dir = ::dirname(exePath);

    if (dir)
    {
      // printf("DIR Path: '%s' \n", dir );
      outputPath.assign(dir);
      return 0;
    }
  }
#endif

  return -1;
}

static int getResourcesPath(std::string &outputPath)
{
  outputPath.clear();
  if (getExecutablePath(outputPath) == -1)
  {
    return -1;
  }
  outputPath += "/../Resources/";
  return 0;
}

#endif
#endif
