SA-MP PVar Plugin
================

[![sampctl](https://img.shields.io/badge/sampctl-PVar-2f2f2f.svg?style=for-the-badge)](https://github.com/rizlone/samp-pvar-plugin)

## Installation

Simply install to your project:

```bash
sampctl package install rizlone/samp-pvar-plugin
```

This plugin is an extension to the per-player variable (PVar) system

- Much faster execution time
- PVars are automatically deleted when a player leaves the server (after OnPlayerDisconnect), meaning you don't have to manually reset variables for the next player who joins.

Build Instructions
---------------------

Install the [CMake](http://cmake.org) and generate a project
