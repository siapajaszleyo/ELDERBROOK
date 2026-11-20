#ifndef HUB_H
#define HUB_H

/* What the town returns to the main loop */
#define HUB_QUIT    0       /* the player saved and quit               */
#define HUB_FINAL  16       /* the player chose to face the final boss */
#define HUB_ENDLESS 17     /* the player entered the endless descent  */
/* 1..15 = the player chose to travel to that location */

/* The town of Elderbrook. Loops until the player travels somewhere or
   quits, and returns one of the values above. */
int hubVisit(void);

#endif
