/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#include "platform.h"

/* Shut your face, Windows */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

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
	const char *directory,
	void *data,
	PLATFORM_PrintFile callback
) {
#if defined(_WIN32)
	WIN32_FIND_DATA findHandle;
	HANDLE hFind = NULL;
	char fileSearch[MAX_PATH];

	sprintf(fileSearch, "%s\\*.*", directory);
	hFind = FindFirstFile(fileSearch, &findHandle);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Could not find directory %s\n", directory);
		return;
	}
	do
	{
		if (	strcmp(findHandle.cFileName, "..") != 0 &&
			strcmp(findHandle.cFileName, ".") != 0	)
		{
			sprintf(fileSearch, "%s\\%s", directory, findHandle.cFileName);
			if ((findHandle.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				callback(data, directory, findHandle.cFileName);
			}
		}
	} while (FindNextFile(hFind, &findHandle));
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
#else
#error Need a PLATFORM_EnumerateFiles implementation!
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
