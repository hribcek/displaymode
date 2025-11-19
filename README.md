# displaymode

`displaymode` is a command-line utility for changing the display resolution on macOS.  It will work on macOS 10.6 or later.

In recent versions of macOS (e.g. macOS Ventura / v13), it's possible to set the resolution via Apple's built-in System Settings app, obviating the need for this utility.  Firstly, enable System Settings > Displays > Advanced > "Show resolutions as list", then enable "Show all resolutions".

## Installation

## With Xcode

If you have Xcode (or the Xcode command line tools) installed, it's best to compile it yourself.  From the terminal:

1. `git clone https://github.com/p00ya/displaymode.git`
2. `cd displaymode`
3. `clang -std=c11 -lm -framework CoreFoundation -framework CoreGraphics -o displaymode displaymode.c`

## Without Xcode

If you don't have Xcode installed, then you can download the binary (note this is not a zip file) then give the OS permission to run it.  From the terminal:

1. `curl -LO https://github.com/p00ya/displaymode/releases/latest/download/displaymode`
2. `xattr -d com.apple.quarantine displaymode`
3. `chmod a+x displaymode`
4. `codesign -f -s "-" -v displaymode`


## Usage

### Change Resolution
To change the resolution of the main display to 1440x900:
```
./displaymode t 1440 900
```

To change the resolution of the secondary display to 1440x900:
```
./displaymode t 1440 900 1
```

Specify a particular refresh rate:
```
./displaymode t 1440 900 @60
```

### List Available Modes
Get a list of active displays and available resolutions:
```
./displaymode d
```

### Standard CLI Flags
Print help message:
```
./displaymode --help
```

Print version information:
```
./displaymode --version
```

Enable verbose output:
```
./displaymode t 1440 900 --verbose
```

### Output Example
```
Display 0 (MAIN):
2560 x 1600 @60.0Hz *
1280 x 800 @60.0Hz
2880 x 1800 @60.0Hz
640 x 480 @60.0Hz !

Display 1:
800 x 600 @75.0Hz *
```
`*` indicates the current mode, and `!` indicates modes not usable for the desktop.

## Other options

`./displaymode h` or `./displaymode --help` prints a summary of the options.

`./displaymode v` or `./displaymode --version` prints the version and copyright notice.

## Logging
`displaymode` now includes a logging system to capture runtime information, warnings, and errors. You can configure the log level using the `setLogLevel` function in the code.

## JSON Output
The tool supports JSON output for display mode information. Use the `--json` flag to enable this feature.

## Tests
To run the tests, use the `make tests` command. This will execute all unit and integration tests, including tests for JSON output and error handling.
