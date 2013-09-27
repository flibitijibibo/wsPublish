/* wsPublish - Steam Workshop Interop Library
 * Written by Ethan "flibitijibibo" Lee
 */

#ifndef WSPUBLISH_H
#define WSPUBLISH_H

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

typedef void (DELEGATECALL *STEAM_OnSharedFile)(const int);

typedef void (DELEGATECALL *STEAM_OnPublishedFile)(
	const int,
	const unsigned long
);

typedef void (DELEGATECALL *STEAM_OnUpdatedFile)(const int);

typedef void (DELEGATECALL *STEAM_OnDeletedFile)(const int);

typedef void (DELEGATECALL *STEAM_OnEnumeratedFiles)(
	const int,
	const unsigned long*,
	const int
);

typedef void (DELEGATECALL *STEAM_OnReceivedFileInfo)(
	const unsigned long fileID,
	const char *title,
	const char *description,
	const char *tags
);

/* Steam Init/Update/Shutdown */

EXPORTFN int STEAM_Initialize(
	const STEAM_OnSharedFile sharedFileDelegate,
	const STEAM_OnPublishedFile publishedFileDelegate,
	const STEAM_OnUpdatedFile updatedFileDelegate,
	const STEAM_OnDeletedFile deletedFileDelegate,
	const STEAM_OnEnumeratedFiles enumeratedFilesDelegate,
	const STEAM_OnReceivedFileInfo receivedFileInfoDelegate
);

EXPORTFN unsigned int STEAM_GetAppID();

EXPORTFN void STEAM_Update();

EXPORTFN void STEAM_Shutdown();

/* Steam Cloud */

EXPORTFN int STEAM_IsCloudEnabled();

EXPORTFN int STEAM_FileExists(const char *name);

EXPORTFN int STEAM_WriteFile(
	const char *name,
	const void *data,
	const int length
);

EXPORTFN int STEAM_ReadFile(const char *name, void *data, const int length);

EXPORTFN int STEAM_DeleteFile(const char *name);

EXPORTFN void STEAM_ShareFile(const char *name);

EXPORTFN int STEAM_GetByteQuota(int *total, int *available);

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
	const unsigned int appid,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int numTags,
	const STEAM_EFileVisibility visibility,
	const STEAM_EFileType type
);

EXPORTFN void STEAM_UpdatePublishedFile(
	const unsigned long fileID,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int numTags,
	const STEAM_EFileVisibility visibility
);

EXPORTFN void STEAM_DeletePublishedFile(const unsigned long fileID);

EXPORTFN void STEAM_EnumeratePublishedFiles();

EXPORTFN void STEAM_GetPublishedFileInfo(const unsigned long fileID);

#undef EXPORTFN
#undef DELEGATECALL

#ifdef __cplusplus
}
#endif

#endif /* WSPUBLISH_H */
