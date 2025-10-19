#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define GRID_SIZE 15
#define CELL_SIZE (SCREEN_WIDTH / GRID_SIZE)


int BLUE_PATH[][2] = {
    {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}, {7, 7}};


int RED_PATH[][2] = {
    {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {7, 13}, {7, 12}, {7, 11}, {7, 10}, {7, 9}, {7, 8}};


int GREEN_PATH[][2] = {
    {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7}, {8, 7}};


int YELLOW_PATH[][2] = {
    {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 14}, {7, 5}, {7, 6}, {7, 7}};


typedef struct
{
    int playerID;
    SDL_Renderer *renderer;
} RollDiceArgs;


const SDL_Color WHITE = {255, 255, 255, 255};
const SDL_Color RED = {255, 0, 0, 255};
const SDL_Color GREEN = {0, 255, 0, 255};
const SDL_Color BLUE = {0, 0, 255, 255};
const SDL_Color YELLOW = {255, 255, 0, 255};
const SDL_Color LIGHTBLUE = {173, 216, 230, 255};   // Lighter blue
const SDL_Color LIGHTGREEN = {144, 238, 144, 255};  // Lighter green
const SDL_Color LIGHTRED = {255, 182, 193, 255};    // Lighter red (light pink)
const SDL_Color LIGHTYELLOW = {125, 255, 204, 255}; // Lighter yellow (light cream)


const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color GRAY = {200, 190, 200, 255}; // Safe zones


// Mutexes for thread safety
pthread_mutex_t gridMutex;
pthread_mutex_t diceMutex;


// Semaphore for token management (1 for each player)
sem_t tokenSemaphore[4];


// Number of tokens for each player
int tokens[4];
int tokenPositions[4][4] = {{-1, -1, -1, -1}, {-1, -1, -1, -1}, {-1, -1, -1, -1}, {-1, -1, -1, -1}};
int lastRoll[4] = {0, 0, 0, 0}; // Tracks last dice roll for each player


int pathLengths[] = {sizeof(BLUE_PATH) / sizeof(BLUE_PATH[0]),
                     sizeof(RED_PATH) / sizeof(RED_PATH[0]),
                     sizeof(GREEN_PATH) / sizeof(GREEN_PATH[0]),
                     sizeof(YELLOW_PATH) / sizeof(YELLOW_PATH[0])};


void drawRect(SDL_Renderer *renderer, int x, int y, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderDrawRect(renderer, &rect);
}


void drawBoard(SDL_Renderer *renderer)
{
    pthread_mutex_lock(&gridMutex); // Lock the grid resource


    // Draw the board grid
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            drawRect(renderer, i, j, WHITE);
        }
    }


    // Home zones (6x6)
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            drawRect(renderer, i, j, BLUE);                                  // Blue home (top-left)
            drawRect(renderer, i, GRID_SIZE - 6 + j, RED);                   // Red home (bottom-left)
            drawRect(renderer, GRID_SIZE - 6 + i, j, YELLOW);                // Yellow home (top-right)
            drawRect(renderer, GRID_SIZE - 6 + i, GRID_SIZE - 6 + j, GREEN); // Green home (bottom-right)
        }
    }


    // Full middle pathways for each player's home path
    for (int i = 0; i < 6; i++)
    {
        drawRect(renderer, 7, 1 + i, YELLOW); // Yellow path (vertical)
        drawRect(renderer, 1 + i, 7, BLUE);   // Blue path (horizontal)
        drawRect(renderer, 7, 8 + i, RED);    // Red path (vertical)
        drawRect(renderer, 8 + i, 7, GREEN);  // Green path (horizontal)
    }


    // Safe zones before entering home (4 positions for each color)
    drawRect(renderer, 12, 6, GRAY); // Green safe zone
    drawRect(renderer, 2, 8, GRAY);  // Blue safe zone
    drawRect(renderer, 6, 2, GRAY);  // Yellow safe zone
    drawRect(renderer, 8, 12, GRAY); // Red safe zone


    // Starting positions for tokens
    drawRect(renderer, 8, 1, YELLOW); // Yellow start
    drawRect(renderer, 6, 13, RED);   // Red start
    drawRect(renderer, 1, 6, BLUE);   // Blue start
    drawRect(renderer, 13, 8, GREEN); // Green start


    pthread_mutex_unlock(&gridMutex); // Unlock the grid resource
}
void drawTokens(SDL_Renderer *renderer)
{
    for (int playerID = 0; playerID < 4; playerID++)
    {
        for (int tokenID = 0; tokenID < tokens[playerID]; tokenID++)
        {
            SDL_Color tokenColor;
            switch (playerID)
            {
            case 0:
                tokenColor = LIGHTBLUE;
                break;
            case 1:
                tokenColor = LIGHTYELLOW;
                break;
            case 2:
                tokenColor = LIGHTGREEN;
                break;
            case 3:
                tokenColor = LIGHTRED;
                break;
            }


            if (tokenPositions[playerID][tokenID] == -1) // Token is in the home area
            {
                int homeX = 0, homeY = 0;


                // Determine the center positions in the home area for each player
                switch (playerID)
                {
                case 0:                        // Blue (top-left)
                    homeX = 2 + (tokenID % 2); // Center columns: 2, 3
                    homeY = 2 + (tokenID / 2); // Center rows: 2, 3
                    break;
                case 1:                                        // Yellow (top-right)
                    homeX = GRID_SIZE - 6 + 2 + (tokenID % 2); // Center columns: GRID_SIZE-4, GRID_SIZE-3
                    homeY = 2 + (tokenID / 2);                 // Center rows: 2, 3
                    break;
                case 2:                                        // Green (bottom-right)
                    homeX = GRID_SIZE - 6 + 2 + (tokenID % 2); // Center columns: GRID_SIZE-4, GRID_SIZE-3
                    homeY = GRID_SIZE - 6 + 2 + (tokenID / 2); // Center rows: GRID_SIZE-4, GRID_SIZE-3
                    break;
                case 3:                                        // Red (bottom-left)
                    homeX = 2 + (tokenID % 2);                 // Center columns: 2, 3
                    homeY = GRID_SIZE - 6 + 2 + (tokenID / 2); // Center rows: GRID_SIZE-4, GRID_SIZE-3
                    break;
                }


                drawRect(renderer, homeX, homeY, tokenColor); // Draw token in home area
            }
            else if (tokenPositions[playerID][tokenID] >= 0) // Token is on the board
            {
                int posIndex = tokenPositions[playerID][tokenID];
                if (posIndex >= pathLengths[playerID])
                {
                    printf("Skipping invalid position for Player %d, Token %d\n", playerID, tokenID);
                    continue;
                }


                // Get token coordinates based on the player's path
                int x = 0, y = 0;
                switch (playerID)
                {
                case 0: // Blue
                    x = BLUE_PATH[posIndex][0];
                    y = BLUE_PATH[posIndex][1];
                    break;
                case 1: // Yellow
                    x = YELLOW_PATH[posIndex][0];
                    y = YELLOW_PATH[posIndex][1];
                    break;
                case 2: // Green
                    x = GREEN_PATH[posIndex][0];
                    y = GREEN_PATH[posIndex][1];
                    break;
                case 3: // Red
                    x = RED_PATH[posIndex][0];
                    y = RED_PATH[posIndex][1];
                    break;
                }


                drawRect(renderer, x, y, tokenColor); // Draw the token with the player's color
            }
        }
    }
}


void checkWinCondition()
{
    for (int playerID = 0; playerID < 4; playerID++)
    {
        bool allTokensHome = true;
        for (int i = 0; i < tokens[playerID]; i++)
        {
            if (tokenPositions[playerID][i] != pathLengths[playerID] - 1)
            {
                allTokensHome = false;
                break;
            }
        }


        if (allTokensHome)
        {
            printf("Player %d wins the game!\n", playerID + 1);
            exit(0); // Exit the game
        }
    }
}


void animateMovement(SDL_Renderer *renderer, int playerID, int diceValue)
{


    for (int i = 0; i < tokens[playerID]; i++)
    {
        if (tokenPositions[playerID][i] != -1) // Token is on the board
        {
            int currentPos = tokenPositions[playerID][i];


            int newPosition = currentPos + diceValue;


            // Ensure the token doesn't exceed the path length
            if (newPosition >= pathLengths[playerID])
            {
                newPosition = pathLengths[playerID] - 1; // Reached the end of the path
            }


            // Animate the movement step-by-step
            for (int step = currentPos + 1; step <= newPosition; step++)
            {
                // tokenPositions[playerID][i] = step;


                // Redraw the board and tokens
                SDL_RenderClear(renderer);
                drawBoard(renderer);
                drawTokens(renderer);
                SDL_RenderPresent(renderer);


                // Delay for animation effect
                SDL_Delay(200); // Delay between each step of the token movement
            }
        }
    }
}
// Add a function to check if a player has won
bool checkForWinner(int playerID)
{
    // Check if all tokens of the player have reached the home position
    for (int i = 0; i < tokens[playerID]; i++)
    {
        if (tokenPositions[playerID][i] != pathLengths[playerID] - 1)
        {
            return false; // At least one token hasn't reached home
        }
    }
    return true; // All tokens have reached home
}


void moveToken(SDL_Renderer *renderer, int playerID, int diceValue)
{
    if (!renderer)
    {
        fprintf(stderr, "Error: SDL renderer is null.\n");
        exit(1);
    }


    // Safe zones positions for each player color (adjust these positions accordingly)
    int safeZones[] = {12, 2, 6, 8}; // These positions are considered safe


    // Track if a move has been made
    bool moveMade = false;
    bool tokenBeaten = false; // Track if the token has beaten an opponent's token


    // Attempt to move a token out of the home area if possible
    for (int i = 0; i < tokens[playerID]; i++)
    {
        if (tokenPositions[playerID][i] == -1 && diceValue == 6)
        {
            printf("Player %d, Token %d is entering the board.\n", playerID + 1, i);
            tokenPositions[playerID][i] = 0;        // Move to starting position
            animateMovement(renderer, playerID, 0); // Animate movement from start
            moveMade = true;                        // Move is made
            break;                                  // Stop after moving one token
        }
    }


    if (!moveMade)
    {
        // Check for tokens that are not in safe zones or already finished
        for (int i = 0; i < tokens[playerID]; i++)
        {
            if (tokenPositions[playerID][i] != -1 && tokenPositions[playerID][i] < pathLengths[playerID] - 1)
            { // Token is in play and hasn't finished
                int currentPos = tokenPositions[playerID][i];
                int newPosition = currentPos + diceValue;


                // Ensure the token doesn't move beyond the path length
                if (newPosition >= pathLengths[playerID])
                {
                    newPosition = pathLengths[playerID] - 1; // Stop at the end of the path
                }


                // Check if the token is in a safe zone (prevent movement if so)
                bool inSafeZone = false;
                for (int j = 0; j < sizeof(safeZones) / sizeof(safeZones[0]); j++)
                {
                    if (currentPos == safeZones[j])
                    {
                        inSafeZone = true;
                        break;
                    }
                }


                // If the token is in a safe zone, skip it
                if (inSafeZone)
                {
                    printf("Player %d's Token %d is in a safe zone at position %d.\n", playerID + 1, i, currentPos);
                    continue; // Skip this token, move to the next one
                }


                // Check if the new position is occupied by another player's token
                for (int j = 0; j < 4; j++)
                {
                    if (j != playerID)
                    { // Skip the current player
                        for (int k = 0; k < tokens[j]; k++)
                        {
                            if (tokenPositions[j][k] == newPosition)
                            {
                                // Beat the opponent's token back to its base
                                tokenPositions[j][k] = -1; // Opponent's token is sent back to their yard
                                printf("Player %d's Token %d was beaten by Player %d and sent back to their yard.\n", j + 1, k, playerID + 1);
                                tokenBeaten = true; // The token has beaten an opponent's token
                            }
                        }
                    }
                }


                // Move the token to the new position
                printf("Player %d, Token %d moves from %d to %d.\n", playerID + 1, i, currentPos, newPosition);
                tokenPositions[playerID][i] = newPosition;


                // Animate the movement for this token only once after it reaches the final position
                animateMovement(renderer, playerID, diceValue);
                moveMade = true; // Move is made
                break;           // Stop after moving one token
            }
        }
    }


    // If no token has moved yet and the player has no valid token to move, attempt to move a token in a safe zone
    if (!moveMade)
    {
        for (int i = 0; i < tokens[playerID]; i++)
        {
            if (tokenPositions[playerID][i] != -1 && tokenPositions[playerID][i] < pathLengths[playerID] - 1)
            {
                int currentPos = tokenPositions[playerID][i];


                // Check if the token is in a safe zone and move it
                for (int j = 0; j < sizeof(safeZones) / sizeof(safeZones[0]); j++)
                {
                    if (currentPos == safeZones[j])
                    {
                        printf("Player %d's Token %d is in a safe zone at position %d and can move.\n", playerID + 1, i, currentPos);
                        tokenPositions[playerID][i] = currentPos + diceValue;


                        animateMovement(renderer, playerID, diceValue);
                        moveMade = true; // Move is made
                        break;
                    }
                }
            }


            if (moveMade)
            {
                break; // Stop after moving one token
            }
        }
    }


    // If no token has beaten an opponent's token, and the player tries to enter the home, reset the token
    if (!tokenBeaten)
    {
        for (int i = 0; i < tokens[playerID]; i++)
        {
            if (tokenPositions[playerID][i] == pathLengths[playerID] - 1)
            {
                printf("Player %d's Token %d failed to beat an opponent's token, restarting path.\n", playerID + 1, i);
                tokenPositions[playerID][i] = -1; // Reset the token
            }
        }
    }


    // Check if all tokens for a player have completed the path and declare winner
    bool allTokensFinished = true;
    for (int i = 0; i < tokens[playerID]; i++)
    {
        if (tokenPositions[playerID][i] < pathLengths[playerID] - 1)
        {
            allTokensFinished = false;
            break;
        }
    }


    if (allTokensFinished)
    {
        printf("Player %d wins! All tokens have completed the path.\n", playerID + 1);
    }


    // No tokens could be moved
    if (!moveMade)
    {
        printf("Player %d has no valid tokens to move.\n", playerID + 1);
    }
}


// Modify rollDice to handle the player's turn and dice roll
void *rollDice(void *arg)
{
    RollDiceArgs *args = (RollDiceArgs *)arg;
    int playerID = args->playerID;
    SDL_Renderer *renderer = args->renderer;


    int rollCount = 0;
    int diceValue = 0;
    int consecutiveSixes = 0;


    // Wait for the player's turn (semaphore)
    sem_wait(&tokenSemaphore[playerID]);


    pthread_mutex_lock(&diceMutex); // Lock the dice resource


    printf("Player %d is rolling the dice...\n", playerID + 1);


    do
    {
        diceValue = (rand() % 6) + 1; // Roll dice between 1 and 6
        printf("Player %d rolled: %d\n", playerID + 1, diceValue);


        if (diceValue == 6)
        {
            consecutiveSixes++;
            if (consecutiveSixes == 3)
            {
                printf("Player %d rolled three consecutive 6s, skipping turn.\n", playerID + 1);
                break; // Skip turn if 3 consecutive 6s are rolled
            }
        }
        else
        {
            consecutiveSixes = 0; // Reset consecutive sixes count
        }


        // Move token if less than 3 consecutive 6s
        if (consecutiveSixes < 3)
        {
            moveToken(renderer, playerID, diceValue);
        }
        if (diceValue != 6)
        {
            break;
        }
        // If dice value is not 6, break the loop (no bonus roll)


        rollCount++; // Increment roll count for consecutive rolls


    } while (rollCount < 3); // Allow up to 3 rolls


    pthread_mutex_unlock(&diceMutex); // Unlock the dice resource


    // Signal that the player's turn is done
    sem_post(&tokenSemaphore[playerID]);


    return NULL;
}


void shuffle(int *array, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}
int main(int argc, char *argv[])
{
    // Initialize mutexes
    pthread_mutex_init(&gridMutex, NULL);
    pthread_mutex_init(&diceMutex, NULL);


    // Initialize semaphores for each player
    for (int i = 0; i < 4; i++)
    {
        sem_init(&tokenSemaphore[i], 0, 1); // 1 token per player
    }


    // Get user input for number of tokens (between 1 and 4)
    int numTokens;
    printf("Enter number of tokens per player (1-4): ");
    scanf("%d", &numTokens);


    for (int i = 0; i < 4; i++)
    {
        tokens[i] = numTokens;
    }


    // SDL setup code for window, renderer, etc.


    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Ludo Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL renderer: %s\n", SDL_GetError());
        exit(1);
    }


    // Main game loop (simplified)
    SDL_Event e;
    int quit = 0;


    // Start game with player turn rolls
    pthread_t threads[4];
    int playerIDs[4] = {0, 1, 2, 3};


    while (!quit)
    {
        for (int currentPlayer = 0; currentPlayer < 4; currentPlayer++)
        {
            // Wait for quit event
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = 1;
                    break;
                }
            }
            if (quit)
                break;
            RollDiceArgs args;
            args.playerID = playerIDs[currentPlayer]; // Set the playerID
            args.renderer = renderer;
            // Roll dice for the current player
            pthread_create(&threads[currentPlayer], NULL, rollDice, &args);
            pthread_join(threads[currentPlayer], NULL); // Wait for the dice roll to complete


            // Redraw the board and tokens
            SDL_RenderClear(renderer);
            drawBoard(renderer);
            drawTokens(renderer);
            SDL_RenderPresent(renderer);
            checkWinCondition();
            // Add a small delay to visualize transitions
            SDL_Delay(1000);
        }
    }


    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}





