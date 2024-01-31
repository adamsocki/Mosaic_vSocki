
// This is my IP address on the local network.
// Only machines on my local network are able to communicate
// with this address.
// open up command prompt and type ipconfig to find out your machine's local IP address

//const uint32 ServerAddress = MakeAddressIPv4(10, 0, 0, 20);
const uint32 ServerAddress = MakeAddressIPv4(192, 168, 56, 1);
const uint16 ServerPort = 30000;

// @NOTE TO STUDENTS: look up the hash function in the engine!
const int32 PacketID = Hash("CubeNet");


struct Player 
{
    int32 objectID;

    vec2 position;
    vec2 velocity;

    vec4 color; 

    Rect rect;

    bool needsWorld;
};

struct WorldObject
{
    int32 objectID;

    vec2 position;
    vec2 size;
    vec4 color;
};



enum InputMovementID
{
    Up,
    Down,
    Left, 
    Right,
};

struct World
{
    vec2 position;
    vec4 color;

    vec2 spawnPoint;


    Rect worldBounds;

    DynamicArray <WorldObject> worldObjects;

};




struct ClientInfo {
    uint32 address;
    uint16 port;

    real32 lastPingTimeFromServer;

    Player* player;
};


struct InputPacket
{
    int32 clientID;

    InputKeyboardDiscrete input;
};




struct PlayerPacket
{
    int32 playerID;

    vec2 position;
};



struct ServerData 
{
    // @NOTE: we could have a buffer of these if we wanted multiple clients.
    DynamicArray<ClientInfo> clients;
    DynamicArray<InputPacket> inputs;

};

struct PlayerData {
    real32 lastPingTimeFromServer;
    bool connected;

    bool isReady;

    bool receivedWorld;

};

struct CriticalPacket 
{
    GamePacketType type;

    real32 timeSinceSent;
    real32 timeToResend;

    GamePacket packet;

};

struct MyCubeGameData
{

    World gameWorld;
    

    ServerData serverData;



    DynamicArray <Player> players;

    PlayerData playerData;

    DynamicArray <CriticalPacket> criticalPackets;

};

// Client mode and a Server mode
// The client is going to send pings to the server
// The server is going to listen for pings and then send pings to all the clients
// The client knows the servers address, but the server doesnt know who the client is yet

MyCubeGameData *GameData = NULL;
bool debugMode = true;

void LostPacketChecker()
{

}

void InitServer()
{ 
    // @TODO:create world


    GameData->serverData.clients = MakeDynamicArray<ClientInfo>(&Game->permanentArena, 128);
    GameData->serverData.inputs = MakeDynamicArray<InputPacket>(&Game->permanentArena, 128);
    GameData->players = MakeDynamicArray<Player>(&Game->permanentArena, 128);


   //GameData->serverData.clients = (ClientInfo*)malloc(numberOfClientsAllowed * sizeof(ClientInfo));


    GameData->gameWorld.spawnPoint = V2(0.0f, 0.0f);
    

    for (int i = 0; i < 4; i++)
    {
        WorldObject w = {};
        w.objectID = i;
        w.position = V2(0.1f + (i / 7.0f), 0.6f);
        w.size = V2(0.1f, 0.05f);
        w.color = V4(1.0f - (i / 5.0f), 0.1f + (i / 5.0f), 0.4f, 1.0f);
        PushBack(&GameData->gameWorld.worldObjects, w);
    }

    //GameData->serverData.clients
    //GameData.gameWorld.position =  V2(0.4f, 0.5f);
    //GameData.gameWorld.color =     V4(0.3f, 0.4f, 0.4f, 1.0);
    //GameData.gameWorld.worldBounds.max =  V2( 10.1f,  10.1f);
    //GameData.gameWorld.worldBounds.min =  V2(-10.1f, -10.1f);

    //memset(GameData->serverData.clients, 0, sizeof(ClientInfo) * 4);


}
void InitClient()
{

}
void MyInit() 
{
    NetworkInfo *network = &Game->networkInfo;

    InitNetwork(&Game->permanentArena);
     
    //Game->myData = malloc(sizeof(MyCubeGameData));
    //GameData = (MyCubeGameData*)Game->myData;
    //memset(GameData, 0, sizeof(MyCubeGameData) * 1);


    //GameData = {};
    //ServerData* server = &GameData.serverData;
    //``sserver->clients = {};
    


    // Create sockets that we'll use to communicate.
    Game->myData = malloc(sizeof(MyCubeGameData));
    GameData = (MyCubeGameData*)Game->myData;
    memset(GameData, 0, sizeof(MyCubeGameData));

    GameData->gameWorld.worldObjects = MakeDynamicArray<WorldObject>(&Game->permanentArena, 128);

    // This means that this code is being executed by server.exe
    if (IS_SERVER) 
    {
        InitServer();

        InitSocket(&network->socket, GetMyAddress(), ServerPort, true);
    }
    else {
        // If we don't care what port we're on we can pass in 0 and it'll pick one
        // for us. This can be useful for doing networking locally where we want
        // to have multiple game instances open and have the server communicate with each.
        InitClient();
        InitSocket(&network->socket, GetMyAddress(), 0, true);
    }
}

void ClientUpdate() {
    // LOGIC
    NetworkInfo *network = &Game->networkInfo;
    //Print("Received: %d", network->packetsReceived.count);

    for (int i = 0; i < network->packetsReceived.count; i++)
    {
        ReceivedPacket *received = &network->packetsReceived[i];
        GamePacket* p = &received->packet;

        if (received->packet.id != PacketID)
        {
            continue;
        }
        Print("Received: %d", network->packetsReceived.count);

        if (p->type == GamePacketType_World)
        {
            //
            //p->data
            WorldObject wo = {};
            memcpy(&wo, p->data, sizeof(WorldObject));
            PushBack(&GameData->gameWorld.worldObjects, wo);
        }
        
    }

    PlayerData *player = &GameData->playerData;
    { 
        GamePacket packet = {};
        packet.id = PacketID;
        packet.type = GamePacketType_Ping;
        packet.frame = Game->frame;

        // If you dont know what memcpy is, you need to look it up ASAP
        memcpy(packet.data, &Game->time, sizeof(real32));

        //memcpy(packet.data + sizeof(real32), false, sizeof(bool));

        // Honestly we could just send the packet directly, but I want to showcase
        // that we have a buffer to accumulate multiple packets and then send
        // over in one pass.
        PushBack(&network->packetsToSend, packet);
    }
   
    // INPUT LISTEN FOR ENTER TO SPAWN INTO GAME

    
    if (InputPressed(Keyboard, Input_Return))
    {
        player->isReady = true;
       // packet.data[0] = client->isReady;
    }


     {
        GamePacket packet = {};
        packet.id = PacketID;
        packet.type = GamePacketType_Input;

        ((InputPacket *)packet.data)->input = Input_KeyboardDiscreteCount;

        if (InputHeld(Keyboard, Input_W)) {
            ((InputPacket *)packet.data)->input = Input_W;
        }
        if (InputHeld(Keyboard, Input_S)) {
            ((InputPacket *)packet.data)->input = Input_S;
        }

        PushBack(&network->packetsToSend, packet);
    }
    
    /*if (client->connected && !client->receivedWorld && client->isReady)
    {
        // request world  
        GamePacket packet = {}; 
        packet.id = PacketID;
        packet.type = GamePacketType_World;

        //packet.data[0] = client-> @@@@ what would go

        PushBack(&network->packetsToSend, packet);

    }*/



    // Here we send the packets where we want to.
    for (int i = 0; i < network->packetsToSend.count; i++) {
        GamePacket *p = &network->packetsToSend[i];
        uint32 bytesSent = SendPacket(&network->socket, ServerAddress, ServerPort, p, sizeof(GamePacket));
        if (bytesSent != sizeof(GamePacket)) {
            Print("Failed to send %d bytes, sent %d instead", sizeof(GamePacket), bytesSent);
        }
    }

    // TODO: We should probably check to make sure the packet id is from our game.
    if (network->packetsReceived.count > 0) {
        player->lastPingTimeFromServer = Game->time;
        player->connected = true;
    }

    real32 timeSincePing = Game->time - player->lastPingTimeFromServer;

    if (timeSincePing > 1.0f) {
        player->connected = false;
    }




    // HANDLE INPUT






    //RENDER 

    if (!player->isReady)
    {
        DrawTextScreen(&Game->serifFont, V2(0.5f, 0.85f), 0.02f, V4(0.0f, 0.98f, 0.2f, 1.0f), true, "Press ENTER to Spawn into World!");
    }
    else if (!player->receivedWorld)
    {
        DrawTextScreen(&Game->serifFont, V2(0.5f, 0.85f), 0.02f, V4(0.8f, 0.1f, 0.2f, 1.0f), true, "World being generated...");
    }


    DrawRectScreen(V2(400, 500), V2(24.0f, 48.0f), V4(0.5f, 0.5f, 0.5f, 0.5f));

    // RENDER WORLD
    for (int i = 0; i < GameData->gameWorld.worldObjects.count; i++)
    {
        WorldObject o = GameData->gameWorld.worldObjects[i];
        DrawRect(o.position, o.size, o.color);
    }
    

    if (debugMode)
    {
        if (player->connected ) {
            DrawTextScreen(&Game->serifFont, V2(0.5f, 0.1f), 0.02f, V4(1), true, "CONNECTED TO SERVER!");
        }
        else {
            DrawTextScreen(&Game->serifFont, V2(0.5f, 0.1f), 0.02f, V4(1, 0, 0, 1), true, "NO CONNECTION...");        
        }

        DrawTextScreen(&Game->serifFont, V2(0.5f, 0.2f), 0.02f, V4(1, 0, 0, 1), true, "Last Ping Time %.2f", timeSincePing);
    }    
}
void ServerUpdate() {
    NetworkInfo *network = &Game->networkInfo;
    
    ServerData *server = &GameData->serverData; 
    
    // GET AND REGISTER CLIENTS
    for (int i = 0; i < network->packetsReceived.count; i++) {
        ReceivedPacket received = network->packetsReceived[i];       
        GamePacket *p = &received.packet;

        if (p->id != PacketID) {
            continue;
        }

        ClientInfo *user = NULL;
        int32 userIndex = 0;
        Print("User Count: %d !", server->clients.count);

        // THIS STEP IDENTIFIES WHICH USER THE RECEIVED PACKET COMES FROM
        for (int j = 0; j < server->clients.count; j++)
        {
            ClientInfo *u = &server->clients[j];
            if (received.fromAddress == u->address && received.fromPort == u->port)
            {
                user = u;
                userIndex = j;
                break;
            }
        }

        if (received.packet.type == GamePacketType_Ping)
        {
            if (user != NULL)
            {
                user->lastPingTimeFromServer = Game->time;
            }
            else 
            {
                ClientInfo user = {};
                user.address = received.fromAddress;    
                user.port = received.fromPort;

                // Create player

                Player player = {};
                player.color = V4(RandfRange(0.1f, 1.0f), RandfRange(0.1f, 1.0f), RandfRange(0.1f, 1.0f), 1.0f);
                player.position = GameData->gameWorld.spawnPoint;
                player.rect.min = V2(-0.2f, -0.8f);
                player.rect.min = V2(0.2f, 0.8f);

                user.player = &player;

                //Print("User Port: %d !", received.fromPort);
                userIndex = server->clients.count - 1;
                user.lastPingTimeFromServer = Game->time;
                PushBack(&server->clients, user);
            }
        }

        if (received.packet.type == GamePacketType_NeedsWorld)
        {
            if (user != NULL)
            {  
               // bool* n = (bool*)(received.packet.data + sizeof(real32));
                user->player->needsWorld = true;
            }
        }

        if (received.packet.type == GamePacketType_Input)
        {
            InputPacket packet = *(InputPacket*)received.packet.data;
            packet.clientID = userIndex;

            Print("Some Input !");

            if (packet.input == Input_S)
            {
                Print("Input S Received!");

            }
            if (packet.input == Input_W)
            {
                Print("Input W Received!");

            }
            if (packet.input == Input_A)
            {
                Print("Input A Received!");

            }
            if (packet.input == Input_D)
            {
                Print("Input D Received!");

            }
            PushBack(&server->inputs, packet);
        }
    }
       
    for (int i = 0; i < server->inputs.count; i++)
    {
        InputPacket input = server->inputs[i];

        Player *player = NULL;

        for (int j = 0; j < server->clients.count; j++)
        {
            //if (input.id == server->clients[j])
        }
    }

 // 1 PING PACKET - SEND
    {
        GamePacket packet = {};
        packet.id = PacketID;
        packet.type = GamePacketType_Ping;
        packet.frame = Game->frame;

        // If you dont know what memcpy is, you need to look it up ASAP
        memcpy(packet.data, &Game->time, sizeof(real32));

        // Honestly we could just send the packet directly, but I want to showcase
        // that we have a buffer to accumulate multiple packets and then send
        // over in one pass.
        PushBack(&network->packetsToSend, packet);
    }

// 2 Check any Player specific packet sending needs
    for (int i = 0; i < server->clients.count; i++)
    {
        Player *player = server->clients[i].player;
        if (player->needsWorld)
        {
            for (int j = 0; j < GameData->gameWorld.worldObjects.count; j++)
            {
                GamePacket packet = {};
                packet.id = PacketID;
                packet.type = GamePacketType_World;
                packet.criticalPacket = true;

                memcpy((WorldObject*)packet.data, &GameData->gameWorld.worldObjects[j], sizeof(WorldObject));
                PushBack(&network->packetsToSend, packet);
            }
        }
    }

    // Here we send the packets where we want to.
    // @NOTE: our server is very simple: it assumes only one client,
    // but if we had multiple clients we might want to send this data to
    // all of them, or maybe only send some data to some of them,
    // in which case we might want something more sophisticated than just
    // one buffer of GamePackets

    //Print("Count: ", server->clients.count);
    for (int i = 0; i < network->packetsToSend.count; i++) {
        GamePacket *p = &network->packetsToSend[i];

        for (int j = 0; j < server->clients.count; j++) {
            ClientInfo *client = &server->clients[j];

            uint32 bytesSent = SendPacket(&network->socket, client->address, client->port, p, sizeof(GamePacket));

            if (bytesSent != sizeof(GamePacket)) {
                Print("Failed to send %d bytes, sent %d instead", sizeof(GamePacket), bytesSent);
            }
            else
            {

            }
         }

        if (p->criticalPacket)
        {
            CriticalPacket c = {};
            c.packet = *p;
            c.type = p->type;
            c.timeSinceSent = 0.0f;
            c.timeToResend = 4.0f;

            PushBack(&GameData->criticalPackets, c);
        }
    }


}
void MyGameUpdate() {
    NetworkInfo *network = &Game->networkInfo;

    // DynamicArrays are like Lists in C# in case you didnt know.
    // We want to clear everything from the array so we only ever send packets
    // that we created this frame. 
    DynamicArrayClear(&network->packetsToSend);

    // Whether client or server we always want to receive packets.
    ReceivePackets(&network->socket);

    GameData->criticalPackets = MakeDynamicArray<CriticalPacket>(&Game->permanentArena, 128);



    if (IS_SERVER) {
        ServerUpdate();
    }
    else {
        ClientUpdate();
    }
}
