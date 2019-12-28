SysHack
=======

![v0.1](https://img.shields.io/badge/dev-v0.1-brightgreen.svg)

System Shock 2 in NCurses.

## Build

```sh
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
$ bin/syshack
```

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
- [ ] Level generation
- [ ] Field of view
- [ ] Difficulty levels
