/* wsPublish Command Line Tool
 *
 * Copyright (c) 2013-2019 Ethan Lee, General Arcade
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Ethan "flibitijibibo" Lee <flibitijibibo@flibitijibibo.com>
 *
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
