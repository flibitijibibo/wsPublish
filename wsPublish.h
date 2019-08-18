/* wsPublish - Steam Workshop Interop Library
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

#ifndef WSPUBLISH_H
#define WSPUBLISH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
	#define EXPORTFN __declspec(dllexport)
	#define DELEGATECALL __stdcall
#else
	#define EXPORTFN
	#define DELEGATECALL
#endif

/* Delegates for Steam callbacks */

typedef void (DELEGATECALL *STEAM_OnSharedFile)(const int32_t);

typedef void (DELEGATECALL *STEAM_OnPublishedFile)(
	const int32_t,
	const uint64_t
);

typedef void (DELEGATECALL *STEAM_OnUpdatedFile)(const int32_t);

typedef void (DELEGATECALL *STEAM_OnDeletedFile)(const int32_t);

typedef void (DELEGATECALL *STEAM_OnEnumeratedFiles)(
	const int32_t,
	const uint64_t*,
	const int
);

typedef void (DELEGATECALL *STEAM_OnReceivedFileInfo)(
	const uint64_t fileID,
	const char *title,
	const char *description,
	const char *tags
);

/* Steam Init/Update/Shutdown */

EXPORTFN int32_t STEAM_Initialize(
	const STEAM_OnSharedFile sharedFileDelegate,
	const STEAM_OnPublishedFile publishedFileDelegate,
	const STEAM_OnUpdatedFile updatedFileDelegate,
	const STEAM_OnDeletedFile deletedFileDelegate,
	const STEAM_OnEnumeratedFiles enumeratedFilesDelegate,
	const STEAM_OnReceivedFileInfo receivedFileInfoDelegate
);

EXPORTFN uint32_t STEAM_GetAppID();

EXPORTFN void STEAM_Update();

EXPORTFN void STEAM_Shutdown();

/* Steam Cloud */

EXPORTFN int32_t STEAM_IsCloudEnabled();

EXPORTFN int32_t STEAM_FileExists(const char *name);

EXPORTFN int32_t STEAM_WriteFile(
	const char *name,
	const void *data,
	const int32_t length
);

EXPORTFN int32_t STEAM_ReadFile(
	const char *name,
	void *data,
	const int32_t length
);

EXPORTFN int32_t STEAM_DeleteFile(const char *name);

EXPORTFN void STEAM_ShareFile(const char *name);

EXPORTFN int32_t STEAM_GetByteQuota(
	uint64_t *total,
	uint64_t *available
);

/* Steam UGC */

typedef enum
{
	STEAM_EFileVisibility_PUBLIC = 0,
	STEAM_EFileVisibility_FRIENDSONLY = 1,
	STEAM_EFileVisibility_PRIVATE = 2
} STEAM_EFileVisibility;

typedef enum
{
	STEAM_EFileType_COMMUNITY = 0,
	STEAM_EFileType_MICROTRANSACTION = 1,
	STEAM_EFileType_COLLECTION = 2,
	STEAM_EFileType_ART = 3,
	STEAM_EFileType_VIDEO = 4,
	STEAM_EFileType_SCREENSHOT = 5,
	STEAM_EFileType_GAME = 6,
	STEAM_EFileType_SOFTWARE = 7,
	STEAM_EFileType_CONCEPT = 8,
	STEAM_EFileType_WEBGUIDE = 9,
	STEAM_EFileType_INTEGRATEDGUIDE = 10,
	STEAM_EFileType_MAX = 11 /* DO NOT USE! */
} STEAM_EFileType;

EXPORTFN void STEAM_PublishFile(
	const uint32_t appid,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int32_t numTags,
	const STEAM_EFileVisibility visibility,
	const STEAM_EFileType type
);

EXPORTFN void STEAM_UpdatePublishedFile(
	const uint64_t fileID,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int32_t numTags,
	const STEAM_EFileVisibility visibility
);

EXPORTFN void STEAM_DeletePublishedFile(const uint64_t fileID);

EXPORTFN void STEAM_EnumeratePublishedFiles();

EXPORTFN void STEAM_GetPublishedFileInfo(
	const uint64_t fileID,
	const uint32_t secondsOld
);

#undef EXPORTFN
#undef DELEGATECALL

#ifdef __cplusplus
}
#endif

#endif /* WSPUBLISH_H */
