/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

/* Shut your face, Windows */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wsPublish/wsPublish.h>

#include "platform.h"

#include "json.h"

/* miniz, go home, you're drunk */
#if !defined(_WIN32)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wlong-long"
#endif

#include "miniz.c"

#if !defined(_WIN32)
#pragma GCC diagnostic pop
#endif
/* End miniz Alcoholism */

#define CMDTOOL_VERSION		"13.09.27"
#define MAX_FILENAME_SIZE	32
#define UPDATE_TIME_MS		1000

/* Structure used to store Steam Workshop item information */
typedef struct CMD_WorkshopItem_s
{
	char name[MAX_FILENAME_SIZE + 4];
	char previewName[MAX_FILENAME_SIZE + 4];
	char title[128 + 1];
	char description[8000];
	const char *tags[5];
	int numTags;
	STEAM_EFileVisibility visibility;
	STEAM_EFileType type;
} CMD_WorkshopItem_t;

/* Steam Workshop Item Information */
static CMD_WorkshopItem_t item;
static unsigned long itemID;
static char *itemName;

/* Used for `list` operation */
static unsigned long wsIDs[100]; /* FIXME: 100 is arbitrary! */
static int numWSIDs = 0;
static int currentWSID = 0;
static int lastWSID = -1;

/* Is the callback running? */
static int operationRunning = 1;

#ifdef _WIN32
#define DELEGATECALL __stdcall
#else
#define DELEGATECALL
#endif

void DELEGATECALL CMD_OnSharedFile(const int success)
{
	operationRunning = 0;
}

void DELEGATECALL CMD_OnPublishedFile(const int success, const unsigned long fileID)
{
	if (success)
	{
		printf("FileID: %lu\n", fileID);
	}
	else
	{
		printf("No FileID, due to failure\n");
	}
	operationRunning = 0;
}

void DELEGATECALL CMD_OnUpdatedFile(const int success)
{
	operationRunning = 0;
}

void DELEGATECALL CMD_OnDeletedFile(const int success)
{
	operationRunning = 0;
}

void DELEGATECALL CMD_OnEnumeratedFiles(
	const int success,
	const unsigned long *fileIDs,
	const int numFileIDs
) {
	int i;
	if (success)
	{
		numWSIDs = numFileIDs;
		for (i = 0; i < numFileIDs; i += 1)
		{
			wsIDs[i] = fileIDs[i];
		}
	}
	printf("Found %i files.\n", numFileIDs);
	operationRunning = 0;
}

void DELEGATECALL CMD_OnReceivedFileInfo(
	const unsigned long fileID,
	const char *title,
	const char *description,
	const char *tags
) {
	printf(
		"Workshop Info for file ID %lu:\n"
		"\tTitle: %s\n\tDescription: %s\n\tTags: %s\n",
		fileID, title, description, tags
	);
	currentWSID += 1;
}

void DELEGATECALL CMD_OnFileEnumerated(void *data, const char *dir, const char *file)
{
	char builtName[(MAX_FILENAME_SIZE * 2) + 1 + 4];

	sprintf(builtName, "%s/%s", dir, file);

	mz_zip_writer_add_file(
		data,
		builtName,
		builtName,
		NULL,
		0,
		MZ_DEFAULT_COMPRESSION
	);
	printf("\tAdded %s to zipfile.\n", builtName);
}

int CMD_List()
{
	int total, available;
	STEAM_GetByteQuota(&total, &available);
	printf(
		"Cloud Quota for AppID %i: %i Used, %i Available, %i Total\n\n",
		STEAM_GetAppID(),
		total - available,
		available,
		total
	);

	printf("Queueing Workshop list callback...\n");
	STEAM_EnumeratePublishedFiles();
	printf("Done!\n\nRunning callbacks...\n");
	while (operationRunning > 0)
	{
		printf(".");
		STEAM_Update();
		PLATFORM_Sleep(UPDATE_TIME_MS);
	}
	printf("Done!\n");
	if (numWSIDs > 0)
	{
		printf("Running published file queries...\n");
		while (currentWSID < numWSIDs)
		{
			if (currentWSID > lastWSID)
			{
				printf(
					"Getting file info for %lu",
					wsIDs[currentWSID]
				);
				STEAM_GetPublishedFileInfo(wsIDs[currentWSID]);
				lastWSID += 1;
			}
			printf(".");
			STEAM_Update();
			PLATFORM_Sleep(UPDATE_TIME_MS);
		}
		printf("Done!\n\n");
	}
	printf("Operation Completed!\n");

	STEAM_Shutdown();
	return 0;
}

int CMD_Delete(char *idString)
{
	unsigned long itemID;
	int stringLength;
	int i;

	itemID = strtoul(
		idString,
		NULL,
		0
	);
	stringLength = strlen(idString);
	for (i = 0; i < stringLength; i += 1)
	{
		if (idString[i] < '0' || idString[i] > '9')
		{
			itemID = 0;
			break;
		}
	}

	if (itemID == 0)
	{
		printf("%s is NOT a valid ID! Exiting.\n", idString);
		return 0;
	}

	printf("Queueing %s for Workshop removal...", idString);
	STEAM_DeletePublishedFile(itemID);
	printf("Done!\n\n");

	printf("Running Steam callbacks...\n");
	while (operationRunning > 0)
	{
		printf(".");
		STEAM_Update();
		PLATFORM_Sleep(UPDATE_TIME_MS);
	}
	printf("\nOperation Completed!\n");

	STEAM_Shutdown();
	return 0;
}

int main(int argc, char** argv)
{
	#define CHECK_STRING(string) (strcmp(argv[1], string) == 0)

	/* JSON/PNG Filename Storage */
	char fileName[(MAX_FILENAME_SIZE * 2) + 1 + 5];

	/* JSON Handles */
	json_value *parser;

	/* miniz Handle */
	mz_zip_archive zip;

	/* File I/O Variables */
	FILE *fileIn;
	void *fileData;
	size_t fileSize = 0;

	/* Category checks */
	int categories[3];

	/* String length storage */
	int stringLength;

	/* Iterator Variable */
	int i;

	/* Do not let printf buffer. */
	setbuf(stdout, NULL);

	/* Verify Command Line Arguments */

	if (	!(argc == 2 && CHECK_STRING("list")) &&
		!(argc == 3 &&
		(	CHECK_STRING("publish")  ||
			CHECK_STRING("delete")	)	) &&
		!((argc == 4 || argc == 5) && CHECK_STRING("update"))	)
	{
		printf(
			"Usage:\n"
			"\tcmdtool publish NAME       - Upload to Workshop\n"
			"\tcmdtool update NAME ID MSG - Update Workshop Entry\n"
			"\tcmdtool delete ID          - Delete Workshop Entry\n"
			"\tcmdtool list               - List Workshop Entries\n"
		);
		return 0;
	}

	/* Extra check for update's fildID */
	if (CHECK_STRING("update"))
	{
		stringLength = strlen(argv[3]);
		for (i = 0; i < stringLength; i += 1)
		{
			if (argv[3][i] < '0' || argv[3][i] > '9')
			{
				printf(
					"update FileID %s is not valid!\n",
					argv[3]
				);
				return 0;
			}
		}
	}

	/* A nice header message */
	printf(	"\n"
		"/***********************************************************\n"
		"*           wsPublish Command Line Tool %s           *\n"
		"***********************************************************/\n"
		"\n",
		CMDTOOL_VERSION
	);

	/* Initialize Steamworks */

	printf("Initializing Steam...\n");
	if (	!STEAM_Initialize(
			(STEAM_OnSharedFile) CMD_OnSharedFile,
			(STEAM_OnPublishedFile) CMD_OnPublishedFile,
			(STEAM_OnUpdatedFile) CMD_OnUpdatedFile,
			(STEAM_OnDeletedFile) CMD_OnDeletedFile,
			(STEAM_OnEnumeratedFiles) CMD_OnEnumeratedFiles,
			(STEAM_OnReceivedFileInfo) CMD_OnReceivedFileInfo
		)
	) {
		printf("Steam failed to initialize!\n");
		return 0;
	}
	printf("Steam initialized!\n\n");

	/* List is a unique operation. */
	if (CHECK_STRING("list"))
	{
		return CMD_List();
	}

	/* Delete is a unique operation. */
	if (CHECK_STRING("delete"))
	{
		return CMD_Delete(argv[2]);
	}

	/* Assign this for the callbacks */
	itemName = argv[2];

	/* Verify Workshop Item Ccripts */

	printf("Verifying Workshop item JSON script...\n");
	printf("Reading %s/%s.json...", argv[2], argv[2]);

	/* Open file */
	sprintf(fileName, "%s/%s.json", argv[2], argv[2]);
	fileIn = fopen(fileName, "r");
	if (!fileIn)
	{
		printf(" %s was not found! Exiting.\n", fileName);
		goto cleanup;
	}

	/* Read the JSON file into memory. Ugh. */
	fseek(fileIn, 0, SEEK_END);
	fileSize = ftell(fileIn);
	rewind(fileIn);
	fileData = malloc(sizeof(char) * fileSize);
	fread(fileData, 1, fileSize, fileIn);

	/* We're done with the file at this point. */
	fclose(fileIn);

	/* Send the JSON string to the parser. */
	parser = json_parse(fileData, fileSize);

	/* We're done with the data at this point. */
	free(fileData);

	/* Begin filling in the Workshop Item struct. */
	sprintf(item.name, "%s.zip", argv[2]);
	sprintf(item.previewName, "%s.png", argv[2]);
	item.type = STEAM_EFileType_COMMUNITY;
	item.visibility = STEAM_EFileVisibility_PUBLIC;

	/* Verify the JSON script. */
	#define PARSE_ERROR(output) \
		printf(" ERROR: %s! Exiting.\n", output); \
		json_value_free(parser); \
		goto cleanup;
	if (parser->type != json_object)
	{
		PARSE_ERROR("Expected Item JSON Object")
	}
	if (parser->u.object.length != 4)
	{
		PARSE_ERROR("Expected only 4 values")
	}
	if (	strcmp(parser->u.object.values[0].name, "Title") != 0 ||
		strcmp(parser->u.object.values[1].name, "Description") != 0 ||
		strcmp(parser->u.object.values[2].name, "Type") != 0 ||
		strcmp(parser->u.object.values[3].name, "Category") != 0	)
	{
		PARSE_ERROR("Expected Title, Description, Type, Category")
	}
	if (parser->u.object.values[0].value->type != json_string)
	{
		PARSE_ERROR("Title is not a string")
	}
	if (parser->u.object.values[0].value->u.string.length > 128)
	{
		PARSE_ERROR("Title is longer than 128 characters")
	}
	if (parser->u.object.values[1].value->type != json_string)
	{
		PARSE_ERROR("Description is not a string")
	}
	if (parser->u.object.values[1].value->u.string.length > 8000)
	{
		PARSE_ERROR("Description is longer than 8000 characters")
	}
	if (parser->u.object.values[2].value->type != json_string)
	{
		PARSE_ERROR("Type is not a string")
	}
	if (parser->u.object.values[3].value->type != json_array)
	{
		PARSE_ERROR("Category is not an array")
	}
	if (parser->u.object.values[3].value->u.array.length < 1)
	{
		PARSE_ERROR("Category is an empty array")
	}
	if (parser->u.object.values[3].value->u.array.length > 3)
	{
		PARSE_ERROR("Category has more than 3 elements")
	}
	for (	i = 0;
		i < parser->u.object.values[3].value->u.array.length;
		i += 1
	) {
		if (parser->u.object.values[3].value->u.array.values[i]->type != json_string)
		{
			PARSE_ERROR("Category element is not a string")
		}
	}

	/* Interpret the JSON script */
	strcpy(
		item.title,
		parser->u.object.values[0].value->u.string.ptr
	);
	strcpy(
		item.description,
		parser->u.object.values[1].value->u.string.ptr
	);

	/* TODO: Just check for !Map, do something like categories later. */
	if (strcmp(parser->u.object.values[2].value->u.string.ptr, "Map") != 0)
	{
		PARSE_ERROR("Type: Expected Map")
	}

	/* Clear the categories before moving forward. */
	categories[0] = 0;
	categories[1] = 0;
	categories[2] = 0;

	/* Parse the category tags */
	for (	i = 0;
		i < parser->u.object.values[3].value->u.array.length;
		i += 1
	) {
		#define COMPARE_TAG(categoryName) \
			strcmp( \
				parser->u.object.values[3].value->u.array.values[i]->u.string.ptr, \
				categoryName \
			) == 0
		if (COMPARE_TAG("Singleplayer"))
		{
			categories[0] = 1;
		}
		else if (COMPARE_TAG("Coop"))
		{
			categories[1] = 1;
		}
		else if (COMPARE_TAG("Deathmatch"))
		{
			categories[2] = 1;
		}
		else
		{
			PARSE_ERROR("Category: Expected Singleplayer, Coop, Deathmatch")
		}
		#undef COMPARE_TAG
	}
	#undef PARSE_ERROR

	/* TODO: Assuming Map, do something like categories later. */
	item.tags[0] = CMDTOOL_VERSION;
	item.tags[1] = "Map";
	item.numTags = 2;
	if (categories[0])
	{
		item.tags[item.numTags] = "Singleplayer";
		item.numTags += 1;
	}
	if (categories[1])
	{
		item.tags[item.numTags] = "Coop";
		item.numTags += 1;
	}
	if (categories[2])
	{
		item.tags[item.numTags] = "Deathmatch";
		item.numTags += 1;
	}

	/* Clean up. */
	json_value_free(parser);

	printf(" Done!\n");
	printf("Verification complete! Beginning Workshop operation.\n\n");

	/* Upload the file */

	/* Create the zipfile */
	printf("Zipping %s folder to heap...\n", argv[2]);
	memset(&zip, 0, sizeof(zip));
	if (!mz_zip_writer_init_heap(&zip, 0, 0))
	{
		printf("Could not open up zip! Exiting.\n");
		goto cleanup;
	}
	PLATFORM_EnumerateFiles(
		argv[2],
		&zip,
		(PLATFORM_PrintFile) CMD_OnFileEnumerated
	);
	if (	!mz_zip_writer_finalize_heap_archive(
			&zip,
			&fileData,
			&fileSize
		)
	) {
		printf("Zipping went wrong! Exiting\n");
		mz_zip_writer_end(&zip);
		goto cleanup;
	}
	printf("Zipping Completed!\n");

	/* Write to Steam Cloud */
	printf("Writing %s to the cloud...", argv[2]);
	if (	!STEAM_WriteFile(
			item.name,
			fileData,
			fileSize
		)
	) {
		printf(" Cloud write failed! Exiting.\n");
		mz_zip_writer_end(&zip);
		goto cleanup;
	}
	mz_zip_writer_end(&zip);
	printf(" Done!\n");

	/* Read the PNG file into memory. Ugh. */
	sprintf(fileName, "%s/%s.png", argv[2], argv[2]);
	fileIn = fopen(fileName, "rb");
	fseek(fileIn, 0, SEEK_END);
	fileSize = ftell(fileIn);
	rewind(fileIn);
	fileData = malloc(sizeof(char) * fileSize);
	fread(fileData, 1, fileSize, fileIn);

	/* Write the PNG file to Steam Cloud */
	printf("Writing %s preview image to cloud...", argv[2]);
	if (	!STEAM_WriteFile(
			item.previewName,
			fileData,
			fileSize
		)
	) {
		printf(" Cloud write failed! Exiting.\n");
		free(fileData);
		goto cleanup;
	}
	free(fileData);
	printf(" Done!\n");

	/* Mark Steam Cloud files as shared */
	printf("Queueing %s for Steam file share...", argv[2]);
	STEAM_ShareFile(item.name);
	STEAM_ShareFile(item.previewName);
	printf(" Done!\n\n");

	/* Wait for the Share operation to complete. */

	printf("Running Steam callbacks...\n");
	while (operationRunning > 0)
	{
		printf(".");
		STEAM_Update();
		PLATFORM_Sleep(UPDATE_TIME_MS);
	}
	printf("\nOperation Completed!\n");

	/* Now, we either publish or update. */

	/* But wait, there's more! */
	operationRunning = 1;

	/* Ensure all files are on Steam Cloud */
	printf("\nVerifying Steam Cloud entries...");

	/* Check for the zipfile in Steam Cloud */
	if (	STEAM_FileExists(item.name) &&
		STEAM_FileExists(item.previewName) )
	{
		printf(" %s is in the cloud.\n", argv[2]);
	}
	else
	{
		printf(
			" %s is not in the cloud! Exiting.\n",
			argv[2]
		);
		goto cleanup;
	}
	printf("Verification complete! Beginning publish process.\n\n");

	if (CHECK_STRING("publish"))
	{
		/* Publish all files on Steam Workshop */
		printf("Queueing %s for Workshop publication...", argv[2]);
		STEAM_PublishFile(
			STEAM_GetAppID(),
			item.name,
			item.previewName,
			item.title,
			item.description,
			item.tags,
			item.numTags,
			item.visibility,
			item.type
		);
		printf(" Done!\n\n");
	}
	else if (CHECK_STRING("update"))
	{
		/* Parse the ID. We have already checked the integrity. */
		itemID = strtoul(
			argv[3],
			NULL,
			0
		);

		/* Queue each Steam Workshop update */
		printf(
			"Queueing %s for Workshop update with ID %lu...",
			argv[2],
			itemID
		);
		STEAM_UpdatePublishedFile(
			itemID,
			item.name,
			item.previewName,
			item.title,
			item.description,
			item.tags,
			item.numTags,
			item.visibility
		);
		printf(" Done!\n\n");
	}

	/* Wait for the Publish/Update operation to complete. */

	printf("Running Steam callbacks...\n");
	while (operationRunning > 0)
	{
		printf(".");
		STEAM_Update();
		PLATFORM_Sleep(UPDATE_TIME_MS);
	}
	printf("\nOperation Completed!\n");

	/* Clean up. We out. */
cleanup:
	STEAM_Shutdown();
	return 0;

	#undef CHECK_STRING
}
