/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

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

#define MAX_WORKSHOP_ITEMS	16
#define MAX_FILENAME_SIZE	32
#define UPDATE_TIME_MS		1000

/* Structure used to store Steam Workshop item information */
typedef struct CMD_WorkshopItem_s
{
	char name[MAX_FILENAME_SIZE + 4];
	char previewName[MAX_FILENAME_SIZE + 4];
	const char *title;
	const char *description;
	const char **tags;
	int numTags;
	STEAM_EFileVisibility visibility;
	STEAM_EFileType type;
} CMD_WorkshopItem_t;

/* Steam Workshop Item Information */
static CMD_WorkshopItem_t items[MAX_WORKSHOP_ITEMS];
static unsigned long itemID[MAX_WORKSHOP_ITEMS];

/* Number of Steam callbacks running, can be used to check current item index */
static int numOperations;
static int operationsRunning;

void CMD_OnSharedFile(const int success)
{
	printf(
		"Share Operation #%i: %s\n",
		numOperations - operationsRunning,
		success ? "SUCCESS" : "FAILURE"
	);
	operationsRunning -= 1;
}

void CMD_OnPublishedFile(const int success, const unsigned long fileID)
{
	FILE *fileOut;
	char builtPath[MAX_FILENAME_SIZE + 5];
	int operationNumber = numOperations - operationsRunning;

	printf("Publish Operation #%i: ", operationNumber);
	if (success)
	{
		printf("SUCCESS, FileID: %lu\n", fileID);

		strcpy(builtPath, items[operationNumber].name);
		strcat(builtPath, ".wsid");

		fileOut = fopen(builtPath, "w");
		fprintf(fileOut, "%lu", fileID);
		fclose(fileOut);
	}
	else
	{
		puts("FAILURE\n");
	}
	operationsRunning -= 1;
}

void CMD_OnUpdatedFile(const int success)
{
	printf(
		"Update Operation #%i: %s\n",
		numOperations - operationsRunning,
		success ? "SUCCESS" : "FAILURE"
	);
	operationsRunning -= 1;
}

void CMD_OnDeletedFile(const int success)
{
	printf(
		"Delete Operation #%i: %s\n",
		numOperations - operationsRunning,
		success ? "SUCCESS" : "FAILURE"
	);
	operationsRunning -= 1;
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
	#define FOREACH_ITEM for (i = 2; i < argc; i += 1)
	#define ITEM argv[i]
	#define ITEMINDEX (i - 2)

	/* JSON Filename */
	char jsonName[MAX_FILENAME_SIZE + 5];

	/* JSON Handles */
	json_value *initial;
	json_value *current;

	/* miniz Handle */
	mz_zip_archive zip;

	/* File I/O Variables */
	FILE *fileIn;
	void *fileData;
	size_t fileSize = 0;

	/* Iterator Variable */
	int i, j;

	/* Verify Command Line Arguments */

	if (	argc < 3 ||
		argc > (MAX_WORKSHOP_ITEMS + 2) ||
		!(	CHECK_STRING("upload")  ||
			CHECK_STRING("publish") ||
			CHECK_STRING("update")  ||
			CHECK_STRING("delete")	)	)
	{
		printf(
			"Usage: %s ACTION FOLDER...\n"
			"\tACTION: upload, publish, update, delete\n"
			"\tFOLDER: Folder name(s) of the Workshop item(s)\n",
			argv[0]
		);
		return 0;
	}

	/* Assign Number of Operations */
	numOperations = argc - 2;
	operationsRunning = numOperations;

	/* Initialize Steamworks */

	puts("Initializing Steam...\n");
	if (	!STEAM_Initialize(
			CMD_OnSharedFile,
			CMD_OnPublishedFile,
			CMD_OnUpdatedFile,
			CMD_OnDeletedFile
		)
	) {
		puts("Steam failed to initialize!\n");
		return 0;
	}
	puts("Steam initialized!\n\n");

	/* Verify Workshop Item Ccripts */

	puts("Verifying Workshop item JSON scripts...\n");
	FOREACH_ITEM
	{
		printf("Reading %s.json...", ITEM);

		/* Open file */
		strcpy(jsonName, ITEM);
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
		initial = json_parse(fileData, fileSize);
		current = initial;

		/* We're done with the data at this point. */
		free(fileData);

		/* Begin filling in the Workshop Item struct. */
		strcpy(items[ITEMINDEX].name, ITEM);
		strcat(items[ITEMINDEX].name, ".zip");
		strcpy(items[ITEMINDEX].previewName, ITEM);
		strcat(items[ITEMINDEX].previewName, ".png");
		items[ITEMINDEX].type = STEAM_EFileType_COMMUNITY;

		/* Verify the JSON script. */
		#define PARSE_ERROR(output) \
			printf(" ERROR: %s! Exiting.\n", output); \
			json_value_free(initial); \
			goto cleanup;
		if (current->type != json_object)
		{
			PARSE_ERROR("Expected Item JSON Object")
		}
		if (current->u.object.length != 3)
		{
			PARSE_ERROR("Expected only 3 values")
		}
		if (	!strcmp(current->u.object.values[0].name, "Title") ||
			!strcmp(current->u.object.values[1].name, "Description") ||
			!strcmp(current->u.object.values[2].name, "Tags")	)
		{
			PARSE_ERROR("Expected Title, Description, Tags")
		}
		if (current->u.object.values[0].value->type != json_string)
		{
			PARSE_ERROR("Title is not a string")
		}
		if (current->u.object.values[1].value->type != json_string)
		{
			PARSE_ERROR("Description is not a string")
		}
		if (current->u.object.values[2].value->type != json_array)
		{
			PARSE_ERROR("Tags is not an array")
		}
		if (current->u.object.values[2].value->u.array.length < 1)
		{
			PARSE_ERROR("Tags is an empty array")
		}
		for (	j = 0;
			j < current->u.object.values[2].value->u.array.length;
			j += 1
		) {
			if (current->u.object.values[2].value->u.array.values[j]->type != json_string)
			{
				PARSE_ERROR("Tag element is not a string")
			}
		}
		#undef PARSE_ERROR

		/* TODO Interpret the JSON script */

		/* Clean up. NEXT. */
		json_value_free(initial);

		puts("Done!\n");
	}
	puts("Verification complete! Beginning Workshop operation.\n\n");

	/* Command Line Operations */

	if (CHECK_STRING("upload"))
	{
		FOREACH_ITEM
		{
			/* Create the zipfile */
			printf("Zipping %s folder to heap...", ITEM);
			mz_zip_writer_init_heap(&zip, 0, 0);
			PLATFORM_EnumerateFiles(
				argv[0],
				ITEM,
				&zip,
				CMD_OnFileEnumerated
			);
			mz_zip_writer_finalize_heap_archive(
				&zip,
				&fileData,
				&fileSize
			);
			puts(" Done!\n");

			/* Write to Steam Cloud */
			printf("Writing %s to the cloud...", ITEM);
			if (	!STEAM_WriteFile(
					items[ITEMINDEX].name,
					fileData,
					fileSize
				)
			) {
				puts(" Cloud write failed! Exiting.\n");
				mz_zip_writer_end(&zip);
				goto cleanup;
			}
			mz_zip_writer_end(&zip);
			puts(" Done!\n");

			/* Read the PNG file into memory. Ugh. */
			fileIn = fopen(items[ITEMINDEX].previewName, "rb");
			fseek(fileIn, 0, SEEK_END);
			fileSize = ftell(fileIn);
			rewind(fileIn);
			fileData = malloc(sizeof(char) * fileSize);
			fread(fileData, 1, fileSize, fileIn);

			/* Write the PNG file to Steam Cloud */
			printf("Writing %s preview image to cloud...", ITEM);
			if (	!STEAM_WriteFile(
					items[ITEMINDEX].previewName,
					fileData,
					fileSize
				)
			) {
				puts(" Cloud write failed! Exiting.\n");
				free(fileData);
				goto cleanup;
			}
			free(fileData);
			puts(" Done!\n");

			/* Mark Steam Cloud files as shared */
			printf("Queueing %s for Steam file share...", ITEM);
			STEAM_ShareFile(items[ITEMINDEX].name);
			STEAM_ShareFile(items[ITEMINDEX].previewName);
			puts(" Done!\n\n");

			/* Two operations per item */
			numOperations *= 2;
			operationsRunning = numOperations;
		}
	}
	else if (CHECK_STRING("publish"))
	{
		/* Ensure all files are on Steam Cloud */
		puts("Verifying Steam Cloud entries...\n");
		FOREACH_ITEM
		{
			/* Check for the zipfile in Steam Cloud */
			if (	STEAM_FileExists(items[ITEMINDEX].name) &&
				STEAM_FileExists(items[ITEMINDEX].previewName) )
			{
				printf("\t%s is in the cloud.\n", ITEM);
			}
			else
			{
				printf(
					"\t%s is not in the cloud! Exiting.\n",
					ITEM
				);
				goto cleanup;
			}
		}
		puts("Verification complete! Beginning publish process.\n\n");

		/* Publish all files on Steam Workshop */
		FOREACH_ITEM
		{
			printf("Queueing %s for Workshop publication...", ITEM);
			STEAM_PublishFile(
				STEAM_GetAppID(),
				items[ITEMINDEX].name,
				items[ITEMINDEX].previewName,
				items[ITEMINDEX].title,
				items[ITEMINDEX].description,
				items[ITEMINDEX].tags,
				items[ITEMINDEX].numTags,
				items[ITEMINDEX].visibility,
				items[ITEMINDEX].type
			);
			puts(" Done!\n");
		}
		puts("\n");
	}
	else if (CHECK_STRING("update"))
	{
		/* Be sure all files are on Steam Workshop */
		puts("Verifying Workshop entries...\n");
		FOREACH_ITEM
		{
			printf("Verifying Workshop ID for %s...", ITEM);
			itemID[ITEMINDEX] = CMD_GetFileID(ITEM);
			if (itemID[ITEMINDEX] == 0)
			{
				printf(
					"%s has no Workshop ID! Exiting.\n",
					ITEM
				);
				goto cleanup;
			}
			puts(" Done!\n");
		}
		puts("Verification complete! Beginning update process.\n\n");

		/* Queue each Steam Workshop update */
		FOREACH_ITEM
		{
			printf("Queueing %s for Workshop update...", ITEM);
			STEAM_UpdatePublishedFile(
				itemID[ITEMINDEX],
				items[ITEMINDEX].name,
				items[ITEMINDEX].previewName,
				items[ITEMINDEX].title,
				items[ITEMINDEX].description,
				items[ITEMINDEX].tags,
				items[ITEMINDEX].numTags,
				items[ITEMINDEX].visibility
			);
			puts(" Done!\n");
		}
		puts("\n");
	}
	else if (CHECK_STRING("delete"))
	{
		/* Be sure all files are on Steam Workshop */
		puts("Verifying Workshop entries...\n");
		FOREACH_ITEM
		{
			printf("Verifying Workshop ID for %s...", ITEM);
			itemID[ITEMINDEX] = CMD_GetFileID(ITEM);
			if (itemID[ITEMINDEX] == 0)
			{
				printf(
					"%s has no Workshop ID! Exiting.\n",
					ITEM
				);
				goto cleanup;
			}
			puts(" Done!\n");
		}
		puts("Verification complete! Beginning delete process.\n");

		/* Queue each Steam Workshop deletion */
		FOREACH_ITEM
		{
			printf("Queueing %s for Workshop removal...", ITEM);
			STEAM_DeletePublishedFile(itemID[ITEMINDEX]);
			puts("Done!\n");
		}
		puts("\n");
	}

	/* Steam Asynchronous Calls */

	puts("Running Steam callbacks...\n");
	STEAM_Update();

	/* Wait for all operations to complete. */

	while (operationsRunning > 0)
	{
		puts(".");
		PLATFORM_Sleep(UPDATE_TIME_MS);
	}
	puts("\nOperation Completed!\n");

	/* Clean up. We out. */
cleanup:
	STEAM_Shutdown();
	return 0;

	#undef CHECK_STRING
	#undef FOREACH_ITEM
	#undef ITEM
	#undef ITEMINDEX
}
