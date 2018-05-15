# this is required
SET(CMAKE_SYSTEM_NAME Linux)

# specify the cross compiler
set(CROSS mips-openwrt-linux-) 
set(CMAKE_AR         ${CROSS}ar CACHE FILEPATH "Archiver")
set(CMAKE_C_COMPILER ${CROSS}gcc)
set(CMAKE_LINKER     ${CROSS}ld)
set(CMAKE_NM         ${CROSS}nm)
set(CMAKE_OBJCOPY    ${CROSS}objcopy)
set(CMAKE_OBJDUMP    ${CROSS}objdump)
set(CMAKE_RANLIB     ${CROSS}ranlib)
set(CMAKE_STRIP      ${CROSS}strip)

