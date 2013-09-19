/* wsPublish - Steam Workshop Interop Library
 * Written by Ethan "flibitijibibo" Lee
 */

#include <wsPublish/wsPublish.h>
#include <steam/steam_api.h>

/* Callbacks */

class SteamCallbackContainer
{
private:
	/* TODO: Delegates... */

	/* TODO: Callbacks... */

public:
	SteamCallbackContainer(/* TODO: Function pointers */)
	{
		/* TODO: Assign private delegates */
	}

	/* TODO: Public async entry points */
};

static SteamCallbackContainer *callbackContainer;

/* Steam Init/Update/Shutdown */

int STEAM_Initialize(/* TODO: Function pointers */)
{
	if (!SteamAPI_Init())
	{
		return 0;
	}
	callbackContainer = new SteamCallbackContainer(/* TODO: Function pointers */);
	return 1;
}

void STEAM_Update()
{
	SteamAPI_RunCallbacks();
}

void STEAM_Shutdown()
{
	delete callbackContainer;
	SteamAPI_Shutdown();
}

/* Steam Cloud */

int STEAM_IsCloudEnabled()
{
	return (	SteamRemoteStorage()->IsCloudEnabledForAccount() &&
			SteamRemoteStorage()->IsCloudEnabledForApp()	);
}

int STEAM_FileExists(const char *name)
{
	return SteamRemoteStorage()->FileExists(name);
}

int STEAM_WriteFile(const char *name, const void *data, const int length)
{
	return SteamRemoteStorage()->FileWrite(name, data, length);
}

int STEAM_ReadFile(const char *name, void *data, const int length)
{
	return SteamRemoteStorage()->FileRead(name, data, length);
}

int STEAM_DeleteFile(const char *name)
{
	return SteamRemoteStorage()->FileDelete(name);
}

/* Steam UGC */

void STEAM_PublishFile(const char *name)
{
	// SteamAPICall_t callback = SteamRemoteStorage()->FileShare(name);
	/* TODO: Async call! */
	/* TODO: Be sure license agreement happens at the end of this! */
}

void STEAM_GetPublishedFile(const unsigned long fileID)
{
	/* TODO: GetFile with PublishedFileId_t */
}

void STEAM_UpdatePublishedFile(const unsigned long fileID)
{
	/* TODO: Run updates for all properties using PublishedFileId_t */
}

void STEAM_DeletePublishedFile(const unsigned long fileID)
{
	// SteamAPICall_t callback = DeletePublishedFile(fileID);
	/* TODO: Async call? */
}
