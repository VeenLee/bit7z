bit7z
=====

**bit7z** is a C++ static library aiming to offer a clean, simple and object-oriented interface to the dlls supplied by the 7zip SDK. It allows to compress to and extract from many archive file formats from C++ code.

![](http://img.shields.io/badge/version-v1.0.0--beta.2-blue.svg?style=flat) ![](http://img.shields.io/badge/license-GPL%20v2-red.svg?style=flat) ![](http://img.shields.io/badge/platform-windows-green.svg?style=flat) ![](http://img.shields.io/badge/compiler-MSVC-lightgrey.svg?style=flat)

## Features ##
bit7z supports the following features:

+ Compression of files and directories with the following archive formats: 7z, XZ, BZIP2, GZIP, TAR, ZIP and WIM
+ Extraction of the following archive formats: 7z, ARJ, BZIP2, CAB, CHM, CPIO, CramFS, DEB, DMG, FAT, GZIP, HFS, ISO, LZH, LZMA, MBR, MSI, NSIS, NTFS, RAR, RPM, SquashFS, TAR, UDF, VHD, WIM, XAR, XZ, Z and ZIP.
+ Creation of encrypted archives (only for 7z and ZIP formats)
+ Archive header encryption (only for 7z format)
+ Choice of the compression level (from none to ultra, depending on the output archive format)
+ Solid archives (only for 7z)

Please note that the presence or not of some of these features depends on the particular .dll used along with bit7z. For example, the 7z.dll should support all these features, while 7za.dll should support only the 7z file format and the 7zxa.dll can only extract 7z files.

## Getting Started (Library Usage) ##

### Extracting files from an archive ###

~~~~~~~~~~~~~{.cpp}
#include "include/bitextractor.hpp"
#include "include/bitexception.hpp"
...
try {
	using namespace bit7z;

	BitLibrary lib(L"7za.dll");
	BitExtractor extractor(lib, BitInFormat::SevenZip);
	
	//extracts a simple archive
	extractor.extract(L"path/to/archive.7z", L"output/dir/");

	//extracts an encrypted archive
	extractor.extract(L"path/to/another/archive.7z", L"output/dir/", L"password");
} catch ( const BitException& e ) {
	//do something with e.what()...
}
~~~~~~~~~~~~~

### Compressing files into an archive ###

~~~~~~~~~~~~~{.cpp}
#include "include/bitcompressor.hpp"
#include "include/bitexception.hpp"
...
try {
	using namespace bit7z;

	vector<std::wstring> files = {L"path/to/file1.jpg", L"path/to/file2.pdf"};
	BitLibrary lib(L"7z.dll");
	BitCompressor compressor(lib, BitOutFormat::Zip);

	//creates a simple zip archive
	compressor.compressFiles(files, L"output_archive.zip");

	//compresses a directory
	compressor.compressDirectory(L"dir/path/", L"dir_archive.zip");

	//creates an encrypted zip archive
	compressor.setPassword(L"password");
	compressor.compressFiles(files, L"protected_archive.zip");
} catch ( const BitException& e ) {
	//do something with e.what()... 
}
~~~~~~~~~~~~~

## Usage Requirements ##
+ **OS:** Windows
+ **Compiler:** MSVC (tested with version 2013)

Note: in order to use this library you should link your program with both **bit7z** and *oleaut32* (e.g. `-lbit7z -loleaut32`)

## Building Requirements ##
+ **OS:** Windows
+ **Compiler:** MSVC 2012/2013
+ **Build Automation:** QMake (used only to maintain the project, bit7z does not depend on Qt libraries!)
+ **Third-party libraries:** 7z SDK (at least version 9.20)

*Note:* the 7z SDK must be placed in the path "&lt;project root&gt;/lib/7zSDK/".

The 7zSDK folder should look like this:

![](http://i.imgur.com/pgS7UHl.png)

In order to build this library you can use Qt Creator or the *build.cmd* in the project root folder.

*Note:* before using the build.cmd script, you should have the paths of Qt 5 (to use qmake) and MSVC in the PATH environment variable.

Example (x64):

~~~~~~~~~~~~~{.bat}
set PATH=%PATH%;C:\path\to\x64\qt5\bin
vcvarsall amd64
build x64
~~~~~~~~~~~~~

Example (x86):

~~~~~~~~~~~~~{.bat}
set PATH=%PATH%;C:\path\to\x86\qt5\bin
vcvarsall x86
build x86
~~~~~~~~~~~~~

## License (GPL v2) ##
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

Copyright (C) 2014 Riccardo Ostani ([@rikyoz](https://github.com/rikyoz))