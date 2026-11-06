# short-notes

`short_notes` is a simple program to add and search short notes.

Example of note created by the program:

```txt
title: this is title
tags: this is tag
date: Fri Nov  6 16:42:13 2026
----
this is content

```

This program is written in pure C. The program is tested on Debian system but it should work on various Linux distribution. It depends mostly on standard C libraries. However, `unistd.h` has been used in a few places, together with the usage of some "unsafe" functions in `string.h`, I presume it will have a hard time running with Visual Studio.

## Usage

Add note:

```
short_notes [add|a]
```

Search note with keyword (in title/tag/content). If no argument is given then in full text.

```
short_notes [search|s] [--title|--tag|--content] keyword
```

Config the repository for all the notes in current directory.
```
short_notes config set <dir>
```

### Config file

By default no config file is needed. One can create a config file precising the repository to save notes with the `short_notes config` command (See [Usage](#usage)), which will create a file with the name ".sn_config", like the following one:

```txt
repo: /home/xxx/short-notes

```

Use this command a second time will overwrite previous configuration.

As there is no parser in this program, please notice that the file is parsed via a `sscanf` call. Any change of the file (e.g. add a line of comment with `#` will just corrupt the config file).

## Compilation

Compilation is configured with [CMakeLists.txt](./CMakeLists.txt), which takes less than 15 lines. For those who are unfamiliar with CMake, you need to install Cmake, then running the following commands in the directory containing the CMakeList file.

```
cmake -Bbuild -S.
cmake --build build
```

Options such as `--parallel` can accelerate the compiling process, if somehow it takes more than three seconds to compile the project.

### Memory leak detection

I commented out the option involving `fsantize` in CMakeLists. Activating these options can provide more details report if the program crashes with memory issues. You may want to activate these options if you want to modify the program. These options do slow down the program.
