/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
	#define DELEGATECALL __stdcall
#else
	#define DELEGATECALL
#endif

typedef void (DELEGATECALL *PLATFORM_PrintFile)(
	void *data,
	const char *dir,
	const char *file
);

void PLATFORM_EnumerateFiles(
	const char *directory,
	void *data,
	PLATFORM_PrintFile callback
);

void PLATFORM_Sleep(int ms);

#undef DELEGATECALL

#endif /* PLATFORM_H */
