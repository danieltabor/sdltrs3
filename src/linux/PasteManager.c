#include <SDL3/SDL.h>
#include <stdlib.h>

char* paste_buffer = 0;
int paste_buffer_len = 0;
int paste_buffer_offset = 0;

int PasteManagerGetChar(unsigned short *character)
{
	if( paste_buffer_offset < paste_buffer_len ) {
		*character = paste_buffer[paste_buffer_offset++];
		return 1;
	}
	else {
		if( paste_buffer ) {
			free(paste_buffer);
			paste_buffer = 0;
		}
		paste_buffer_len = 0;
		paste_buffer_offset = 0;
		return 0;
	}
}

int PasteManagerStartPaste(void)
{ 
	paste_buffer = strdup(SDL_GetClipboardText());
	if( paste_buffer ) {
		paste_buffer_len = strlen(paste_buffer);
	}
	else {
		paste_buffer_len = 0;
	}
	paste_buffer_offset = 0;
}

void PasteManagerStartCopy(char *string)
{
	SDL_SetClipboardText(string);
}
