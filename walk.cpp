#include <iostream>
#include <cmath>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

using namespace std;

const int Width = 30;
const int Height = 20;
const double PI = 3.14159265359;
const double FOV = 120.0; // Field of view in degrees

// Forward declaration
class Map;

class Player
{
    public:
        double x, y;
        double angle; // Direction in degrees (0 = right, 90 = down, 180 = left, 270 = up)
        
        Player(double startX, double startY) : x(startX), y(startY), angle(0.0) {}
        
        void rotate(double deltaAngle)
        {
            angle += deltaAngle;
            if (angle < 0) angle += 360;
            if (angle >= 360) angle -= 360;
        }
        
        void move(double distance, Map& gameMap);
};

class Map
{
    private:
        char map[Height][Width];
        bool visible[Height][Width];
    public:
        Map()
        {
            initizeMap();
        }

        void initizeMap()
        {
            for (int i = 0; i < Height; i++)
            {
                for (int j = 0; j < Width; j++)
                {
                    if(i == 0 || i == Height - 1 || j == 0 || j == Width - 1)
                    {    map[i][j] = '#';}
                    else
                    {    map[i][j] = '.';}
                }
            }
            
            // Create a more interesting map with multiple structures
            
            // Central column (vertical wall)
            for (int i = 7; i <= 12; i++)
            {
                map[i][15] = '#';
            }
            
            // Left room walls
            for (int j = 5; j <= 10; j++)
            {
                map[8][j] = '#';
                map[12][j] = '#';
            }
            map[9][5] = '#';
            map[10][5] = '#';
            map[11][5] = '#';
            
            // Right room walls
            for (int j = 20; j <= 25; j++)
            {
                map[8][j] = '#';
                map[12][j] = '#';
            }
            map[9][25] = '#';
            map[10][25] = '#';
            map[11][25] = '#';
            
            // Scattered obstacles
            map[4][7] = '#';
            map[4][8] = '#';
            map[15][7] = '#';
            map[15][8] = '#';
            
            map[4][22] = '#';
            map[4][23] = '#';
            map[15][22] = '#';
            map[15][23] = '#';
            
            // Small pillars
            map[6][12] = '#';
            map[6][18] = '#';
            map[14][12] = '#';
            map[14][18] = '#';
        }
        
        bool isInFOV(double playerX, double playerY, double playerAngle, int targetX, int targetY)
        {
            double dx = targetX - playerX;
            double dy = targetY - playerY;
            // convertimos a grados
            double angleToTarget = atan2(dy, dx) * 180.0 / PI;
            // Normalize angles
            if (angleToTarget < 0) angleToTarget += 360;
            // calculate angle difference
            double angleDiff = angleToTarget - playerAngle;
            if (angleDiff < -180) angleDiff += 360;
            if (angleDiff > 180) angleDiff -= 360;
            // check if within FOV
            return fabs(angleDiff) <= FOV / 2.0;
        }
        
        bool hasLineOfSight(double x1, double y1, int x2, int y2)
        {
            // Bresenham's line algorithm with continuous start point
            double dx = x2 - x1;
            double dy = y2 - y1;
            double distance = sqrt(dx * dx + dy * dy);
            
            if (distance < 0.01) return true;
            
            int steps = (int)(distance * 2) + 1;
            double stepX = dx / steps;
            double stepY = dy / steps;
            
            for (int i = 1; i < steps; i++)
            {
                double checkX = x1 + stepX * i;
                double checkY = y1 + stepY * i;
                int gridX = (int)round(checkX);
                int gridY = (int)round(checkY);
                
                if (gridX >= 0 && gridX < Width && gridY >= 0 && gridY < Height)
                {
                    if (map[gridY][gridX] == '#')
                    {
                        return false;
                    }
                }
            }
            return true;
        }
        
        void calculateVisibility(Player& player)
        {
            // Reset visibility
            for (int i = 0; i < Height; i++)
            {
                for (int j = 0; j < Width; j++)
                {
                    visible[i][j] = false;
                }
            }
            
            // Check each cell
            for (int i = 0; i < Height; i++)
            {
                for (int j = 0; j < Width; j++)
                {
                    if (isInFOV(player.x, player.y, player.angle, j, i))
                    {
                        if (hasLineOfSight(player.x, player.y, j, i))
                        {
                            visible[i][j] = true;
                        }
                    }
                }
            }
        }

        char getDirectionChar(double angle)
        {
            // Normalize angle to 0-360
            while (angle < 0) angle += 360;
            while (angle >= 360) angle -= 360;
            
            // Return character based on direction
            if (angle >= 337.5 || angle < 22.5) return '>';      // Right
            else if (angle >= 22.5 && angle < 67.5) return '\\';  // Down-Right
            else if (angle >= 67.5 && angle < 112.5) return 'v';  // Down
            else if (angle >= 112.5 && angle < 157.5) return '/'; // Down-Left
            else if (angle >= 157.5 && angle < 202.5) return '<'; // Left
            else if (angle >= 202.5 && angle < 247.5) return '/'; // Up-Left
            else if (angle >= 247.5 && angle < 292.5) return '^'; // Up
            else return '\\'; // Up-Right
        }
        
        void displayMap(Player& player)
        {
            calculateVisibility(player);
            
            // Clear screen and move cursor to top
            cout << "\033[2J\033[H";
            
            for (int i = 0; i < Height; i++)
            {
                for (int j = 0; j < Width; j++)
                {
                    int playerGridX = (int)round(player.x);
                    int playerGridY = (int)round(player.y);
                    
                    if (j == playerGridX && i == playerGridY)
                    {
                        // Show player with direction indicator
                        cout << getDirectionChar(player.angle);
                    }
                    else if (visible[i][j])
                    {
                        // Show visible tiles
                        cout << map[i][j];
                    }
                    else if (map[i][j] == '#')
                    {
                        // Show walls/obstacles even if not directly visible (but dimmed)
                        // Check if it's adjacent to visible area
                        bool nearVisible = false;
                        for (int di = -1; di <= 1 && !nearVisible; di++)
                        {
                            for (int dj = -1; dj <= 1 && !nearVisible; dj++)
                            {
                                int ni = i + di;
                                int nj = j + dj;
                                if (ni >= 0 && ni < Height && nj >= 0 && nj < Width)
                                {
                                    if (visible[ni][nj])
                                    {
                                        nearVisible = true;
                                    }
                                }
                            }
                        }
                        
                        if (nearVisible)
                        {
                            cout << '#'; // Show walls adjacent to visible areas
                        }
                        else
                        {
                            cout << ' '; // Hide far away walls
                        }
                    }
                    else
                    {
                        cout << ' '; // Empty space for non-visible areas
                    }
                }
                cout << endl;
            }
            
            // Display info with direction indicator
            cout << "\nPosition: (" << (int)round(player.x) << ", " << (int)round(player.y) << ")";
            cout << " | Facing: " << (int)player.angle << " degrees " << getDirectionChar(player.angle);
            cout << "\nControls: W/S=Forward/Back | A/D=Rotate | Q=Quit" << endl;
            cout << "FOV: 120 degrees | Vision blocked by walls (#)" << endl;
        }
        
        char getCell(int x, int y)
        {
            if (x >= 0 && x < Width && y >= 0 && y < Height)
                return map[y][x];
            return '#';
        }
};

// Player move implementation (after Map class is defined)
void Player::move(double distance, Map& gameMap)
{
    // Calculate new position based on angle
    double radians = angle * PI / 180.0;
    double newX = x + cos(radians) * distance;
    double newY = y + sin(radians) * distance;
    
    // Check if new position is valid
    int gridX = (int)round(newX);
    int gridY = (int)round(newY);
    
    if (gridX >= 0 && gridX < Width && gridY >= 0 && gridY < Height)
    {
        if (gameMap.getCell(gridX, gridY) != '#')
        {
            x = newX;
            y = newY;
        }
    }
}

// Terminal input setup
struct termios orig_termios;

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char readKey()
{
    char c = '\0';
    read(STDIN_FILENO, &c, 1);
    return c;
}

int main()
{
    Map gameMap;
    Player player(Width / 2.0, Height / 2.0);
    
    enableRawMode();
    
    bool running = true;
    gameMap.displayMap(player);
    
    while (running)
    {
        char key = readKey();
        
        bool needsRedraw = false;
        
        switch(key)
        {
            case 'w':
            case 'W':
                player.move(1.0, gameMap);
                needsRedraw = true;
                break;
            case 's':
            case 'S':
                player.move(-1.0, gameMap);
                needsRedraw = true;
                break;
            case 'a':
            case 'A':
                player.rotate(-15.0);
                needsRedraw = true;
                break;
            case 'd':
            case 'D':
                player.rotate(15.0);
                needsRedraw = true;
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
        }
        
        if (needsRedraw)
        {
            gameMap.displayMap(player);
        }
        
        usleep(10000); // Small delay to prevent CPU spinning
    }
    
    cout << "\033[2J\033[H"; // Clear screen
    cout << "Game exited." << endl;
    
    return 0;
}