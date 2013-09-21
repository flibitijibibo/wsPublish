/* wsPublish - Steam Workshop Interop Library
 * Written by Ethan "flibitijibibo" Lee
 */

#include <wsPublish/wsPublish.h>
#include <steam/steam_api.h>

/* Callbacks */

class SteamCallbackContainer
{
private:
	STEAM_OnSharedFile sharedFileDelegate;
	STEAM_OnPublishedFile publishedFileDelegate;
	STEAM_OnUpdatedFile updatedFileDelegate;
	STEAM_OnDeletedFile deletedFileDelegate;

public:
	SteamCallbackContainer(
		const STEAM_OnSharedFile sharedFile,
		const STEAM_OnPublishedFile publishedFile,
		const STEAM_OnUpdatedFile updatedFile,
		const STEAM_OnDeletedFile deletedFile
	) {
		sharedFileDelegate = sharedFile;
		publishedFileDelegate = publishedFile;
		updatedFileDelegate = updatedFile;
		deletedFileDelegate = deletedFile;
	}

	void SharedFile(
		RemoteStorageFileShareResult_t *result,
		bool bIOFailure
	) {
		if (bIOFailure)
		{
			printf(
				"SharedFile: INTERNAL STEAM ERROR: %i\n",
				result->m_eResult
			);
		}
		else
		{
			printf("SharedFile: SUCCESS\n");
		}
		if (sharedFileDelegate)
		{
			sharedFileDelegate(!bIOFailure);
		}
	}

	void PublishedFile(
		RemoteStoragePublishFileResult_t *result,
		bool bIOFailure
	) {
		if (bIOFailure)
		{
			printf(
				"PublishedFile: INTERNAL STEAM ERROR: %i\n",
				result->m_eResult
			);
		}
		else
		{
			printf("PublishedFile: SUCCESS\n");
		}
		if (publishedFileDelegate)
		{
			publishedFileDelegate(
				!bIOFailure,
				result->m_nPublishedFileId
			);
		}
	}

	void UpdatedFile(
		RemoteStorageUpdatePublishedFileResult_t *result,
		bool bIOFailure
	) {
		if (bIOFailure)
		{
			printf(
				"UpdatedFile: INTERNAL STEAM ERROR: %i\n",
				result->m_eResult
			);
		}
		else
		{
			printf("UpdatedFile: SUCCESS\n");
		}
		if (updatedFileDelegate)
		{
			updatedFileDelegate(!bIOFailure);
		}
	}

	void DeletedFile(
		RemoteStorageDeletePublishedFileResult_t *result,
		bool bIOFailure
	) {
		if (bIOFailure)
		{
			printf(
				"DeletedFile: INTERNAL STEAM ERROR: %i\n",
				result->m_eResult
			);
		}
		else
		{
			printf("DeletedFile: SUCCESS\n");
		}
		if (deletedFileDelegate)
		{
			deletedFileDelegate(!bIOFailure);
		}
	}
};

static SteamCallbackContainer *callbackContainer;

/* Steam Init/Update/Shutdown */

int STEAM_Initialize(
	const STEAM_OnSharedFile sharedFileDelegate,
	const STEAM_OnPublishedFile publishedFileDelegate,
	const STEAM_OnUpdatedFile updatedFileDelegate,
	const STEAM_OnDeletedFile deletedFileDelegate
) {
	if (!SteamAPI_Init())
	{
		return 0;
	}
	callbackContainer = new SteamCallbackContainer(
		sharedFileDelegate,
		publishedFileDelegate,
		updatedFileDelegate,
		deletedFileDelegate
	);
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

void STEAM_ShareFile(const char *name)
{
	static CCallResult<SteamCallbackContainer, RemoteStorageFileShareResult_t> fileSharedResult;

	SteamAPICall_t hSteamAPICall = 0;
	hSteamAPICall = SteamRemoteStorage()->FileShare(name);

	if (hSteamAPICall != 0)
	{
		fileSharedResult.Set(
			hSteamAPICall,
			callbackContainer,
			&SteamCallbackContainer::SharedFile
		);
	}
	else
	{
		printf("Steam file share did not happen! D:\n");
	}
}

/* Steam UGC */

void STEAM_PublishFile(
	const unsigned int appid,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int numTags,
	const STEAM_EFileVisibility visibility,
	const STEAM_EFileType type
) {
	static CCallResult<SteamCallbackContainer, RemoteStoragePublishFileResult_t> filePublishResult;
	static SteamParamStringArray_t stringParams;

	stringParams.m_ppStrings = tags;
	stringParams.m_nNumStrings = numTags;

	SteamAPICall_t hSteamAPICall = 0;
	hSteamAPICall = SteamRemoteStorage()->PublishWorkshopFile(
		name,
		previewName,
		appid,
		title,
		description,
		(ERemoteStoragePublishedFileVisibility) visibility,
		&stringParams,
		(EWorkshopFileType) type
	);

	if (hSteamAPICall != 0)
	{
		filePublishResult.Set(
			hSteamAPICall,
			callbackContainer,
			&SteamCallbackContainer::PublishedFile
		);
	}
	else
	{
		printf("Steam file publish did not happen! D:\n");
	}
}

void STEAM_UpdatePublishedFile(
	const unsigned long fileID,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int numTags,
	const STEAM_EFileVisibility visibility
) {
	PublishedFileUpdateHandle_t handle = SteamRemoteStorage()->CreatePublishedFileUpdateRequest(fileID);

	SteamParamStringArray_t stringParams;
	stringParams.m_ppStrings = tags;
	stringParams.m_nNumStrings = numTags;

	SteamRemoteStorage()->UpdatePublishedFileFile(handle, name);
	SteamRemoteStorage()->UpdatePublishedFilePreviewFile(handle, previewName);
	SteamRemoteStorage()->UpdatePublishedFileTitle(handle, title);
	SteamRemoteStorage()->UpdatePublishedFileDescription(handle, description);
	SteamRemoteStorage()->UpdatePublishedFileTags(handle, &stringParams);
	SteamRemoteStorage()->UpdatePublishedFileVisibility(handle, (ERemoteStoragePublishedFileVisibility) visibility);

	static CCallResult<SteamCallbackContainer, RemoteStorageUpdatePublishedFileResult_t> fileUpdateResult;
	SteamAPICall_t hSteamAPICall = 0;
	hSteamAPICall = SteamRemoteStorage()->CommitPublishedFileUpdate(handle);

	if (hSteamAPICall != 0)
	{
		fileUpdateResult.Set(
			hSteamAPICall,
			callbackContainer,
			&SteamCallbackContainer::UpdatedFile
		);
	}
	else
	{
		printf("Steam file update did not happen! D:\n");
	}
}

void STEAM_DeletePublishedFile(const unsigned long fileID)
{
	static CCallResult<SteamCallbackContainer, RemoteStorageDeletePublishedFileResult_t> fileDeleteResult;

	SteamAPICall_t hSteamAPICall = 0;
	hSteamAPICall = SteamRemoteStorage()->DeletePublishedFile(fileID);

	if (hSteamAPICall != 0)
	{
		fileDeleteResult.Set(
			hSteamAPICall,
			callbackContainer,
			&SteamCallbackContainer::DeletedFile
		);
	}
	else
	{
		printf("Steam file delete did not happen! D:\n");
	}
}
