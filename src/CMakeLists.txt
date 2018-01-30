project(RRPP)

#set(CMAKE_EXE_LINKER_FLAGS -pthread)

set(rrpplibs node protocol kit)

# rrpp
add_executable(rrpp main.c)
target_link_libraries(rrpp ${rrpplibs} cgi ip1829 portseq)


install(TARGETS rrpp
        RUNTIME DESTINATION /tftpboot/
)
