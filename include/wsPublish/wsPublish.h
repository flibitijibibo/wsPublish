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

/* Steam Init/Update/Shutdown */

EXPORTFN int STEAM_Initialize(/* TODO: Function pointers */);

EXPORTFN void STEAM_Update();

EXPORTFN void STEAM_Shutdown();

/* Steam Cloud */

EXPORTFN int STEAM_IsCloudEnabled();

EXPORTFN int STEAM_FileExists(const char *name);

EXPORTFN int STEAM_WriteFile(const char *name, const void *data, const int length);

EXPORTFN int STEAM_ReadFile(const char *name, void *data, const int length);

EXPORTFN int STEAM_DeleteFile(const char *name);

/* Steam UGC */

EXPORTFN void STEAM_PublishFile(const char *name);

EXPORTFN void STEAM_GetPublishedFile(const unsigned long fileID);

EXPORTFN void STEAM_UpdatePublishedFile(const unsigned long fileID);

EXPORTFN void STEAM_DeletePublishedFile(const unsigned long fileID);

#undef EXPORTFN
#undef DELEGATECALL

#ifdef __cplusplus
}
#endif

#endif /* WSPUBLISH_H */
