SysHack
=======

[![v0.1](https://img.shields.io/badge/dev-v0.1-brightgreen.svg)](../../tree/v0.1)

System Shock 2 in NCurses.

## Build

```sh
syshack $ cd build
build $ cmake -DCMAKE_BUILD_TYPE=Debug ..
buiid $ make
buiid $ bin/syshack
```

`compile_commands.json` for use with LSP can be retrieved this way:

```sh
build $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
build $ mv compile_commands.json ../
```

## Keys

|                                                        |                               |
| ---                                                    | ---                           |
| <kbd>h</kbd>, <kbd>j</kbd>, <kbd>k</kbd>, <kbd>l</kbd> | Move WEST, SOUTH, NORTH, EAST |
| <kbd>y</kbd>, <kbd>u</kbd>, <kbd>b</kbd>, <kbd>n</kbd> | Move NW, NE, SW, SE           |
| <kbd>a</kbd>+`<dir>`                                   | Attack                        |
| <kbd>q</kbd>                                           | Use cowardice                 |

## TODO

✔ Walking character  
✔ Walking droid  
✔ Ebites!
[![v0.1](https://img.shields.io/badge/-v0.1-brightgreen.svg)](../../tree/v0.1)  
・ Level generation  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\- rooms  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\- connect rooms  
・ Collision detection  
・ NPC interactions
[![v0.2](https://img.shields.io/badge/-v0.2-lightgray.svg)](../../tree/v0.2)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\- dialogs  
・ Field of view  
・ Difficulty levels
