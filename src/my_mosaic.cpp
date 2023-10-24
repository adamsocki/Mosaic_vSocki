// @TODO: standardize if our position is the top left or the center
// center is easier visually, but is kinda quirky when it comes to bounding boxes

// @DESIGN: ball passes thru you unless you type the words which activates collision

// @TODO: what if game was about keeping the ball from reaching either side rather than scoring a goal?

// @TODO: main menu

// @BUG: ball can get really fast if the goalie hits the ball behind it, idk
// @BUG: looks like ball counts as scored too early on the right goal? 


const int32 InputLength = 8;

struct Player {
    int32 inputIndex;
    char input[InputLength];
};

const int32 PieceSize = 5;
real32 CollisionDuration = 0.5f;

struct Piece {
    // this is the top left position relative to the axel
    vec2 localPosition;
    vec2 worldPosition;

    bool collisionActive;
    real32 timeCollisionActivated;
};

// @TOOD: get the pieces

const int32 GoalHeight = 32;
const int32 GoalWidth = 4;

struct Goal {
    vec2 position;
};

const int32 BallSize = 3;
const real32 BallMinSpeed = 19;


struct Ball {
    vec2 position;
    vec2 velocity;
};

struct Axel {
    char string[InputLength];

    vec2 position;
    vec2 velocity;

    int32 pieceCount;
    Piece *pieces;
};

const int32 AxelCount = 3;

struct Team {
    bool isPlayer;
    int32 score;
    
    vec4 color;
    Axel axels[AxelCount];
};

struct GameMem {
    Player player;

    Team teams[2];
    Goal goals[2];

    Ball ball;
};

const int32 ArenaWidth = 180;
const int32 ArenaHeight = 80;

struct GameMem Foose = {};

 
void GenerateAxelString(char *string) {
    int32 length = RandiRange(2, 4);

    for (int i = 0; i < length; i++) {
        // our chars will automatically get cast to ints
        string[i] = (char)RandiRange('a', 'z');
    }
}


void StartRound() {
    Ball *ball = &Foose.ball;

    ball->position = V2(ArenaWidth / 2, ArenaHeight / 2);
    
    real32 speed = 17;
    ball->velocity = Rotate(DegToRad(RandfRange(-30, 30)), V2(-1, 0)) * speed;

    for (int t = 0; t < 2; t++) {
        Team *team = &Foose.teams[t];

        // find which direction we should move to have one of our pieces intercept the ball
        for (int a = 0; a < AxelCount; a++) {
            Axel *axel = &team->axels[a];

            GenerateAxelString(axel->string);
        }
    }
}


void MyMosaicInit() {
    //Foose.teams[1].color = V4(0.3f, 0.3f, 0.8f, 1.0f);
    
    SetMosaicGridSize(ArenaWidth, ArenaHeight);

    vec2 center = V2(ArenaWidth / 2, ArenaHeight / 2);

    int32 AxelColumns[6];

    AxelColumns[0] = (ArenaWidth / 2) - 20;
    AxelColumns[1] = AxelColumns[0] - 30;
    AxelColumns[2] = AxelColumns[1] - 30;

    AxelColumns[3] = (ArenaWidth / 2) + 20;
    AxelColumns[4] = AxelColumns[3] + 30;
    AxelColumns[5] = AxelColumns[4] + 30;

    {
        Goal *goal = &Foose.goals[0];
        goal->position.x = 0;
        goal->position.y = (ArenaHeight - GoalHeight) / 2;
    }

    {
        Goal *goal = &Foose.goals[1];
        goal->position.x = ArenaWidth - GoalWidth;
        goal->position.y = (ArenaHeight - GoalHeight) / 2;
    }
    
    {
        Team *team = &Foose.teams[0];
        team->isPlayer = true;
        
        team->color = V4(0.7f, 0.2f, 0.0f, 1.0f);

        {
            Axel *axel = &team->axels[0];
            axel->pieceCount = 3;
            axel->pieces = (Piece*)malloc(sizeof(Piece) * axel->pieceCount);

            int32 offsetY = 20;
            int32 trimmedHeight = ArenaHeight - offsetY;
            int32 strideY = trimmedHeight / 3;
            
            axel->position = V2(AxelColumns[0], offsetY);

            vec2 cursor = V2(0, 0);
            for (int i = 0; i < 3; i++) {
                axel->pieces[i].localPosition = cursor;
                axel->pieces[i].collisionActive = false;
                axel->pieces[i].timeCollisionActivated = 0;

                cursor.y += strideY;
            }
        }

        {
            Axel *axel = &team->axels[1];
            axel->pieceCount = 2;
            axel->pieces = (Piece*)malloc(sizeof(Piece) * axel->pieceCount);

            int32 offsetY = 20;
            int32 trimmedHeight = ArenaHeight - offsetY;
            int32 strideY = trimmedHeight / 2;

            axel->position = V2(AxelColumns[1], offsetY);
            
            vec2 cursor = V2(0);
            for (int i = 0; i < 3; i++) {
                axel->pieces[i].localPosition = cursor;
                axel->pieces[i].collisionActive = false;
                axel->pieces[i].timeCollisionActivated = 0;

                cursor.y += strideY;
            }
        }

        {
            Axel *axel = &team->axels[2];
            axel->pieceCount = 1;
            axel->pieces = (Piece*)malloc(sizeof(Piece) * axel->pieceCount);

            axel->position = V2(AxelColumns[2], ArenaHeight / 2);

            axel->pieces[0].localPosition = V2(0);
            axel->pieces[0].collisionActive = false;
            axel->pieces[0].timeCollisionActivated = 0;

        }
    }

    {
        Team *team = &Foose.teams[1];
        
        team->color = V4(0.0f, 0.2f, 0.7f, 1.0f);

        {
            Axel *axel = &team->axels[0];
            axel->pieceCount = 3;
            axel->pieces = (Piece*)malloc(sizeof(Piece) * axel->pieceCount);

            int32 offsetY = 20;
            int32 trimmedHeight = ArenaHeight - offsetY;
            int32 strideY = trimmedHeight / 3;
            
            axel->position = V2(AxelColumns[3], offsetY);

            vec2 cursor = V2(0, 0);
            for (int i = 0; i < 3; i++) {
                axel->pieces[i].localPosition = cursor;
                axel->pieces[i].collisionActive = false;
                axel->pieces[i].timeCollisionActivated = 0;

                cursor.y += strideY;
            }
        }

        {
            Axel *axel = &team->axels[1];
            axel->pieceCount = 2;
            axel->pieces = (Piece*)malloc(sizeof(Piece) * axel->pieceCount);

            int32 offsetY = 20;
            int32 trimmedHeight = ArenaHeight - offsetY;
            int32 strideY = trimmedHeight / 2;

            axel->position = V2(AxelColumns[4], offsetY);
            
            vec2 cursor = V2(0);
            for (int i = 0; i < 3; i++) {
                axel->pieces[i].localPosition = cursor;
                axel->pieces[i].collisionActive = false;
                axel->pieces[i].timeCollisionActivated = 0;
                
                cursor.y += strideY;
            }
        }

        {
            Axel *axel = &team->axels[2];
            axel->pieceCount = 1;
            axel->pieces = (Piece*)malloc(sizeof(Piece) * axel->pieceCount);

            axel->position = V2(AxelColumns[5], ArenaHeight / 2);

            axel->pieces[0].localPosition = V2(0);
            axel->pieces[0].collisionActive = false;
            axel->pieces[0].timeCollisionActivated = 0;
        }
    }

    StartRound();
}

void DrawPiece(Piece *piece, vec4 color) {
    for (int y = 0; y < PieceSize; y++) {
        for (int x = 0; x < PieceSize; x++) {
            SetTileColor(piece->worldPosition.x + x, piece->worldPosition.y + y, color);
        }
    }
}

void DrawBall(Ball *ball) {
    for (int y = 0; y < BallSize; y++) {
        for (int x = 0; x < BallSize; x++) {
            SetTileColor(ball->position.x + x, ball->position.y + y, V4(0.8f, 0.8f, 0.8f, 1.0f));
        }
    }
}

void DrawGoal(Goal *goal) {
    for (int y = 0; y < GoalHeight; y++) {
        for (int x = 0; x < GoalWidth; x++) {
            SetTileColor(goal->position.x + x, goal->position.y + y, V4(0.3f, 0.3f, 0.3f, 1.0f));
        }
    }
}

void RenderTeam(Team *team) {
    for (int a = 0; a < AxelCount; a++) {
        Axel *axel = &team->axels[a];

        if (team->isPlayer) {
            vec2 textPos = V2(axel->position.x, 0);
            DrawTextTile(textPos, 2.0f, V4(1), axel->string);
        }

        for (int j = 0; j < axel->pieceCount; j++) {
            Piece *piece = &axel->pieces[j];
            vec4 color = team->color;

            if (piece->collisionActive) {
                color = V4(1, 0, 0, 1);
            }
            
            DrawPiece(&axel->pieces[j], color);
        }
    }
}

void MyMosaicUpdate() {
    ClearTiles(0.1f, 0.3f, 0.15f);

    {
        Player *player = &Foose.player;

        // @TODO: if you get a match we should also clear it
        
        if (InputPressed(Keyboard, Input_Return)) {
            for (int i = 0; i < InputLength; i++) {
                player->input[i] = 0;
            }
            
            player->inputIndex = 0;
        }
        else if (InputPressed(Keyboard, Input_Backspace)) {
            player->input[player->inputIndex] = 0;

            if (player->inputIndex > 0) {
                player->inputIndex--;
            }
        }
        else {
            if (Input->charCount > 0) {
                char input = Input->inputChars[0];

                if (player->inputIndex < InputLength) {
                    player->input[player->inputIndex] = input;
                    player->inputIndex++;
                }
            }
        }
    }


    {
        Player *player = &Foose.player;
        Ball *ball = &Foose.ball;
        for (int t = 0; t < 2; t++) {
            Team *team = &Foose.teams[t];

            bool foundMatch = false;
            
            // find which direction we should move to have one of our pieces intercept the ball
            for (int a = 0; a < AxelCount; a++) {
                Axel *axel = &team->axels[a];

                // activate collision
                if (team->isPlayer) {
                    int32 axelStringLength = 0;

                    for (int32 c = 0; c < InputLength; c++) {
                        if (axel->string[c] != 0) {
                            axelStringLength++;
                        }
                    }

                    bool matches = axelStringLength == player->inputIndex;
                                        
                    for (int32 c = 0; c < player->inputIndex; c++) {
                        if (axel->string[c] != player->input[c]) {
                            matches = false;
                        }
                    }

                    if (matches) {
                        for (int32 i = 0; i < axel->pieceCount; i++) {
                            Piece *piece = &axel->pieces[i];
                            piece->collisionActive = true;
                            piece->timeCollisionActivated = Time;
                        }

                        foundMatch = true;

                        GenerateAxelString(axel->string);
                    }
                }

                for (int32 i = 0; i < axel->pieceCount; i++) {
                    Piece *piece = &axel->pieces[i];
                    real32 elapsed = Time - piece->timeCollisionActivated;

                    if (elapsed >= CollisionDuration) {
                        piece->collisionActive = false;
                    }
                }

                int32 closestPieceIndex = -1;
                real32 closestY = 999999;
                real32 closestX = 999999;
                real32 dirY = 0;

                // find which piece is closest to the ball along the y axis and move to get it closer
                for (int32 i = 0; i < axel->pieceCount; i++) {
                    Piece *piece = &axel->pieces[i];

                    real32 yDiff = ball->position.y - piece->worldPosition.y;
                    real32 xDiff = ball->position.x - piece->worldPosition.x;

                    if (Abs(yDiff) < closestY) {
                        closestY = Abs(yDiff);

                        if (yDiff < -0.5f) {
                            dirY = -1;
                        }
                        else if (yDiff > 0.5f) {
                            dirY = 1;
                        }

                        closestPieceIndex = i;
                    }

                    // don't check abs because we care about 
                    if (xDiff < closestX) {
                        closestX = xDiff;
                    }
                }

                Print("Closest piece %d", closestPieceIndex);

                if (!team->isPlayer) {
                    if (closestX < 0 && closestX > -5) {
                        real32 cooldown = 2.0f;
                        real32 timeSince = Time - axel->pieces[0].timeCollisionActivated;

                        if (timeSince >= cooldown) {
                            for (int32 i = 0; i < axel->pieceCount; i++) {
                                Piece *piece = &axel->pieces[i];
                                piece->collisionActive = true;
                                piece->timeCollisionActivated = Time;
                            }
                        }
                    }
                }

                real32 speed = 12;

                if (!team->isPlayer) {
                    speed = 6 + (4 * ((sinf(Time * 0.5f) + 1) * 0.5f));
                }

                axel->velocity = V2(0, dirY * speed);
                axel->position.y += axel->velocity.y * DeltaTime;

                //axel->position.y = 40 + (5 * sinf(Time));

                vec2 axelMin = V2(INFINITY);
                vec2 axelMax = V2(-INFINITY);
                
                for (int32 i = 0; i < axel->pieceCount; i++) {
                    Piece *piece = &axel->pieces[i];
                    piece->worldPosition = piece->localPosition + axel->position;

                    vec2 pieceMax = piece->worldPosition + V2(PieceSize);

                    axelMin = Min(piece->worldPosition, axelMin);
                    axelMax = Max(pieceMax, axelMax);
                }

                // resolve collision with the top of the field
                real32 borderOverlap = 0;
                if (axelMin.y < 0) {
                    borderOverlap = axelMin.y;
                }
                else if (axelMax.y >= ArenaHeight) {
                    borderOverlap = axelMax.y - ArenaHeight;
                }

                if (borderOverlap != 0) {
                    axel->position.y -= borderOverlap;
                    axel->position.y = roundf(axel->position.y);
                }

                vec2 axelRounded = axel->position;

                // update the worldPositionof the pieces if the axel has been moved
                for (int32 i = 0; i < axel->pieceCount; i++) {
                    Piece *piece = &axel->pieces[i];
                    piece->worldPosition = piece->localPosition + axelRounded;
                }
            }

            if (foundMatch) {
                // reset input for everything
                for (int i = 0; i < InputLength; i++) {
                    player->input[i] = 0;
                }
            
                player->inputIndex = 0;

                // @TODO: regenerate the strings for axels
            }
        }
    }

    {
        Ball *ball = &Foose.ball;

        real32 damping = 0.2;
        ball->velocity = ball->velocity - (ball->velocity * damping * DeltaTime);

        real32 speed = Length(ball->velocity);

        // so our ball never slows to a complete stop.
        if (speed < BallMinSpeed) {
            ball->velocity = Normalize(ball->velocity) * BallMinSpeed;
        }
        
        ball->position = ball->position + ball->velocity * DeltaTime;

        // check for collisions
        vec2 min = ball->position;
        vec2 max = ball->position + V2(BallSize);

        if (min.y < 0) {
            ball->velocity.y *= -1;

            ball->position.y += (0 - min.y);
        }
        else if (max.y >= ArenaHeight) {
            ball->velocity.y *= -1;

            ball->position.y += (ArenaHeight - max.y);
        }
        else if (min.x < 0) {
            ball->velocity.x *= -1;

            ball->position.x += (0 - min.x);
        }
        else if (max.x >= ArenaWidth) {
            ball->velocity.x *= -1;

            ball->position.x += (ArenaWidth - max.x);
        }

        // check collision against player
        for (int t = 0; t < 2; t++) {
            Team *team = &Foose.teams[t];

            // find which direction we should move to have one of our pieces intercept the ball
            for (int a = 0; a < AxelCount; a++) {
                Axel *axel = &team->axels[a];

                for (int32 i = 0; i < axel->pieceCount; i++) {
                    Piece *piece = &axel->pieces[i];

                    vec2 pieceMin = piece->worldPosition;
                    vec2 pieceMax = piece->worldPosition + V2(PieceSize);

                    if (!piece->collisionActive) { continue; }

                    vec2 dir;
                    if (TestAABBAABB(min, max, pieceMin, pieceMax, &dir)) {
                        ball->position = ball->position + dir;

                        real32 force = 12.0f;
                        if (dir.x < 0) {
                            ball->velocity.x *= -1;
                        }
                        else if (dir.x > 0) {
                            ball->velocity.x *= -1;
                        }
                        else if (dir.y < 0) {
                            ball->velocity.y *= -1;
                        }
                        else if (dir.x > 0) {
                            ball->velocity.y *= -1;
                        }

                        // compare the centers to each other
                        vec2 playerCenter = Lerp(pieceMin, pieceMax, 0.5f);
                        vec2 ballCenter = Lerp(min, max, 0.5f);

                        vec2 kickDir = Normalize(ballCenter - playerCenter);

                        ball->velocity = ball->velocity + (kickDir * force);
                    }
                }
            }
        }

        for (int i = 0; i < 2; i++) {
            Goal *goal = &Foose.goals[i];

            vec2 goalMin = goal->position;
            vec2 goalMax = goalMin + V2(GoalWidth, GoalHeight);

            vec2 dir;
            if (TestAABBAABB(min, max, goalMin, goalMax, &dir)) {
                if (i == 0) {
                    Foose.teams[1].score++;
                }
                else {
                    Foose.teams[0].score++;
                }

                StartRound();
            }
        }
    }

    {
        Player *player = &Foose.player;
        DrawTextTile(V2(ArenaWidth / 2, ArenaHeight + 1), 5.0f, V4(1), false, "%.*s", player->inputIndex, player->input);
    }

    RenderTeam(&Foose.teams[0]);
    RenderTeam(&Foose.teams[1]);
    
    DrawGoal(&Foose.goals[0]);
    DrawGoal(&Foose.goals[1]);

    DrawTextTile(V2(ArenaWidth / 2, ArenaHeight + 3), 2.0f, V4(1), true, "Score %d | %d", Foose.teams[0].score, Foose.teams[1].score);

    //RenderTeam(&Foose.teams[1]);

    DrawBall(&Foose.ball);
}
