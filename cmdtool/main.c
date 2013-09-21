/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#include <stdio.h>
#include <string.h>
#include <wsPublish/wsPublish.h>

#ifdef _WIN32
#include <windows.h>
#define sleepms(x) Sleep(x)
#else
#include <unistd.h>
#define sleepms(x) usleep((x) * 1000)
#endif

#define MAX_WORKSHOP_ITEMS	16
#define MAX_FILENAME_SIZE	32
#define UPDATE_TIME_MS		1000

typedef struct CMD_WorkshopItem_s
{
	const char *name;
	const char *previewName;
	const char *title;
	const char *description;
	const char **tags;
	int numTags;
	STEAM_EFileVisibility visibility;
	STEAM_EFileType type;
} CMD_WorkshopItem_t;

static int operationsRunning;

void CMD_OnSharedFile(const int success)
{
	/* TODO */
	operationsRunning -= 1;
}

void CMD_OnPublishedFile(const int success, const unsigned long fileID)
{
	/* TODO */
	operationsRunning -= 1;
}

void CMD_OnUpdatedFile(const int success)
{
	/* TODO */
	operationsRunning -= 1;
}

void CMD_OnDeletedFile(const int success)
{
	/* TODO */
	operationsRunning -= 1;
}

int main(int argc, char** argv)
{
	#define CHECK_STRING(string) (strcmp(argv[1], string) == 0)
	#define FOREACH_ITEM for (i = 2; i < argc; i += 1)
	#define ITEM argv[i]
	#define ITEMINDEX (i - 2)

	/* Workshop Entries */
	CMD_WorkshopItem_t items[MAX_WORKSHOP_ITEMS];
	unsigned long itemID[MAX_WORKSHOP_ITEMS];

	/* Used to obtain zipfile data, push to Steam Cloud */
	unsigned char *data = NULL;

	/* Used to build zipfile name */
	char builtPath[MAX_FILENAME_SIZE];

	/* File size of the zipfile */
	int builtLength = 0;

	/* Iterator Variable */
	int i;

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
	operationsRunning = argc - 2;

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
		/* TODO: JSON CHECK */
		puts("Done!\n");
	}
	puts("Verification complete! Beginning Workshop operation.\n\n");

	/* Command Line Operations */

	if (CHECK_STRING("upload"))
	{
		FOREACH_ITEM
		{
			/* Create the zipfile name */
			strcpy(builtPath, ITEM);
			strcat(builtPath, ".zip");

			/* Create the zipfile */
			printf("Creating zip file of %s...", ITEM);
			/* TODO: ZIP FOLDER */
			puts(" Done!\n");

			/* Write to Steam Cloud */
			printf("Writing %s to the cloud...", ITEM);
			if (!STEAM_WriteFile(builtPath, data, builtLength))
			{
				puts(" Cloud write failed! Exiting.\n");
				goto cleanup;
			}
			puts(" Done!\n");

			/* Mark Steam Cloud file as shared */
			printf("Queueing %s for Steam file share...", ITEM);
			STEAM_ShareFile(builtPath);
			puts(" Done!\n\n");
		}
	}
	else if (CHECK_STRING("publish"))
	{
		/* Ensure all files are on Steam Cloud */
		puts("Verifying Steam Cloud entries...\n");
		FOREACH_ITEM
		{
			/* Create the zipfile name */
			strcpy(builtPath, ITEM);
			strcat(builtPath, ".zip");

			/* Check for the zipfile in Steam Cloud */
			if (STEAM_FileExists(builtPath))
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
				0, /* TODO: Get AppID */
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
			/* TODO: Get fileID */
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
			/* TODO: Get fileID */
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
		sleepms(UPDATE_TIME_MS);
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
