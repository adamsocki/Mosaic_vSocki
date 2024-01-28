
// This is my IP address on the local network.
// Only machines on my local network are able to communicate
// with this address.
// open up command prompt and type ipconfig to find out your machine's local IP address
const uint32 ServerAddress = MakeAddressIPv4(192, 168, 56, 1);
const uint16 ServerPort = 30000;

// @NOTE TO STUDENTS: look up the hash function in the engine!
const int32 PacketID = Hash("CubeNet");


struct Player 
{
    vec2 position;
    vec2 velocity;

    vec4 color; 

    Rect rect;

};

struct World
{
    vec2 position;
    vec4 color;

    Rect rect;
};



struct ClientInfo {
    uint32 address;
    uint16 port;

    real32 lastPingTimeFromServer;
};

struct ServerData 
{
    // @NOTE: we could have a buffer of these if we wanted multiple clients.
    DynamicArray<ClientInfo> clients;
};

struct ClientData {
    real32 lastPingTimeFromServer;
    bool connected;

    bool isReady;

    bool receivedWorld;

};

struct MyCubeGameData
{

    World gameWorld;


    ServerData serverData;

    ClientData clientData;


};

// Client mode and a Server mode
// The client is going to send pings to the server
// The server is going to listen for pings and then send pings to all the clients
// The client knows the servers address, but the server doesnt know who the client is yet

MyCubeGameData *GameData = NULL;

bool debugMode = true;

void InitServer()
{ 
    // GameData->serverData = {};
    // @TODO:create world
    GameData->serverData.clients = MakeDynamicArray<ClientInfo>(&Game->permanentArena, 128);
   //GameData->serverData.clients = (ClientInfo*)malloc(numberOfClientsAllowed * sizeof(ClientInfo));

    //GameData->serverData.clients
    Print("M");
    //GameData.gameWorld.position =  V2(0.4f, 0.5f);
    //GameData.gameWorld.color =     V4(0.3f, 0.4f, 0.4f, 1.0);
    //GameData.gameWorld.rect.max =  V2( 0.1f,  0.1f);
    //GameData.gameWorld.rect.min =  V2(-0.1f, -0.1f);

    //memset(GameData->serverData.clients, 0, sizeof(ClientInfo) * 4);
}

void InitClient()
{

}


void MyInit() {
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

    Print("Received: %d", network->packetsReceived.count);


    for (int i = 0; i < network->packetsReceived.count; i++)
    {
        ReceivedPacket *received = &network->packetsReceived[i];

        if (received->packet.id != PacketID)
        {
            continue;
        }
        Print("Received: %d", network->packetsReceived.count);


        
    }

    ClientData *client = &GameData->clientData;

    GamePacket packet = {};
    packet.id = PacketID;
    packet.type = GamePacketType_Ping;
    packet.frame = Game->frame;

    // INPUT LISTEN FOR ENTER TO SPAWN INTO GAME

    
    if (InputPressed(Keyboard, Input_Return))
    {
        client->isReady = true;
       // packet.data[0] = client->isReady;
    }




    // If you dont know what memcpy is, you need to look it up ASAP
    memcpy(packet.data, &Game->time, sizeof(real32));

    // Honestly we could just send the packet directly, but I want to showcase
    // that we have a buffer to accumulate multiple packets and then send
    // over in one pass.
    PushBack(&network->packetsToSend, packet);
    
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
        client->lastPingTimeFromServer = Game->time;
        client->connected = true;
    }

    real32 timeSincePing = Game->time - client->lastPingTimeFromServer;

    if (timeSincePing > 1.0f) {
        client->connected = false;
    }




    // HANDLE INPUT






    //RENDER 

    if (!client->isReady)
    {
        DrawTextScreen(&Game->serifFont, V2(0.5f, 0.85f), 0.02f, V4(0.0f, 0.98f, 0.2f, 1.0f), true, "Press ENTER to Spawn into World!");
    }
    else if (!client->receivedWorld)
    {
        DrawTextScreen(&Game->serifFont, V2(0.5f, 0.85f), 0.02f, V4(0.8f, 0.1f, 0.2f, 1.0f), true, "World being generated...");
    }



    

    if (debugMode)
    {
        if (client->connected ) {
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

    
    Print("packets received %u", network->packetsReceived.count);
        // GET AND REGISTER CLIENTS
    for (int i = 0; i < network->packetsReceived.count; i++) {
        ReceivedPacket received = network->packetsReceived[i];       
        GamePacket *p = &received.packet;


        Print("receiver");

        Print("packets received %d", received.fromAddress);

        ClientInfo *user = NULL;

        for (int j = 0; j < server->clients.count; j++)
        {
            ClientInfo *u = &server->clients[j];
            if (received.fromAddress == u->address)
            {
                user = u;
                break;
            }
        }


        if (received.packet.type == GamePacketType_Ping)
        {
            if (user != NULL)
            {
                user->lastPingTimeFromServer = Game->time;
            }
            else {
                ClientInfo user1 = {};
                user1.address = received.fromAddress;
                user1.port = received.fromPort;

                user1.lastPingTimeFromServer = Game->time;
                Print("added already!");
                PushBack(&server->clients, user1);
            }
        }

    }
        /*// Print("Hitting :Packet ");

        // if (p->id != PacketID) {
        //     continue;
        // }

        // ClientInfo *foundClient = NULL;
        
        // for (int j = 0; j < server->clients.count; j++) 
        // {
        //     ClientInfo *client = &server->clients[j];

        //     if (received.fromAddress == client->address && received.fromPort == client->port) // Have we already connected ??
        //     {
        //         foundClient = client;
        //         break;
        //     }
        //     else if (client->address == 0) // if we havent already connected, lets
        //     {
        //         client->address = received.fromAddress;
        //         client->port = received.fromPort;

        //         foundClient = client;
        //         break;
        //     }

           

        // }

         

        // if (p->type == GamePacketType_Ping)
        // {
        //     Print("Hitting Ping ");

        //     if (foundClient != NULL) // DO WE HAVE THAT CLIENT REGISTERED?
        //     {
        //         foundClient->lastPingTimeFromServer = *((real32 *)p->data);
        //     }
        //     else // LET'S REGISTER THAT CLIENT
        //     {
        //         ClientInfo foundClient = {};
        //         foundClient.address = received.fromAddress;
        //         foundClient.lastPingTimeFromServer = *((real32 *)p->data);

        //         PushBack(&server->clients, foundClient);
        //     }
        // }




        // if (foundClient == NULL) {
        //     continue;
        // } */




 // 1 PING PACKET - SEND

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

        // 2 CHECK FOR NEED TO SEND WORLD
        
    
        





        Print("ping");

        // Here we send the packets where we want to.
        // @NOTE: our server is very simple: it assumes only one client,
        // but if we had multiple clients we might want to send this data to
        // all of them, or maybe only send some data to some of them,
        // in which case we might want something more sophisticated than just
        // one buffer of GamePackets

        //Print("Count: ", server->clients.count);
        for (int i = 0; i < network->packetsToSend.count; i++) {
            GamePacket *p = &network->packetsToSend[i];

            Print("bbb");

            for (int j = 0; j < server->clients.count; j++) {
                Print("n");
                ClientInfo *client = &server->clients[j];

                uint32 bytesSent = SendPacket(&network->socket, client->address, client->port, p, sizeof(GamePacket));

                if (bytesSent != sizeof(GamePacket)) {
                    Print("Failed to send %d bytes, sent %d instead", sizeof(GamePacket), bytesSent);
                }
                else
                {
                    Print("sent");
                }
             }
        }

   // if (server->clients[0].address != 0) 
   // {
//
//
   //    
   // }
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
