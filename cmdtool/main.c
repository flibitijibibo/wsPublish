/* wsPublish Command Line Tool
 * Written by Ethan "flibitijibibo" Lee
 */

#include <stdio.h>
#include <wsPublish/wsPublish.h>

int main(int argc, char** argv)
{
	if (!STEAM_Initialize())
	{
		printf("Steam failed to initialize!\n");
		return 0;
	}
	printf("\n\n\nTODO: A whole bunch of stuff\n\n\n");
	STEAM_Shutdown();
	return 0;
}
