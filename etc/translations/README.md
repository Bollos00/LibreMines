# Requirements

In order to help translating LibreMines, it is necessary to have the [Qt Linguistic Tool](https://doc.qt.io/qt-5/qtlinguist-index.html) installed.

On Arch Linux and its derivatives, it can be done by installing the package `qt6-tools` (or `qt5-tools` for Qt5):
```sh
sudo pacman -Sy --needed qt6-tools
```

On Debian and its derivatives, it can be done with:
```sh
sudo apt-get install qt6-tools-dev
```

# Updating and creating translation files

In order to update translation files (TS format), on the main directory, run:
```sh
lupdate src -ts etc/translations/libremines_es.ts etc/translations/libremines_pt_BR.ts
```

It will update the files `etc/translations/libremines_es.ts` (Spanish) and `etc/translations/libremines_pt_BR.ts` (Brazilian Portuguese) using the source directory `src`.

To generate a TS file for a new language, just append it to the end of the command above. For example, to generate a TS file for French, run:
```sh
lupdate src -ts etc/translations/libremines_es.ts etc/translations/libremines_pt_BR.ts etc/translations/libremines_fr.ts
```

The command above will generate the file `etc/translations/libremines_fr.ts`.

# Translating the software

You can use Qt Linguistic tool to translate the software. To do so, open Qt Linguistic and open the TS file you want to translate.

# Generating QM files from the TS files

To generate the QM files, simply pass the option `UPDATE_TRANSLATIONS="YES"` on the cmake part on the [build process](https://github.com/Bollos00/LibreMines#building):

```sh
git clone https://github.com/Bollos00/LibreMines.git
cd LibreMines
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DUPDATE_TRANSLATIONS="YES" ..
make

```

Qt uses files of type QM to translate the application. While TS files are more human readable and easier to edit, QM files are non-human readable and faster to process.
