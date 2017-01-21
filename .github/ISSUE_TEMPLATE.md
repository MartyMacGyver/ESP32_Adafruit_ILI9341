# If this is a feature request, describe what you're looking to see in a future version!

# If this is a problem report, follow these steps:

### Do a clean build

This is the quickest way to flush out common problems.

### Erase before flash

Rarely, this can resolve strange issues on the target.

### Check to see if the problem has already been reported

First, please search for matching issue here (including closed issues, for which the fix may not have been released yet).

### Describe your build environment
  - OS version Ubuntu 16.04, Windows 7, etc.

  - If Windows, are you using the MSYS2 environment from Espressif or something different?

  - What esp-idf version are you using?
`git -C "$IDF_PATH" rev-parse HEAD`

  - Check that required components described in the documentation are installed

### Desrcibe what happened

  - Did this not work on the target? What did it do?

  - Did the build fail? Send part or all of the build log.

  - Logging a build (and most commands) is easy:
`make {options}  2>&1 | tee build.log`

