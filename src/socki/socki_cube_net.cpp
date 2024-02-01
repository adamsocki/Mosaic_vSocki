
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
    int32 playerID;

    vec2 position;
    vec2 velocity;

    vec4 color; 

    vec2 size;

    bool needsWorld;
    bool sendUpdatePacket;
    bool isActive;
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
    bool hasWorld;

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


    // Create sockets that we'll use to communicate.
    Game->myData = malloc(sizeof(MyCubeGameData));
    GameData = (MyCubeGameData*)Game->myData;
    memset(GameData, 0, sizeof(MyCubeGameData));

    GameData->gameWorld.worldObjects = MakeDynamicArray<WorldObject>(&Game->permanentArena, 128);
    GameData->criticalPackets = MakeDynamicArray<CriticalPacket>(&Game->permanentArena, 128);
    GameData->players = MakeDynamicArray<Player>(&Game->permanentArena, 128);

    // This means that this code is being executed by server.exe
    if (IS_SERVER) 
    {
        InitServer();
        Print("Server");
        InitSocket(&network->socket, GetMyAddress(), ServerPort, true);
    }
    else {
        InitClient();
        InitSocket(&network->socket, GetMyAddress(), 0, true);
    }
}

void ClientUpdate() {
    // LOGIC
    NetworkInfo *network = &Game->networkInfo;
    PlayerData* player = &GameData->playerData;

    //Print("Received: %d", network->packetsReceived.count);

    for (int i = 0; i < network->packetsReceived.count; i++)
    {
        ReceivedPacket *received = &network->packetsReceived[i];
        GamePacket* p = &received->packet;
        GamePacket p2 = received->packet;

        if (received->packet.id != PacketID)
        {
            continue;
        }
        //Print("Received: %d", network->packetsReceived.count);

        if (p->type == GamePacketType_World)
        {
            //
            //p->data
            WorldObject wo = {};
            memcpy(&wo, p->data, sizeof(WorldObject));
            PushBack(&GameData->gameWorld.worldObjects, wo);
            player->receivedWorld = true;

        }

        if (p->type == GamePacketType_PlayerInit)
        {
            Player receivedPlayer = {};
            memcpy(&receivedPlayer, p->data, sizeof(Player));
            PushBack(&GameData->players, receivedPlayer);
        }

        if (p->type == GamePacketType_PlayerUpdate)
        {

            for (int j = 0; j < GameData->players.count; j++)
            {
                Player* inGamePlayer = &GameData->players[j];
                Player receivedPlayer = {};

                int32 receivedPlayerID = {};

                memcpy(&receivedPlayerID, p->data, sizeof(int32));

                memcpy(&inGamePlayer->position, p2.data + sizeof(int32), sizeof(vec2));
                memcpy(&inGamePlayer->isActive, p->data + sizeof(int32) + sizeof(vec2), sizeof(bool));

               //    if (&inGamePlayer.playerID == receivedPlayerID || true)
                {
                   // memcpy(&inGamePlayer.playerID, p->data, sizeof(int32));
                   
                }


            }
           
        }
        
    }

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
        if (!player->isReady)
        {
            player->isReady = true;
            
            GamePacket packet = {};
            packet.id = PacketID;
            packet.type = GamePacketType_NeedsWorld;
            packet.criticalPacket = true;
            

            PushBack(&network->packetsToSend, packet);

        }
       // packet.data[0] = client->isReady;

    }

    if (player->receivedWorld)
    {
        if (InputPressed(Keyboard, Input_X))
        {
            GamePacket packet = {};
            packet.id = PacketID;
            packet.type = GamePacketType_PlayerSpawnRequest;
            PushBack(&network->packetsToSend, packet);

        }
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
    else
    {
        DrawTextScreen(&Game->serifFont, V2(0.5f, 0.85f), 0.02f, V4(0.8f, 0.1f, 0.2f, 1.0f), true, "Press X to spawn into world...");
    }

    DrawRectScreen(V2(400, 500), V2(24.0f, 48.0f), V4(0.5f, 0.5f, 0.5f, 0.5f));

    // RENDER WORLD
    for (int i = 0; i < GameData->gameWorld.worldObjects.count; i++)
    {
        WorldObject o = GameData->gameWorld.worldObjects[i];
        DrawRect(o.position, o.size, o.color);
    }

    // RENDER PLAYERS
    for (int i = 0; i < GameData->players.count; i++)
    {
        Player p = GameData->players[i];

        if (true)
        {
            Print("no of worldObj : %d", GameData->gameWorld.worldObjects.count);


            DrawRect(p.position, p.size, p.color);
        }

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
        //Print("User Count: %d !", server->clients.count);

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
//                user->player->needsWorld = false;
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
                player.playerID = userIndex = server->clients.count - 1;

                player.needsWorld = false;
                player.size = V2(0.4f, 0.4f);
                player.isActive = true;

                user.player = &player;

                //Print("User Port: %d !", received.fromPort);
                user.lastPingTimeFromServer = Game->time;
                PushBack(&server->clients, user);
                PushBack(&GameData->players, player);

                GamePacket packet = {};
                packet.id = PacketID;
                packet.type = GamePacketType_PlayerInit;
                
                memcpy(&packet.data, &player, sizeof(Player));
                PushBack(&network->packetsToSend, packet);

            }
        }

        if (received.packet.type == GamePacketType_NeedsWorld)
        {
            Print("NeedsWorld");
            if (user != NULL)
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
                // bool* n = (bool*)(received.packet.data + sizeof(real32));
               // user->player->needsWorld = true;
            }
        }

        if (received.packet.type == GamePacketType_Input)
        {
            InputPacket packet = *(InputPacket*)received.packet.data;
            packet.clientID = userIndex;


            //Print("Some Input !");

            if (packet.input == Input_S)
            {
                //Print("Input S Received!");

            }
            if (packet.input == Input_W)
            {
                // Print("Input W Received!");

            }
            if (packet.input == Input_A)
            {
                // Print("Input A Received!");

            }
            if (packet.input == Input_D)
            {
                // Print("Input D Received!");

            }
            PushBack(&server->inputs, packet);
        }

        if (received.packet.type == GamePacketType_PlayerSpawnRequest)
        {
            if (user != NULL)
            {

                user->player->isActive = true;
                
                //player.isActive = true;


            }
            else
            {
                ASSERT(true);
            }


        }
    }

    for (int i = 0; i < server->inputs.count; i++)
    {
        InputPacket inputPacket = server->inputs[i];

        Player* player = NULL;

        for (int j = 0; j < server->clients.count; j++)
        {
            //if (input.id == server->clients[j])

            player = server->clients[j].player;
            bool sendUpdate = false;

            if (true)
            {
                if (inputPacket.input == Input_S)
                {
                    Print("Input S Received!");
                    player->position.y -= 7 * (Game->deltaTime);
                    sendUpdate = true;
                }
                else if (inputPacket.input == Input_W)
                {
                    Print("Input W Received!");
                    player->position.y += 2 * (Game->deltaTime);
                    sendUpdate = true;


                }
                else if (inputPacket.input == Input_A)
                {
                    Print("Input A Received!");
                    player->position.x -= 2 * (Game->deltaTime);
                    sendUpdate = true;

                }
                else if (inputPacket.input == Input_D)
                {
                    Print("Input D Received!");
                    player->position.x += 2 * (Game->deltaTime);
                    sendUpdate = true;

                }

               
            }
            if (sendUpdate)
            {
                if (player != NULL)
                {
                    GamePacket packet = {};
                    packet.id = PacketID;
                    packet.type = GamePacketType_PlayerUpdate;
                    memcpy(packet.data, &player->playerID, sizeof(int32));
                    memcpy(packet.data + sizeof(int32), &player->position, sizeof(vec2));
                    memcpy(packet.data + sizeof(int32) + sizeof(vec2), &player->isActive, sizeof(bool));
                    PushBack(&network->packetsToSend, packet);
                }

            }
           
            DynamicArrayClear(&server->inputs);
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
        Print("player count: %d", server->clients.count);
        Player *player = server->clients[i].player;
        
        if (player->sendUpdatePacket)
        {

            GamePacket p = {};
            p.type = GamePacketType_PlayerUpdate;
            p.id = PacketID;
            
        }

    }

    //Print("Count: ", server->clients.count);
    for (int i = 0; i < network->packetsToSend.count; i++) {
        GamePacket *p = &network->packetsToSend[i];

        for (int j = 0; j < server->clients.count; j++) {
            ClientInfo *client = &server->clients[j];

            uint32 bytesSent = SendPacket(&network->socket, client->address, client->port, p, sizeof(GamePacket));

            if (bytesSent != sizeof(GamePacket)) {
                //Print("Failed to send %d bytes, sent %d instead", sizeof(GamePacket), bytesSent);
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




    if (IS_SERVER) {
        ServerUpdate();
    }
    else {
        ClientUpdate();
    }
}
