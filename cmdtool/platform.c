/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#include "platform.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#else
#warning You probably want to check your platform includes...
#endif

#include <physfs.h>

void PLATFORM_EnumerateFiles(
	const char *argv0,
	const char *directory,
	void *data,
	PLATFORM_PrintFile callback
) {
	/* TODO: Replace this, we should not need PhysicsFS. */
	PHYSFS_init(argv0);
	PHYSFS_mount(directory, directory, 1);
	PHYSFS_enumerateFilesCallback(
		directory,
		(PHYSFS_EnumFilesCallback) callback,
		data
	);
	PHYSFS_deinit();
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
