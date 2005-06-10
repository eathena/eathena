Building a dll with visual c results (among others) in a <name>.dll and a <name>.lib. 

The <name>.lib fails to link in Borland C++ Builder ("<name>.lib contains invalid OMF record [...]"). This is due to an incompatible binary format: C++ Builder uses OMF, Visual C++ uses COFF formats. See http://www.bcbdev.com/articles/vcdll.htm for details.

There are two different calling conventions for functions in Windows: __cdecl and __stdcall. The default is __cdecl; so if no calling convention is defined, it uses __cdecl. With this convention, Borland C++ expects functions names to start with a leading underscore "_", whereas Visual C++ (and Intel Compiler as it seems) do omit the leading "_".

The workaround is as follows: start a command prompt, change to the directory where the dll and the lib is installed and type "impdef <name>.def <name>.dll". This creates a .def file from the DLL containing all the exported function names using Borland's impdef-utility. Now one has to create aliasses from the Borland function names to the exported names and add them to the "EXPORTS" section of the def-file. An example for an alias:

    _get_version                  = get_version

The following Python script def_cdecl_borland.py automates this task:

#!/usr/bin/env python
# this program automates the introduction of aliases for function
# names starting with a leading underscore for FFTW3 in a Windows
# def-file

import sys

if (len(sys.argv) < 3):
    raise "Usage: %s  " % sys.argv[0]

lines = open(sys.argv[1], "r").readlines()

outfile=open(sys.argv[2], "w")
outfile.writelines(lines)
exports=0
for line in lines:
    if (exports != 0):
        fields = line.split()
        if (len(fields)> 0):
            outfile.write("    _%-30s = %s\r\n" % (fields[0], fields[0]))
    if (line.find("EXPORTS")!=-1):
        exports=1

The script is called with the filenames for the old and new def-files as arguments ("def_cdecl_borland.py <source>.def <target>.def"). The last step is to create a .lib file from the def-file: "implib <name>.lib <name>.def". The resulting <name>.lib can be added to the C++ Project in C++ Builder and compiles. 