
enum TileState {
     TileState_Default,
};

struct Tile {
    bool active;
    TileState state;

    // This is kinda redundant because the tile can't move, but it's easier to have a tile
    // and know where it is than pass it's position around. 
    vec2i position;
    vec4 color;
};

// @NOTE: origin of the grid is top left so coordinate (0, 0) is top left.

struct MyData {
    // @NOTE: this is where you can put in any data specific to your game.
};

struct MosaicMem {
    // @TODO: separate this stuff out into internal state that the user doesnt touch
    real32 lineThickness;
    real32 tileSize;
    // This is the size of the grid in world units
    vec2 gridSize;
    vec2 gridOrigin;
    real32 padding;

    RectBuffer rectBuffer;
    
    vec4 screenColor;
    vec4 boardColor;
    vec4 lineColor;
    vec4 textColor;

    bool onlyDrawBorder;
    
    uint8 gridWidth;
    uint8 gridHeight;

    uint32 tileCapacity;
    Tile *tiles;

    Tile *hoveredTile;
    Tile *hoveredTilePrev;

    MyData data;

    vec2 guyPos;
    float guyDir;
    float guyUp;
};
