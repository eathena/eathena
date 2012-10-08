-------------------------
 Lua 5.2.1 (by flaviojs)
-------------------------

I decided to use the distribution in https://github.com/LuaDist/lua since it 
uses the cmake build system, which can build almost everywhere. The version 
being used is 59da638d4f1fe2a7c4fbef632d080536bc2a9b2e.

They do the same for a lot of lua libraries, making it easier to integrate 
them if the need arises. Note that they might not build properly in MSVC if a 
dll is being built, since it requires a def file or the use of 
__declspec(dllexport).



 After building:
-----------------

The lua dynamic library goes next to the server executables.
The headers go into the include folder.
The library file goes to the lib folder.

Lua modules that include C files should be built with them.



 Using system libraries:
-------------------------

If it detects lua 5.2 in the system, it will use it by default. For 
non-standard locations, set the environment variable LUA_DIR to the lua 
installation path.

Lua modules that include C files should be built with them.
