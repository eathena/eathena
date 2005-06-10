zlib-asm-st-zlib.lib 
- zlib static library build with asm option on single thread model
  link with this if your app is single thread

zlib-asm-mt-zlib.lib 
- zlib static library build with asm option on multithread model
  link with this if your app is multithread

zlib1.lib
- dll stub library for dynamically loading zlib1.dll

zlib1.exp
- dll export table, necessary for some linkers to find proper symbols

zlib1.dll
- dynamic library build with asm option