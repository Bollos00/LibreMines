New minefield themes can be easily included into LibreMines. They are a bunch of SVG files inside a directory that indicates the theme name, as it shown above:
```
minefield_theme_name
├── 0.svg
├── 1.svg
├── 2.svg
├── 3.svg
├── 4.svg
├── 5.svg
├── 6.svg
├── 7.svg
├── 8.svg
├── boom.svg
├── flag.svg
├── mine.svg
├── no_flag.svg
└── wrong_flag.svg
```

The name of each file should be exactly as shown, you can use one of the available themes ([like this one](https://github.com/Bollos00/LibreMines/tree/master/share/minefield_themes/classic_dark)) as template.

Once the theme is created, you need to move the directory to a location where the game can find it.

In Windows you can put it in one of those directories:

|                                            |
|--------------------------------------------|
| `C:/Users/<USER>/AppData/Local/libremines/minefield_extra_themes` |
| `C:/ProgramData/libremines/minefield_extra_themes`                |

While on GNU/Linux, the location is one of the above:

|                              |
|------------------------------|
| `~/.local/share/libremines/minefield_extra_themes`   |
| `/usr/local/share/libremines/minefield_extra_themes` |
| `/usr/share/libremines/minefield_extra_themes`       |

See the [QStandardPaths documentation](https://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum) for more details.

A proper configuration on GNU/Linux systems would be:

```
└── ~/.local/share/libremines/minefield_extra_themes
    └── minefield_theme_name
        ├── 0.svg
        ├── 1.svg
        ├── 2.svg
        ├── 3.svg
        ├── 4.svg
        ├── 5.svg
        ├── 6.svg
        ├── 7.svg
        ├── 8.svg
        ├── boom.svg
        ├── flag.svg
        ├── mine.svg
        ├── no_flag.svg
        └── wrong_flag.svg

```

And for Windows it would look like this:
```
└── C:/Users/<USER>/AppData/Local/libremines/minefield_extra_themes
    └── minefield_theme_name
        ├── 0.svg
        ├── 1.svg
        ├── 2.svg
        ├── 3.svg
        ├── 4.svg
        ├── 5.svg
        ├── 6.svg
        ├── 7.svg
        ├── 8.svg
        ├── boom.svg
        ├── flag.svg
        ├── mine.svg
        ├── no_flag.svg
        └── wrong_flag.svg

```
