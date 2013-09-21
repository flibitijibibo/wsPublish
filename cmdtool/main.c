/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#include <stdio.h>
#include <wsPublish/wsPublish.h>

void CMD_OnSharedFile(const int success)
{
	// TODO
}

void CMD_OnPublishedFile(const int success, const unsigned long fileID)
{
	// TODO:
}

void CMD_OnUpdatedFile(const int success)
{
	// TODO
}

void CMD_OnDeletedFile(const int success)
{
	// TODO
}

int main(int argc, char** argv)
{
	if (	!STEAM_Initialize(
			CMD_OnSharedFile,
			CMD_OnPublishedFile,
			CMD_OnUpdatedFile,
			CMD_OnDeletedFile
		)
	) {
		printf("Steam failed to initialize!\n");
		return 0;
	}
	printf("\n\n\nTODO: A whole bunch of stuff\n\n\n");
	STEAM_Shutdown();
	return 0;
}
