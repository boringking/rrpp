project(RRPP)
set(src major_node.c transport_node.c)
add_library(node STATIC ${src})
