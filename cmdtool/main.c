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

#define MAX_FILENAME_SIZE	32
#define UPDATE_TIME_MS		1000

/* Structure used to store Steam Workshop item information */
typedef struct CMD_WorkshopItem_s
{
	char name[MAX_FILENAME_SIZE + 4];
	char previewName[MAX_FILENAME_SIZE + 4];
	char title[64]; /* FIXME: This character limit is arbitrary! */
	char description[8000]; /* Max, according to TF2 wiki */
	char **tags; /* FIXME: Not using malloc here would be super rad. */
	int numTags;
	STEAM_EFileVisibility visibility;
	STEAM_EFileType type;
} CMD_WorkshopItem_t;

/* Steam Workshop Item Information */
static CMD_WorkshopItem_t item;
static unsigned long itemID;
static char *itemName;

/* Is the callback running? */
static int operationRunning = 1;

void CMD_OnSharedFile(const int success)
{
	operationRunning = 0;
}

void CMD_OnPublishedFile(const int success, const unsigned long fileID)
{
	FILE *fileOut;
	char builtPath[MAX_FILENAME_SIZE + 5];
	if (success)
	{
		printf("FileID: %lu\n", fileID);

		strcpy(builtPath, itemName);
		strcat(builtPath, ".wsid");

		fileOut = fopen(builtPath, "w");
		fprintf(fileOut, "%lu", fileID);
		fclose(fileOut);
	}
	else
	{
		printf("No FileID, due to failure\n");
	}
	operationRunning = 0;
}

void CMD_OnUpdatedFile(const int success)
{
	operationRunning = 0;
}

void CMD_OnDeletedFile(const int success)
{
	char builtPath[MAX_FILENAME_SIZE + 5];
	if (success)
	{
		strcpy(builtPath, itemName);
		strcat(builtPath, ".wsid");

		remove(builtPath);

		printf("%s has been deleted.\n", builtPath);
	}
	else
	{
		printf("FileID still exists, due to failure\n");
	}
	operationRunning = 0;
}

void CMD_OnFileEnumerated(void *data, const char *dir, const char *file)
{
	char builtName[(MAX_FILENAME_SIZE * 2) + 1 + 4];
	strcpy(builtName, dir);
	strcat(builtName, PLATFORM_GetDirectorySeparator());
	strcat(builtName, file);
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

unsigned long CMD_GetFileID(const char *name)
{
	unsigned long returnVal;
	char builtPath[MAX_FILENAME_SIZE + 5];
	FILE *fileIn;

	strcpy(builtPath, name);
	strcat(builtPath, ".wsid");

	fileIn = fopen(builtPath, "r");
	if (!fileIn)
	{
		return 0;
	}

	returnVal = strtoul(
		fgets(builtPath, 21, fileIn),
		NULL,
		0
	);

	fclose(fileIn);
	return returnVal;
}

int main(int argc, char** argv)
{
	#define CHECK_STRING(string) (strcmp(argv[1], string) == 0)

	/* JSON Filename */
	char jsonName[MAX_FILENAME_SIZE + 5];

	/* JSON Handles */
	json_value *parser;

	/* miniz Handle */
	mz_zip_archive zip;

	/* File I/O Variables */
	FILE *fileIn;
	void *fileData;
	size_t fileSize = 0;

	/* Iterator Variable */
	int i;

	/* Verify Command Line Arguments */

	if (	argc < 3 ||
		argc > 3 ||
		!(	CHECK_STRING("upload")  ||
			CHECK_STRING("publish") ||
			CHECK_STRING("update")  ||
			CHECK_STRING("delete")	)	)
	{
		printf(
			"Usage: %s ACTION FOLDER\n"
			"\tACTION: upload, publish, update, delete\n"
			"\tFOLDER: Folder name of the Workshop item\n",
			argv[0]
		);
		return 0;
	}

	/* Initialize Steamworks */

	printf("Initializing Steam...\n");
	if (	!STEAM_Initialize(
			(STEAM_OnSharedFile) CMD_OnSharedFile,
			(STEAM_OnPublishedFile) CMD_OnPublishedFile,
			(STEAM_OnUpdatedFile) CMD_OnUpdatedFile,
			(STEAM_OnDeletedFile) CMD_OnDeletedFile
		)
	) {
		printf("Steam failed to initialize!\n");
		return 0;
	}
	printf("Steam initialized!\n\n");

	/* Assign this for the callbacks */
	itemName = argv[2];

	/* Verify Workshop Item Ccripts */

	printf("Verifying Workshop item JSON script...\n");
	printf("Reading %s.json...", argv[2]);

	/* Open file */
	strcpy(jsonName, argv[2]);
	strcat(jsonName, ".json");
	fileIn = fopen(jsonName, "r");
	if (!fileIn)
	{
		printf(" %s was not found! Exiting.\n", jsonName);
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
	strcpy(item.name, argv[2]);
	strcat(item.name, ".zip");
	strcpy(item.previewName, argv[2]);
	strcat(item.previewName, ".png");
	item.type = STEAM_EFileType_COMMUNITY;

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
		strcmp(parser->u.object.values[2].name, "Tags") != 0 ||
		strcmp(parser->u.object.values[3].name, "Visibility") != 0	)
	{
		PARSE_ERROR("Expected Title, Description, Tags, Visibility")
	}
	if (parser->u.object.values[0].value->type != json_string)
	{
		PARSE_ERROR("Title is not a string")
	}
	if (parser->u.object.values[0].value->u.string.length > 64)
	{
		PARSE_ERROR("Title is longer than 64 characters")
	}
	if (parser->u.object.values[1].value->type != json_string)
	{
		PARSE_ERROR("Description is not a string")
	}
	if (parser->u.object.values[1].value->u.string.length > 8000)
	{
		PARSE_ERROR("Description is longer than 8000 characters")
	}
	if (parser->u.object.values[2].value->type != json_array)
	{
		PARSE_ERROR("Tags is not an array")
	}
	if (parser->u.object.values[2].value->u.array.length < 1)
	{
		PARSE_ERROR("Tags is an empty array")
	}
	if (parser->u.object.values[2].value->u.array.length > 32)
	{
		PARSE_ERROR("Tags has more than 32 elements")
	}
	for (	i = 0;
		i < parser->u.object.values[2].value->u.array.length;
		i += 1
	) {
		if (parser->u.object.values[2].value->u.array.values[i]->type != json_string)
		{
			PARSE_ERROR("Tag element is not a string")
		}
		if (parser->u.object.values[2].value->u.array.values[i]->u.string.length > 32)
		{
			PARSE_ERROR("Tag element is longer than 32 characters")
		}
	}
	if (parser->u.object.values[3].value->type != json_string)
	{
		PARSE_ERROR("Visibility is not a string")
	}
	if (	strcmp(parser->u.object.values[3].value->u.string.ptr, "Public") != 0 &&
		strcmp(parser->u.object.values[3].value->u.string.ptr, "Friends") != 0 &&
		strcmp(parser->u.object.values[3].value->u.string.ptr, "Private") != 0	)
	{
		PARSE_ERROR("Visibility: Expected Public/Friends/Private")
	}
	#undef PARSE_ERROR

	/* Interpret the JSON script */
	strcpy(
		item.title,
		parser->u.object.values[0].value->u.string.ptr
	);
	strcpy(
		item.description,
		parser->u.object.values[1].value->u.string.ptr
	);
	/* FIXME: malloc, ugh. */
	item.tags = (char**) malloc(
		sizeof(char*) *
		 parser->u.object.values[2].value->u.array.length
	);
	for (	i = 0;
		i < parser->u.object.values[2].value->u.array.length;
		i += 1
	) {
		item.tags[i] = (char*) malloc(sizeof(char) * 33);
		strcpy(
			item.tags[i],
			parser->u.object.values[2].value->u.array.values[i]->u.string.ptr
		);
	}
	item.numTags = i;
	if (strcmp(parser->u.object.values[3].value->u.string.ptr, "Public") == 0)
	{
		item.visibility = STEAM_EFileVisibility_PUBLIC;
	}
	if (strcmp(parser->u.object.values[3].value->u.string.ptr, "Friends") == 0)
	{
		item.visibility = STEAM_EFileVisibility_FRIENDSONLY;
	}
	if (strcmp(parser->u.object.values[3].value->u.string.ptr, "Private") == 0)
	{
		item.visibility = STEAM_EFileVisibility_PRIVATE;
	}

	/* Clean up. */
	json_value_free(parser);

	printf("Done!\n");
	printf("Verification complete! Beginning Workshop operation.\n\n");

	/* Command Line Operations */

	if (CHECK_STRING("upload"))
	{
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
		fileIn = fopen(item.previewName, "rb");
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
	}
	else if (CHECK_STRING("publish"))
	{
		/* Ensure all files are on Steam Cloud */
		printf("Verifying Steam Cloud entries...\n");

		/* Check for the zipfile in Steam Cloud */
		if (	STEAM_FileExists(item.name) &&
			STEAM_FileExists(item.previewName) )
		{
			printf("\t%s is in the cloud.\n", argv[2]);
		}
		else
		{
			printf(
				"\t%s is not in the cloud! Exiting.\n",
				argv[2]
			);
			goto cleanup;
		}
		printf("Verification complete! Beginning publish process.\n\n");

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
		/* Be sure all files are on Steam Workshop */
		printf("Verifying Workshop entry...\n");
		printf("Verifying Workshop ID for %s...", argv[2]);
		itemID = CMD_GetFileID(argv[2]);
		if (itemID == 0)
		{
			printf(
				"%s has no Workshop ID! Exiting.\n",
				argv[2]
			);
			goto cleanup;
		}
		printf(" Done!\n");
		printf("Verification complete! Beginning update process.\n\n");

		/* Queue each Steam Workshop update */
		printf("Queueing %s for Workshop update...", argv[2]);
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
	else if (CHECK_STRING("delete"))
	{
		/* Be sure all files are on Steam Workshop */
		printf("Verifying Workshop entry...\n");
		printf("Verifying Workshop ID for %s...", argv[2]);
		itemID = CMD_GetFileID(argv[2]);
		if (itemID == 0)
		{
			printf(
				"%s has no Workshop ID! Exiting.\n",
				argv[2]
			);
			goto cleanup;
		}
		printf(" Done!\n");
		printf("Verification complete! Beginning delete process.\n");

		/* Queue each Steam Workshop deletion */
		printf("Queueing %s for Workshop removal...", argv[2]);
		STEAM_DeletePublishedFile(itemID);
		printf("Done!\n\n");
	}

	/* Steam Asynchronous Calls */

	printf("Running Steam callbacks...");
	while (operationRunning > 0)
	{
		STEAM_Update();
		puts("...");
		PLATFORM_Sleep(UPDATE_TIME_MS);
	}
	printf("\nOperation Completed!\n");

	/* Clean up. We out. */
cleanup:
	if (!item.tags)
	{
		for (i = 0; i < item.numTags; i += 1)
		{
			free(item.tags[i]);
		}
		free(item.tags);
	}
	STEAM_Shutdown();
	return 0;

	#undef CHECK_STRING
}
