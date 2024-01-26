#define NETWORKING_001 0
#define CUBENET_001 1


#if NETWORKING_001
#include "socki/socki_simple_net.cpp"

#elif CUBENET_001
#include "socki/socki_cube_net.cpp"

#endif



