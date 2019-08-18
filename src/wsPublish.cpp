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
	STEAM_OnEnumeratedFiles enumeratedFilesDelegate;
	STEAM_OnReceivedFileInfo receivedFileInfoDelegate;

public:
	SteamCallbackContainer(
		const STEAM_OnSharedFile sharedFile,
		const STEAM_OnPublishedFile publishedFile,
		const STEAM_OnUpdatedFile updatedFile,
		const STEAM_OnDeletedFile deletedFile,
		const STEAM_OnEnumeratedFiles enumeratedFiles,
		const STEAM_OnReceivedFileInfo receivedFileInfo
	) {
		sharedFileDelegate = sharedFile;
		publishedFileDelegate = publishedFile;
		updatedFileDelegate = updatedFile;
		deletedFileDelegate = deletedFile;
		enumeratedFilesDelegate = enumeratedFiles;
		receivedFileInfoDelegate = receivedFileInfo;
	}

	#define CHECK_FAILURE(CallbackName) \
		if (bIOFailure) \
		{ \
			printf( \
				"\nFile %s: INTERNAL STEAM ERROR: %i\n", \
				CallbackName, \
				result->m_eResult \
			); \
		} \
		else \
		{ \
			printf("\nFile %s succeeded!\n", CallbackName); \
		}

	void SharedFile(
		RemoteStorageFileShareResult_t *result,
		bool bIOFailure
	) {
		CHECK_FAILURE("Share")
		if (sharedFileDelegate)
		{
			sharedFileDelegate(!bIOFailure);
		}
	}

	void PublishedFile(
		RemoteStoragePublishFileResult_t *result,
		bool bIOFailure
	) {
		if (!bIOFailure)
		{
			printf(
				"\nFile Publish succeeded! FileID: %llu\n",
				result->m_nPublishedFileId
			);
		}
		else
		{
			printf("\nFile Publish failed! No FileID returned.\n");
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
		CHECK_FAILURE("Update")
		if (updatedFileDelegate)
		{
			updatedFileDelegate(!bIOFailure);
		}
	}

	void DeletedFile(
		RemoteStorageDeletePublishedFileResult_t *result,
		bool bIOFailure
	) {
		CHECK_FAILURE("Delete")
		if (deletedFileDelegate)
		{
			deletedFileDelegate(!bIOFailure);
		}
	}

	void EnumeratedFiles(
		RemoteStorageEnumerateUserPublishedFilesResult_t *result,
		bool bIOFailure
	) {
		uint64_t retVals[result->m_nResultsReturned];
		int32_t i;
		if (!bIOFailure)
		{
			printf(
				"\nListing succeeded! %i files found.\n",
				result->m_nResultsReturned
			);
		}
		else
		{
			printf("\nListing failed! No files found.\n");
		}
		if (enumeratedFilesDelegate)
		{
			for (i = 0; i < result->m_nResultsReturned; i++)
			{
				retVals[i] = result->m_rgPublishedFileId[i];
			}
			enumeratedFilesDelegate(
				!bIOFailure,
				retVals,
				result->m_nResultsReturned
			);
		}
	}

	void ReceivedFileInfo(
		RemoteStorageGetPublishedFileDetailsResult_t *result,
		bool bIOFailure
	) {
		if (!bIOFailure)
		{	
			printf(
				"\nWorkshop Info for file ID %llu:\n"
				"\tTitle: %s\n\tDescription: %s\n\tTags: %s\n",
				result->m_nPublishedFileId,
				result->m_rgchTitle,
				result->m_rgchDescription,
				result->m_rgchTags
			);
		}
		else
		{
			printf("\nFile Listing failed! No info found.\n");
		}
		if (receivedFileInfoDelegate)
		{
			receivedFileInfoDelegate(
				result->m_nPublishedFileId,
				result->m_rgchTitle,
				result->m_rgchDescription,
				result->m_rgchTags
			);
		}
	}

	#undef CHECK_FAILURE
};

static SteamCallbackContainer *callbackContainer;

/* Steam Init/Update/Shutdown */

int32_t STEAM_Initialize(
	const STEAM_OnSharedFile sharedFileDelegate,
	const STEAM_OnPublishedFile publishedFileDelegate,
	const STEAM_OnUpdatedFile updatedFileDelegate,
	const STEAM_OnDeletedFile deletedFileDelegate,
	const STEAM_OnEnumeratedFiles enumeratedFilesDelegate,
	const STEAM_OnReceivedFileInfo receivedFileInfoDelegate
) {
	if (!SteamAPI_Init())
	{
		return 0;
	}
	callbackContainer = new SteamCallbackContainer(
		sharedFileDelegate,
		publishedFileDelegate,
		updatedFileDelegate,
		deletedFileDelegate,
		enumeratedFilesDelegate,
		receivedFileInfoDelegate
	);
	return 1;
}

uint32_t STEAM_GetAppID()
{
	return SteamUtils()->GetAppID();
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

int32_t STEAM_IsCloudEnabled()
{
	return (	SteamRemoteStorage()->IsCloudEnabledForAccount() &&
			SteamRemoteStorage()->IsCloudEnabledForApp()	);
}

int32_t STEAM_FileExists(const char *name)
{
	return SteamRemoteStorage()->FileExists(name);
}

int32_t STEAM_WriteFile(
	const char *name,
	const void *data,
	const int32_t length
) {
	return SteamRemoteStorage()->FileWrite(name, data, length);
}

int32_t STEAM_ReadFile(const char *name, void *data, const int32_t length)
{
	return SteamRemoteStorage()->FileRead(name, data, length);
}

int32_t STEAM_DeleteFile(const char *name)
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

int32_t STEAM_GetByteQuota(uint64_t *total, uint64_t *available)
{
	return SteamRemoteStorage()->GetQuota(
		(uint64*) total,
		(uint64*) available
	);
}

/* Steam UGC */

void STEAM_PublishFile(
	const uint32_t appid,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int32_t numTags,
	const STEAM_EFileVisibility visibility,
	const STEAM_EFileType type
) {
	static CCallResult<SteamCallbackContainer, RemoteStoragePublishFileResult_t> filePublishResult;

	SteamParamStringArray_t stringParams;
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
	const uint64_t fileID,
	const char *name,
	const char *previewName,
	const char *title,
	const char *description,
	const char **tags,
	const int32_t numTags,
	const STEAM_EFileVisibility visibility
) {
	static CCallResult<SteamCallbackContainer, RemoteStorageUpdatePublishedFileResult_t> fileUpdateResult;

	PublishedFileUpdateHandle_t handle = SteamRemoteStorage()->CreatePublishedFileUpdateRequest(fileID);

	SteamParamStringArray_t stringParams;
	stringParams.m_ppStrings = tags;
	stringParams.m_nNumStrings = numTags;

	SteamRemoteStorage()->UpdatePublishedFileFile(handle, name);
	SteamRemoteStorage()->UpdatePublishedFilePreviewFile(
		handle,
		previewName
	);
	SteamRemoteStorage()->UpdatePublishedFileTitle(handle, title);
	SteamRemoteStorage()->UpdatePublishedFileDescription(
		handle,
		description
	);
	SteamRemoteStorage()->UpdatePublishedFileTags(handle, &stringParams);
	SteamRemoteStorage()->UpdatePublishedFileVisibility(
		handle,
		(ERemoteStoragePublishedFileVisibility) visibility
	);

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

void STEAM_DeletePublishedFile(const uint64_t fileID)
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

void STEAM_EnumeratePublishedFiles()
{
	static CCallResult<SteamCallbackContainer, RemoteStorageEnumerateUserPublishedFilesResult_t> enumerateFilesResult;

	SteamAPICall_t hSteamAPICall = 0;
	hSteamAPICall = SteamRemoteStorage()->EnumerateUserPublishedFiles(0);

	if (hSteamAPICall != 0)
	{
		enumerateFilesResult.Set(
			hSteamAPICall,
			callbackContainer,
			&SteamCallbackContainer::EnumeratedFiles
		);
	}
	else
	{
		printf("Steam enumerate published files did not happen! D:\n");
	}
}

void STEAM_GetPublishedFileInfo(
	const uint64_t fileID,
	const uint32_t secondsOld
) {
	static CCallResult<SteamCallbackContainer, RemoteStorageGetPublishedFileDetailsResult_t> receivedFileResult;

	SteamAPICall_t hSteamAPICall = 0;
	hSteamAPICall = SteamRemoteStorage()->GetPublishedFileDetails(
		fileID,
		secondsOld
	);

	if (hSteamAPICall != 0)
	{
		receivedFileResult.Set(
			hSteamAPICall,
			callbackContainer,
			&SteamCallbackContainer::ReceivedFileInfo
		);
	}
	else
	{
		printf("Steam get published file info did not happen! D:\n");
	}
}
