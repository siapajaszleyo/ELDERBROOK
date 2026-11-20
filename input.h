#ifndef INPUT_H
#define INPUT_H

/* Read one non-empty line of text (blank lines are skipped).
   Exits the program cleanly if the input ends. */
void readLine(char *buffer, int size);

/* Read one line, but an empty line is allowed (returns ""). */
void readOptional(char *buffer, int size);

/* Read a whole number. Returns -1 if the player typed anything else. */
int readInt(void);

#endif
