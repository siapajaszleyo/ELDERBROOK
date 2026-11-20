# Elderbrook — Epic RPG Adventure

A text-adventure RPG, written entirely in C. You choose a class and
discipline, travel the road, clear dungeons, play minigames, and face the
final boss — or you do not. There is no forced victory, and no world that
suddenly becomes peaceful: where power is not taken, a vacuum remains.

## Build

    make          # builds ./rpg  (classic build)
    make tui      # builds ./rpg-tui (ASCII/TUI build)

## Run

    ./rpg              # fresh game, random seed
    ./rpg 42           # same seed = same run (reproducible)
    ./rpg-tui          # the terminal UI build

## Save

The game saves automatically at every checkpoint. A save is plain text
(`savegame.sav`, with `savegame.bak` as the previous-state backup). The
format carries a version header and loads older versions too, so old saves
still work after content is added. Keep your saves backed up — they are not
stored in the repository.

## Structure

- `rpg.c`          entry, the town/expedition loop, class screen, finale
- `main loop`      `startGame()` alternates between town and expedition
- `world.c`        the 15 locations, the trip planner, the event deck
- `combat.c`       turn-based fight engine; damage from `balance.h`
- `events.c`       the event runner (every episode inside an expedition)
- `hub.c`          town: shop, talk, train, plan a trip
- `save.c`         the field table that drives all of saving/loading
- `ascii.c`, `tui.c`, `ui.c`  presentation only — the display wall
- `story.c`, `storybeats.c`  the through-line; additive, never rewrites
- `guild.c`        factions, guild ranks, and the final battle's muster
- `minigames/`     chess, snake, dice, blackjack, memory, reflex, archery, lockpick
- `balance.h`      the single file for all tuning numbers

## Adding content

Per `BACKEND_HANDOFF.txt`: append to enums and the save field table, keep
the save contract stable, and never reorder existing indices. New content
must not break old saves.

## Notes for maintainers

This repository holds the C source and the build rule. Your saved games
(`savegame.sav` / `.bak`) are git-ignored and are not shipped to GitHub.
The full handoff and lore documents live separately under `Deliverables/`
(outside the archive), including the engine handoff and the lore canon.
