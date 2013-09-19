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

int STEAM_Initialize(/* TODO: Function pointers */);

void STEAM_Update();

void STEAM_Shutdown();

/* Steam Cloud */

int STEAM_IsCloudEnabled();

int STEAM_FileExists(const char *name);

int STEAM_WriteFile(const char *name, const void *data, const int length);

int STEAM_ReadFile(const char *name, void *data, const int length);

int STEAM_DeleteFile(const char *name);

/* Steam UGC */

void STEAM_PublishFile(const char *name);

void STEAM_GetPublishedFile(const unsigned long fileID);

void STEAM_UpdatePublishedFile(const unsigned long fileID);

void STEAM_DeletePublishedFile(const unsigned long fileID);

#undef EXPORTFN
#undef DELEGATECALL

#ifdef __cplusplus
}
#endif

#endif /* WSPUBLISH_H */
