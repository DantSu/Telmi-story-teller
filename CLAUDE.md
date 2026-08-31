# Telmi OS — Boîte à histoires & lecteur MP3 pour Miyoo Mini Plus

## Contraintes absolues

- **NE JAMAIS compiler.** L'utilisateur compile lui-même. Ne jamais lancer `make`, `gcc`, `g++` ou tout outil de compilation.
- **NE JAMAIS aller dans `/design/`**. Ce répertoire n'est pas utile.
- **`/build`, `/cache`, `/release`** sont des répertoires temporaires, supprimés à chaque build. Ne pas y chercher de code source.
- Le code source réel est dans **`/src`**, **`/Makefile`**, et **`/static/build/.tmp_update/runtime.sh`**.
- Le code le plus important est dans **`/src/storyTeller`**.

## Architecture générale

Le projet est écrit en **C** (C18, `gnu18`) pour **Miyoo Mini Plus** (ARM Cortex-A7, cross-compilé avec un toolchain Docker `aemiii91/miyoomini-toolchain:latest`).

### Binaire principal : `storyTeller`

Fichier d'entrée : `src/storyTeller/storyTeller.c`

Boucle principale :
1. `display_init()` → `video_audio_init()` → `settings_init()` → `parameters_init()` → `app_init()`
2. Boucle infinie : `poll()` sur `/dev/input/event0` → gestion des boutons → `app_update()`
3. Sortie : `app_save()` → `display_setScreen(true)` → `video_audio_quit()` → `system_shutdown()`

### Module `app_selector` (routeur d'applications)

`src/storyTeller/app_selector.h` — 3 apps possibles :
| Index | Constante | App |
|-------|-----------|-----|
| 0 | `APP_STORIES` | Histoires (lecture de stories STUdio) |
| 1 | `APP_MUSIC` | Lecteur MP3 |
| 2 | `APP_NIGHTMODE` | Mode nuit (playlist d'histoires) |

Routes d'appels : `app_update()`, `app_menu()`, `app_previous()`, `app_next()`, `app_up()`, `app_down()`, `app_pause()`, `app_ok()`, `app_home()`, `app_save()`, `app_init()`

### Module Histoires : `stories_reader.h`

Cœur du projet. Lecture d'histoires exportées depuis Telmi Sync.

**Chemin sur la SD** : `/mnt/SDCARD/Stories/<story_name>/`
- `nodes.json` — structure de la story (stages, actions, inventory, transitions)
- `audios/` — fichiers MP3
- `images/` — images PNG
- `title.png` — vignette
- `title.mp3` — son de titre

**Format `nodes.json` et structure de dossier** : voir **[TELMI_STORY.md](./TELMI_STORY.md)** pour la documentation complète du format (metadata.json, nodes.json, notes.json, audios/, images/).

**Modes d'affichage** : `STORIES_DISPLAY_MODE_SINGLE` (0) ou `STORIES_DISPLAY_MODE_TILES` (1, grille 3x3)

**Features** :
- Inventaire avec conditions (`stories_inventory_*`)
- Mode nuit : playlist de 16 histoires audio jouées en boucle, écran éteint
- Sauvegarde de session : `/mnt/SDCARD/Saves/.storytellerState`
- Sauvegarde par histoire : `/mnt/SDCARD/Saves/Stories/<nomHistoire>`
- Timeline (barre de progression) avec pause/play, rewind ±10s

**Callbacks** :
- `callback_stories_autoplay` → `stories_autoplay()`
- `callback_stories_reset` → `stories_reset()`
- `callback_stories_nightMode` → `stories_nightMode_next()`
- `callback_stories_audio_hook` → appelé quand l'audio se termine

### Module Musique : `music_player.h`

Lecteur MP3 avec mode "player" et mode "albums".

**Chemin sur la SD** : `/mnt/SDCARD/Music/`
- Fichiers MP3 nommés : `Artiste_Album_Track_Titre.mp3`
- Images de pochettes : `Artiste_Album_Track_Titre.png`

**Repeat modes** : `REPEAT_ALL`(0), `REPEAT_SHUFFLE`(1), `REPEAT_ALBUM`(2), `REPEAT_TITLE`(3)

### Module SDL : `sdl_helper.h`

- SDL2 : fenêtre 640x480, OpenGL ES 2.0
- SDL_mixer : audio MP3 (44100Hz)
- SDL_image : chargement PNG
- SDL_ttf : polices Exo2 (Regular, Bold) en 16/18/20/24
- SDL_gfx : `rotozoomSurface`
- **Cache surfaces** : LRU de 16 surfaces (clé = path|width)
- Thread dédié : calcul de durée MP3 en arrière-plan
- `video_applyToVideo()` : composite appSurface → screen → texture → renderer → present
  - Affiche la batterie, le lock, les barres volume/luminosité

### Modules utilitaires

| Fichier | Rôle |
|---------|------|
| `app_parameters.h` | Params persistants JSON `/mnt/SDCARD/Saves/.parameters` (volume, luminosité, temps inactivité, features) |
| `app_autosleep.h` | Extinction auto écran (temps configurable) |
| `app_lock.h` | Verrouillage écran (long-press bouton menu ~1s) |
| `app_volume.h` | Contrôles volume (affichés 2s) |
| `app_brightness.h` | Contrôles luminosité (affichés 2s) |
| `app_battery.h` | Pourcentage batterie (cache 10s) |
| `app_file.h` | `file_save(path, format, ...)` — écriture JSON |
| `time_helper.h` | `get_time()`, `time_wait300ms()` (anti-rebond boutons) |
| `logs_helper.h` | `writeLog(title, message)` |
| `array_helper.h` | `sort(char** arr, int n)` — tri alphabétique |

### Chemins système sur la SD

| Chemin | Contenu |
|--------|---------|
| `/mnt/SDCARD/.tmp_update/res/` | Ressources système (images, polices) |
| `/mnt/SDCARD/Stories/` | Histoires |
| `/mnt/SDCARD/Music/` | Fichiers MP3 |
| `/mnt/SDCARD/Saves/` | Sauvegardes et paramètres |
| `/mnt/SDCARD/Saves/.parameters` | Paramètres JSON |
| `/mnt/SDCARD/Saves/.storytellerState` | Session en cours |
| `/mnt/SDCARD/.tmp_update/config/` | Configs système |

### Autres composants (non prioritaires)

| Dossier | Binaire | Rôle |
|---------|---------|------|
| `src/axp/` | axp | Gestionnaire d'alimentation |
| `src/bootScreen/` | bootScreen | Écran de démarrage |
| `src/chargingState/` | chargingState | Écran de charge |
| `src/batmon/` | batmon | Daemon surveillance batterie |

### Code commun partagé

- `src/common/system/` — wrappers système (display, battery, settings, keymap, axp, etc.)
- `src/common/utils/` — utilitaires (str, json, file, log)
- `src/common/config.mk`, `src/common/commands.mk`, `src/common/recipes.mk` — Makefiles communs
- `include/` — headers 3rd party (SDL2, cJSON, gfx, shmvar, png)
- `lib/` — shared libraries pré-compilées

## Conventions de code

- **C18** (`-std=gnu18`)
- Headers uniquement (`.h`) pour les modules storyTeller — tout est `static` + `#ifndef` guards
- Pas de séparation .h/.c dans storyTeller : chaque module est un header auto-contenu
- `STR_MAX` = taille de buffer standard (défini dans `utils/str.h`)
- `STR_DIRNAME` = 128 (noms de fichiers)
- Les images sont des PNG, les audio des MP3
- Cross-compilation ARM : `-marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve`
- `DISPLAY_WIDTH` = 640, `DISPLAY_HEIGHT` = 480

## Build system

- **Makefile** racine : orchestration
- Toolchain : Docker `aemiii91/miyoomini-toolchain:latest`
- Commande build : `make build` (ou `make all` / `make dist`)
- Cible `clean` : supprime `/build`, `/build_test`, les `.o`
- Cible `release` : génère zips de distribution

## Mapping boutons (Miyoo Mini Plus)

| Bouton       | Code                           | Action (contexte normal)                           |
|--------------|--------------------------------|----------------------------------------------------|
| MENU         | `HW_BTN_MENU`                  | Menu (change le mode d'affichage stories / albums) |
| POWER        | `HW_BTN_POWER`                 | Arrêt (long-press 1s)                              |
| LEFT         | `HW_BTN_LEFT`                  | Précédent                                          |
| RIGHT        | `HW_BTN_RIGHT`                 | Suivant                                            |
| UP           | `HW_BTN_UP`                    | Haut                                               |
| DOWN         | `HW_BTN_DOWN`                  | Bas                                                |
| START/SELECT | `HW_BTN_START`/`HW_BTN_SELECT` | Pause/Play                                         |
| A/B          | `HW_BTN_A`/`HW_BTN_B`          | OK (valider)                                       |
| Y/X          | `HW_BTN_Y`/`HW_BTN_X`          | Home (retour)                                      |
| L2           | `HW_BTN_L2`                    | Luminosité - (si MENU maintenu)                    |
| R2           | `HW_BTN_R2`                    | Luminosité + (si MENU maintenu)                    |
| VOL-         | `HW_BTN_VOLUME_DOWN`           | Luminosité - (si MENU maintenu) / Volume -         |
| VOL+         | `HW_BTN_VOLUME_UP`             | Luminosité + (si MENU maintenu) / Volume +         |

**MENU long-press (~1s)** : verrouille/déverrouille l'écran.

## Paramètres par défaut (`app_parameters.h`)

| Paramètre | Défaut | Description |
|-----------|--------|-------------|
| `audioVolumeStartup` | 0.3 | Volume au démarrage (fraction du max système) |
| `audioVolumeMax` | 0.6 | Max volume utilisateur |
| `systemAudioVolumeMax` | 25.0 | Max système |
| `screenBrightnessStartup` | 0.3 | Luminosité au démarrage |
| `screenBrightnessMax` | 0.6 | Max luminosité utilisateur |
| `systemScreenBrightnessMax` | 10.0 | Max système |
| `screenOnInactivityTime` | 120s | Extinction écran (allumé) |
| `screenOffInactivityTime` | 300s | Extinction écran (éteint) |
| `musicInactivityTime` | 3600s | Inactivité musique |
| `storyDisplayTiles` | false | Mode grille 3x3 |
| `storyDisableNightMode` | false | Désactiver mode nuit |
| `storyDisableTimeline` | false | Désactiver timeline |
| `musicDisableRepeatModes` | false | Désactiver modes repeat |

## Polices et couleurs

| Nom | Police | Taille |
|-----|--------|--------|
| `fontBold24` | Exo2-Bold | 24px |
| `fontBold20` | Exo2-Bold | 20px |
| `fontBold18` | Exo2-Bold | 18px |
| `fontRegular20` | Exo2-Regular | 20px |
| `fontRegular18` | Exo2-Regular | 18px |
| `fontRegular16` | Exo2-Regular | 16px |

| Couleur | RGB |
|---------|-----|
| `colorWhite` | (255, 255, 255) |
| `colorWhite60` | (189, 186, 193) |
| `colorPurple` | (37, 16, 58) |
| `colorOrange` | (255, 181, 0) |
| `colorRed` | (238, 45, 0) |
