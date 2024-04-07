SysHack
=======

[![v0.2](https://img.shields.io/badge/dev-v0.2-brightgreen.svg)](../../tree/v0.2)

Something like System Shock, but in NCurses.

## Build

```sh
$ ./build.sh
$ build/bin/syshack
```

`build.sh` also generates `compile_commands.json` for use with
[LSP](https://github.com/microsoft/language-server-protocol).

## Keys

|                                                        |                               |
| ---                                                    | ---                           |
| <kbd>h</kbd>, <kbd>j</kbd>, <kbd>k</kbd>, <kbd>l</kbd> | Move WEST, SOUTH, NORTH, EAST |
| <kbd>y</kbd>, <kbd>u</kbd>, <kbd>b</kbd>, <kbd>n</kbd> | Move NW, NE, SW, SE           |
| <kbd>a</kbd>+`<dir>`                                   | Attack                        |
| <kbd>q</kbd>                                           | Use cowardice                 |

## TODO

- [x] Walking character
- [x] Walking droid
- [x] Ebites!
- [x] Collision detection
- [ ] Level generation
  - ~rooms merging~
  - ~connect rooms with doors~
  - ~shift & connect more~
  - stairs
    ![v0.3](https://img.shields.io/badge/-v0.3-lightgray.svg)
- [ ] Rendering
  - ~text~
  - several objects per tile
- [ ] NPC interactions
  - dialogs
    ![v0.4](https://img.shields.io/badge/-v0.4-lightgray.svg)
- [ ] Field of view
- [ ] Difficulty levels
      ![v1.0](https://img.shields.io/badge/-v1.0-lightgray.svg)
- [ ] Unit tests with [Check](https://github.com/libcheck/check)
