/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#include "platform.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <dirent.h>
#else
#warning You probably want to check your platform includes...
#endif

void PLATFORM_EnumerateFiles(
	const char *argv0,
	const char *directory,
	void *data,
	PLATFORM_PrintFile callback
) {
#if defined(_WIN32)
#error TODO: Win32 directory listing!
#elif defined(__linux__) || defined(__APPLE__)
	DIR *dir = NULL;
	struct dirent *de = NULL;

	dir = opendir(directory);
	if (!dir)
	{
		printf("Could not find directory %s\n", directory);
		return;
	}
	for (de = readdir(dir); de != NULL; de = readdir(dir))
	{
		if (	strcmp(de->d_name, "..") == 0 ||
			strcmp(de->d_name, ".") == 0	)
		{
			continue;
		}
		callback(data, directory, de->d_name);
	}
#endif
}

const char *PLATFORM_GetDirectorySeparator()
{
#if defined(_WIN32)
	return "\\";
#elif defined(__linux__) || defined(__APPLE__)
	return "/";
#else
#error Need a PLATFORM_GetDirectorySeparator implementation!
#endif
}

void PLATFORM_Sleep(int ms)
{
#if defined(_WIN32)
	Sleep(ms);
#elif defined(__linux__) || defined(__APPLE__)
	usleep(ms * 1000);
#else
#error Need a PLATFORM_Sleep implementation!
#endif
}
