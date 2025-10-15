#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>

using namespace std;

const int Width = 70;
const int Height = 20;

// function to set cursor position
void gotoxy(int x, int y) 
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// function to hide the cursor
void hideCursor() 
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// class game 
class Game 
{
    private:
        vector<string> buffer; // Buffer for the map
        int playerX; // player x position 
        int playerY; // player y position
        int speed; // player speed
        bool playing; // is the game playing

        // variables for jumping mechanics
        float jumpVelocity;       // jump velocity
        bool onGround;             // is the player on the ground?
        bool canJump;              // can the player keep jumping?
        int framesJumping;        // how many frames the player has been jumping

        const float GRAVITY = 0.9f;           // gravity force (increased to fall faster)
        const float INITIAL_JUMP_IMPULSE = -3.5f;   // initial jump impulse
        const float MAINTAINED_JUMP_IMPULSE = -0.5f; // impulse while holding the key (reduced)
        const int MAX_JUMP_FRAMES = 8;        // maximum frames the jump can be maintained (reduced)

    public:
        // builder
        Game() 
        {
            speed = 2; // Initial speed: normal
            playing = true;
            buffer.resize(Height, string(Width, ' '));

            // Initialize the map with platforms
            initializeMap();

            // Initial player position
            playerX = 10;
            playerY = Height - 2; // Start on the ground

            // Initialize jump variables
            jumpVelocity = 0;
            onGround = true;
            canJump = false;
            framesJumping = 0;
        }

        // Initialize the map with platforms
        void initializeMap() {
            // Clear the buffer
            for (int y = 0; y < Height; y++) 
            {
                for (int x = 0; x < Width; x++) buffer[y][x] = ' ';
            }
            
            // draw borders
            for (int x = 0; x < Width; x++) 
            {
                buffer[0][x] = '#';           // 
                buffer[Height - 1][x] = '#';    // floor
            }
            for (int y = 0; y < Height; y++) 
            {
                buffer[y][0] = '#';           // wall left
                buffer[y][Width - 1] = '#';   // wall right
            }

            // Create custom platforms
            for (int x = 5; x <= 10; x++) buffer[Height - 5][x] = '#';
            
            for (int x = 15; x <= 22; x++) buffer[Height - 8][x] = '#';
            
            for (int x = 30; x <= 36; x++) buffer[Height - 12][x] = '#';
            
            for (int x = 40; x <= 44; x++) buffer[Height - 6][x] = '#';
        }

        // Function to draw the map
        void drawMap() {

            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++) 
                {
                    if (buffer[y][x] == '@') buffer[y][x] = ' ';
                }
            }

            // Check if the player's position is valid before drawing
            if (playerX >= 0 && playerX < Width && playerY >= 0 && playerY < Height) 
            {
                buffer[playerY][playerX] = '@';
            }

            // Print the complete buffer
            for(int i=0; i<Height; i++) 
            {
                gotoxy(0, i);
                cout << buffer[i];
            }
            gotoxy(0, Height);
            cout << "A (izquierda) | D (derecha) | ESPACIO (Saltar)";
            gotoxy(0, Height + 1);
            cout << "Shift Izq (Lento) | Ctrl Izq (Rapido)         ";
            gotoxy(0, Height + 2);
            cout << "Presiona ESC para salir                       ";
        }
        

        // Check if there is a solid block (#) at a position
        bool isSolid(int x, int y) 
        {
            if (x < 0 || x >= Width || y < 0 || y >= Height) return true; // Out of bounds is considered solid
            return buffer[y][x] == '#';
        }
        
        // input
        void input() {
            // Detect Shift (slow) and Ctrl (fast) keys
            int teclaShift = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) ? 1 : 0;
            int teclaCtrl = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) ? 1 : 0;
            // calculate speed
            speed = (teclaCtrl - teclaShift) + 2;
            
            // Detect movement keys
            int teclaA = (GetAsyncKeyState('A') & 0x8000) ? 1 : 0;
            int teclaD = (GetAsyncKeyState('D') & 0x8000) ? 1 : 0;

            // Calculate movement
            int mov = (teclaD - teclaA) * speed;

            // Move player horizontally
            movplayer(mov);
            
            // Jump
            Jump();
            
            // gravity
            Gravity();
            
            // exit
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) playing = false;
        }

        // process jump
        void Jump() {
            bool spacePressed = GetAsyncKeyState(VK_SPACE) & 0x8000;
            
            // start jump
            if (onGround && spacePressed) 
            {
                jumpVelocity = INITIAL_JUMP_IMPULSE;
                onGround = false;
                canJump = true;
                framesJumping = 0;
            }
            // extend jump
            else if (!onGround && spacePressed && canJump) 
            {
                framesJumping++;

                // Only apply impulse if going up and haven't exceeded the limit
                if(jumpVelocity < 0 && framesJumping < MAX_JUMP_FRAMES) 
                {
                    jumpVelocity += MAINTAINED_JUMP_IMPULSE;
                }
                else canJump = false; // Can't jump anymore
            }
            // If space is released, cancel additional impulse
            else if (!spacePressed && !onGround) canJump = false;
        }

        // Apply gravity and ground collision
        void Gravity() {
            // Apply gravity if not on the ground
            if (!onGround) 
            {
                jumpVelocity += GRAVITY;
                // Limit maximum falling speed
                //if (jumpVelocity > 10) jumpVelocity = 10;
            }

            // If falling
            if (jumpVelocity > 0) {
                // Check for ground collision
                for (int step = 0; step < (int)jumpVelocity; step++) 
                {
                    // Check if there is ground in the next position
                    if (playerY + 1 >= Height || isSolid(playerX, playerY + 1)) 
                    {
                        // There is ground below, stop
                        jumpVelocity = 0;
                        onGround = true;
                        return;
                    }
                    // Move down one pixel
                    playerY++;
                }
                onGround = false;
            }
            // If rising
            else if (jumpVelocity < 0) 
            {
                // Check pixel by pixel upwards
                int steps = (int)(-jumpVelocity);
                for (int step = 0; step < steps; step++) 
                {
                    // Check if there is a ceiling above
                    if (playerY - 1 <= 0 || isSolid(playerX, playerY - 1)) 
                    {
                        // Hit the ceiling, stop vertical movement
                        jumpVelocity = 0;
                        onGround = false;
                        return;
                    }
                    // Move up one pixel
                    playerY--;
                }
                onGround = false;
            }
            // If velocity is 0
            else 
            {
                // Check if there is still ground below
                if (playerY + 1 < Height && !isSolid(playerX, playerY + 1)) onGround = false; 
                else onGround = true;
            }
        }

        // Move player
        void movplayer(int mov) {
            if (mov == 0) return; // No movement

            // Calculate new position
            int newX = playerX + mov;

            // Check for collision with solid blocks
            if (!isSolid(newX, playerY)) playerX = newX;
            // If there is a collision, don't move
        }

        // Initialize the game
        void start() 
        {
            hideCursor();
            system("cls");
        }

        // Main game loop
        void run() 
        {
            while (playing) 
            {
                input();
                drawMap();
                Sleep(50); // Control game speed
            }
        }
        
        // Finalizar el juego
        void end() 
        {
            system("cls");
            cout << "Game over push any key to continue..." << endl;
            _getch();
        }
};


int main() 
{
    Game game;

    game.start();
    game.run();
    game.end();

    return 0;
}
