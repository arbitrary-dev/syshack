SysHack
=======

[![v0.1](https://img.shields.io/badge/dev-v0.1-brightgreen.svg)](../../tree/v0.1)

System Shock 2 in NCurses.

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

- [v] Walking character
- [v] Walking droid
- [v] Ebites!
  [![v0.1](https://img.shields.io/badge/-v0.1-brightgreen.svg)](../../tree/v0.1)
- [ ] Level generation
  - rooms merging
  - rendering
  - connect rooms with doors
- [ ] Collision detection
- [ ] Rebase on `v0.2`
  [![v0.2](https://img.shields.io/badge/-v0.2-lightgray.svg)](../../tree/v0.2)
・ NPC interactions
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- dialogs
- [ ] Field of view
- [ ] Difficulty levels
  [![v1.0](https://img.shields.io/badge/-v1.0-lightgray.svg)](../../tree/v1.0)
- [ ] Unit tests with [Check](https://github.com/libcheck/check)
