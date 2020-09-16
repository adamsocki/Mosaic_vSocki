
// @TODO: I want to be able to specity a game to run in this file so I can provide numerous examples.

#define MOSAIC 1
#define NETWORK_SIMPLE 0
#define MESSAGING 0
#define PONG_BAD 0
#define PARTICLES 0
#define TYPING 0
#define COLLISION 0
#define SPRITE 0

#if MOSAIC
#include "mosaic.cpp"

#elif NETWORK_SIMPLE
#include "examples/network_simple.cpp"

#elif MESSAGING
#include "examples/messaging_example.cpp"

#elif PONG_BAD
#include "examples/pong_bad.cpp"

#elif PARTICLES
#include "examples/pretty.cpp"

#elif TYPING
#include "examples/typing_game.cpp"

#elif COLLISION
#include "examples/collision_example.cpp"

#elif SPRITE
#include "examples/sprite_basics.cpp"

#endif

