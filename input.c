#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "input.h"

/* THE BIG LESSON: scanf("%d") breaks if the player types a letter, and
   can loop forever. Reading a WHOLE LINE with fgets and converting it
   ourselves, checking the conversion worked, is always safe. */

/* Remove spaces from both ends of a string, in place. */
static void trim(char *text)
{
    char *start = text;

    while (isspace((unsigned char)*start))
        start++;

    size_t length = strlen(start);

    while (length > 0 && isspace((unsigned char)start[length - 1]))
        length--;

    memmove(text, start, length);
    text[length] = '\0';
}

/* Read one raw line, trimmed. Blank lines are returned as "". */
void readOptional(char *buffer, int size)
{
    fflush(stdout);             /* make sure the prompt is on screen first */

    if (fgets(buffer, size, stdin) == NULL)
    {
        /* Input closed (Ctrl+D / Ctrl+Z, or a piped file ran out) */
        printf("\n\nInput ended. Goodbye!\n");
        exit(0);
    }

    /* If the line was longer than the buffer, throw away the rest */
    if (strchr(buffer, '\n') == NULL)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    trim(buffer);
}

void readLine(char *buffer, int size)
{
    do
    {
        readOptional(buffer, size);
    }
    while (buffer[0] == '\0');
}

int readInt(void)
{
    char line[64];
    char *end;

    readLine(line, sizeof line);

    long value = strtol(line, &end, 10);

    if (end == line || *end != '\0')
        return -1;

    return (int)value;
}
