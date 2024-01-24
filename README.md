# MSense File Extraction Program for Windows
Special thanks to for the dokany littlefs integration code on which this repository is based off of.



## Instructions for use

To run, make sure an Msense device is connected to the PC through USB. Once this is verified double click the exe, (MSenseFileExtraction.exe).
After running the the application will make a folder in the directory with which the exe was ran pop up, and all files from the MSense Device will be transfered there.
Optionally, you can also run with command line arguments:

`MSenseFileExtraction.exe <folder path>`<mount point> <block device> 


To obtain a list of physical drives, you might want to use `wmic diskdrive list brief`
* `<folder path>` A folder on the PC to which all device files will be copied to. 
* `<mount point>` A folder to mount the filesystem to. Can be a new drive or existing folder (`C:/...`)
* `<block device>` block storage for the filesystem. Can be either drive letter (`E:`) 
  or `PhysicalDriveN`. Beware that using `E:` notation will write at *partition* and no at the physical drive.
* `-d` Enable debug mode
* `--block-size <size>` Specify block size (default 4096)
* `--unit-size <size>` Specify read/prog size (default 512)

## How to build
* Init the git repo submodules properly
* Open the Visual Studio Sln, and build the project.


To add Dokany (not required, just an optional module if you would like to view the MSense file system as a drive)
* Install the [dokany driver](https://github.com/dokan-dev/dokany/wiki/Installation)
* Install [CMake](https://cmake.org/download/)
* Install Visual Studio (or CLion)
* run `generate-project.bat`
* Open the `win/littlefs_dokany.sln`
* Build
* Run it: `\win\Debug>littlefs_dokany.exe K:\ E:`
This will mount a new device `K:` using existing device (`PhysicalDrive2`, e.g. an SD card) as block storage.
