# Telmi Story

## Telmi Story Format

### Folder Structure

```
/metadata.json
/nodes.json
/notes.json (Optional)
/title.mp3
/title.png
/cover.png (Optional)
/audios/s0.mp3
/audios/s1.mp3
/audios/s2.mp3
...
/images/s1.png
/images/s3.png
...
```

### File: /metadata.json

JSON file containing the story metadata such as title, description, age, etc.

```json
{
  "title": "The title of the story", // Text
  "uuid": "ffffff-199f066d199", // Text, unique identifier of the story
  "image": "title.png", // Enum, possible values: [ title.png, cover.png ]
  "version": 2, // Integer, story version (optional property)
  "category": "The category in which it is classified", // Text (optional property)
  "description": "The description of my story.", // Text (optional property)
  "age": "5" // Integer, recommended age for the story (optional property)
}
```

### File: /nodes.json

JSON file containing all the nodes that define the behavior of the game.

The game starts with 'startAction'. Then, an action leads to one or more scenes.  
If multiple scenes are in the action and the currently playing scene does not have `control.autoplay` set to true, the user can switch between scenes in the current action.  
If `control.autoplay` of the current scene is true, the left/right buttons are disabled, and the next action will be triggered at the end of the current audio playback or when pressing the A, B, X, or Y buttons according to the values of `control.home` and `control.ok`.

```json
{
  "startAction": { // Contains the first action executed when the game starts
    "action": "a0", // Text, action key
    "index": 0 // Integer, index of the scene in the action selected by default, if `index` equals -1 then the scene is randomly selected
  },
  "inventory": [ // If there are no inventory items, the `inventory` property must be omitted (optional property)
    {
      "name": "My Item 1", // Text, item name
      "initialNumber": 0, // Integer, number of items at the start of the game
      "maxNumber": 3, // Integer, maximum number of items the player can obtain
      "display": 0, // Enum (integer), possible values: [ 0 (display image and item count), 1 (display image and item gauge), 2 (item not displayed) ]
      "image": "i0.png" // Text, PNG image filename (128x128px) of the item located in /images
    },
    ...
  ],
  "stages": { // Object containing all story scenes
    "s1": { // A scene contains the image and audio played during its display and the behavior executed at the end of the audio or when the player presses A, B, X, or Y.
      "image": "s1.png", // Text or null, PNG image filename (640x480px) located in /images
      "audio": "s1.mp3", // Text or null, MP3 audio filename located in /audios
      "ok": { // Action executed when the player presses A or B if `control.ok` is true, or when the audio ends if `control.autoplay` is true
        // Note: "index" and "indexItem" are mutually exclusive — only one of them can be present at a time.
        "action": "a6", // Text, action key
        "index": 0, // Integer, index of the scene in the selected action, if `index` equals -1 then the scene is randomly selected
        "indexItem": 0 // Integer, index of the inventory item whose quantity will be used to select the next scene in the selected action
      },
      "home": { // Action executed when the player presses X or Y if `control.home` is true
        // Note: "index" and "indexItem" are mutually exclusive — only one of them can be present at a time.
        "action": "a3", // Text, action key
        "index": 0, // Integer, index of the scene in the selected action, if `index` equals -1 then the scene is randomly selected
        "indexItem": 0 // Integer, index of the inventory item whose quantity will be used to select the next scene in the selected action
      },
      "control": { // Object containing behavior permissions
        "ok": true, // Boolean, if true the action in ok will be executed when the player presses A or B, if false nothing happens
        "home": true, // Boolean, if true the action in home will be executed when the player presses X or Y, if false nothing happens
        "autoplay": false // Boolean, if true the action in ok will be executed at the end of the MP3 playback defined in audio, if audio is null then the ok action is executed immediately
      },
      "items": [ // Contains updates to the player's inventory. If no updates are needed, the items property must be omitted (optional property)
        {
          "type": 0, // Enum integer, assignment operator (equal, add, subtract, multiply...), possible values: [ 0:'+=' (adds `number` items), 1:'-=' (removes `number` items), 2:'=' (sets inventory to `number` items), 3:'*=' (multiplies by `number`), 4:'/=' (divides by `number`), 5:'%=' (sets inventory to modulo `number`) ]
          "item": 0, // Integer, index of the inventory item affected
          "number": 2 // Integer, value to assign
        },
        {
          "type": 2, // Same as above
          "item": 1, // Integer, index of the affected inventory item
          "assignItem": 0 // Integer, index of the inventory item whose value is assigned to `item`
        },
        {
          "type": 3, // Same as above
          "item": 2, // Integer, index of the affected inventory item
          "playingTime": true // If defined and true, the assigned value will be the elapsed playtime in seconds
        },
        ...
      ],
      "inventoryReset": true // If the `inventoryReset` property exists and is set to true, all items in the inventory are reset to their default values (optional property, omit if not used)
    },
    ...
  },
  "actions": { // Lists all game actions. An action is a list of scenes that can contain display conditions
    "a1": [ // Action containing a list of scenes to play
      {
        "stage": "s2", // Text, key of the scene to play
        "conditions": [ // If there are no display conditions, the `conditions` property must be omitted (optional property)
          {
            "comparator": 2, // Enum integer, comparison operator, possible values: [ 0:'<', 1:'<=', 2:'==', 3:'>', 4:'>=', 5:'!=' ]
            "item": 0, // Integer, index of the inventory item whose value is used on the left side of the comparison
            "number": 2 // Integer, value on the right side of the comparison
          },
          {
            "comparator": 1, // Enum integer, comparison operator, possible values: [ 0:'<', 1:'<=', 2:'==', 3:'>', 4:'>=', 5:'!=' ]
            "item": 0, // Integer, index of the inventory item whose value is used on the left side of the comparison
            "compareItem": 1 // Integer, index of the inventory item whose value is used on the right side of the comparison
          },
          ...
        ]
      },
      ...
    ],
    ...
  }
}
```

### File: /notes.json (Optional file)

JSON file containing the title, text, and color of each story scene.  
This file is only used by the Telmi Sync Studio and does not affect the game in Telmi OS.

```json
{
  "Scene key": {
    "title": "Scene title", // Text, title displayed on scenes in the Telmi Sync diagram
    "notes": "Scene text", // Text, contains the story and can optionally be read by Telmi Sync TTS upon request
    "color": "red2" // Enum (optional property), scene color in Telmi Sync diagram, possible values: [ pink, pink2, purple3, purple4, purple5, yellow, orange2, orange3, red, red2, brown, green, green2, green3, green4, blue, blue2, blue3, blue4 ]
  },
  ...
}
```

### File: /title.png

PNG image used as the story title in Telmi OS.

### File: /title.mp3

Audio file containing the story title in MP3 format, 44100 Hz, constant bitrate between 64 and 192 kbps.

### File: /cover.png (Optional file)

PNG cover image used in Telmi Sync.

### Folder: /audios/

Contains scene audio files in MP3 format, 44100 Hz, constant bitrate between 64 and 192 kbps.

### Folder: /images/

Contains PNG image files for scenes and inventory items.
