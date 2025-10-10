#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>


using namespace std;
//map dimensions
const int Width = 50;
const int Height = 30;
//room constraints
const int Min_Rooms = 5;
const int Max_Rooms = 7;
//room size constraints
const int Min_Rooms_Size = 6;
const int Max_Rooms_Size = 12;
//tile types
enum Tile{
    Wall = '#',
    Floor = '.',
    Door = '+',
    Corridor = ' '
};

struct Room
{
    // coord x, y
    int x, y;
    // size
    int width, height;
    //builder
    Room(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h) {}
    // returns the coordinate x of the center of the room
    int centerX() const { return x + width / 2; }
    // returns the coordinate y of the center of the room
    int centerY() const { return y + height / 2; }
    // verifies if this room overlaps with another (margin of 2)
    bool overlaps(const Room& other, int margin = 2) const
    {
        return !(x + width + margin < other.x || 
                 other.x + other.width + margin < x ||
                 y + height + margin < other.y || 
                 other.y + other.height + margin < y);
    }
};


class Map{
    private:
    // map
    char map[Height][Width];
    // array of rooms
    vector <Room> rooms;

    public:
    //builder
    Map()
    {
        initializeMap();
    }
    // initializes the map
    void initializeMap(){
        for(int i=0; i<Height; i++)
        {
            for(int j=0; j<Width;j++)
            {
                map[i][j] = Wall;
            }
        }
    }
    // creates a room if it is within the map limits
    void createRoom(const Room& room)
    {
        for(int i=room.y; i<room.y + room.height; i++)
        {
            for(int j=room.x; j<room.x + room.width; j++)
            {
                if(i >= 0 && i < Height && j >= 0 && j < Width)
                {
                    map[i][j] = Floor;
                }
            }
        }
    }

    bool canplaceRoom(const Room& newRoom)
    {
        // verify map limits
        if(newRoom.x <1 || newRoom.y < 1 ||
           newRoom.x + newRoom.width >= Width -1 ||
           newRoom.y + newRoom.height >= Height -1)
        {
            return false;
        }
        // verify overlapping with other rooms
        for(const Room& room : rooms)
        {
            if(newRoom.overlaps(room))
            {
                return false;
            }
        }
        return true;
    }
    // creates a horizontal corridor
    void createHorizontalCorridor(int x1, int x2, int y)
    {
        // calculate the start
        int startx = min(x1,x2);
        // calculate the end
        int endx = max(x1,x2);
        // create the corridor
        for(int x = startx; x<= endx; x++)
        {
            if(x >=0 && x < Width && y >=0 && y < Height)
            {
                if(map[y][x] == Wall) map[y][x] = Corridor;
            }
        }
    }
    // creates a vertical corridor
    void createVerticalCorridor(int x, int y1, int y2)
    {
        // calculate the start
        int starty = min(y1,y2);
        // calculate the end
        int endy = max(y1,y2);
        // create the corridor
        for(int y = starty; y<= endy; y++)
        {
            if(x >=0 && x < Width && y >=0 && y < Height)
            {
                if(map[y][x] == Wall) map[y][x] = Corridor;
            }
        }
    }
    // connects two rooms with a corridor in L shape
    void connectRooms(const Room& room1, const Room& room2)
    {
        // get the center of the rooms to connect
        int x1 = room1.centerX();
        // get the center of the rooms to connect
        int y1 = room1.centerY();
        // get the center of the rooms to connect
        int x2 = room2.centerX();
        // get the center of the rooms to connect
        int y2 = room2.centerY();
        // create the L shaped corridor randomly
        if(rand() % 2 == 0)
        {
            createHorizontalCorridor(x1,x2,y1);
            createVerticalCorridor(x2,y1,y2);
        }
        else
        {
            createVerticalCorridor(x1,y1,y2);
            createHorizontalCorridor(x1,x2,y2);
        }
    }
    // door placement
    void placeDoors()
    {
        // iterate over all generated rooms
        for(const Room& room : rooms)
        {
            // incorridor flag  
            bool inCorridor = false;
            // top wall detection
            for(int x = room.x; x< room.x + room.width; x++)
            {
                if(room.y > 0 && map[room.y][x] == Floor &&
                   map[room.y -1][x] == Corridor)
                {
                    if(!inCorridor)
                    {
                        map[room.y][x] = Door;
                        inCorridor = true;
                    }
                    else
                    {
                        map[room.y][x] = Wall;
                    }
                }
                else
                {
                    inCorridor = false;
                }
            }
            // bottom wall
            inCorridor = false;
            for(int x = room.x; x< room.x + room.width; x++)
            {
                if(room.y + room.height < Height &&
                   map[room.y + room.height -1][x] == Floor &&
                   map[room.y + room.height][x] == Corridor)
                {
                    if(!inCorridor)
                    {
                        map[room.y + room.height -1][x] = Door;
                        inCorridor = true;
                    }
                    else
                    {
                        map[room.y + room.height -1][x] = Wall;
                    }
                }
                else
                {
                    inCorridor = false;
                }
                // left wall
                inCorridor = false;
                for(int y = room.y; y< room.y + room.height; y++)
                {
                    if(room.x > 0 && map[y][room.x] == Floor &&
                       map[y][room.x - 1] == Corridor)
                    {
                        if(!inCorridor)
                        {
                            map[y][room.x] = Door;
                            inCorridor = true;
                        }
                        else
                        {
                            map[y][room.x] = Wall;
                        }
                    }
                    else
                    {
                        inCorridor = false;
                    }
                }
                // right wall
                inCorridor = false;
                for(int y = room.y; y< room.y + room.height; y++)
                {
                    if(room.x + room.width < Width &&
                       map[y][room.x + room.width -1] == Floor &&
                       map[y][room.x + room.width] == Corridor)
                    {
                        if(!inCorridor)
                        {
                            map[y][room.x + room.width -1] = Door;
                            inCorridor = true;
                        }
                        else
                        {
                            map[y][room.x + room.width -1] = Wall;
                        }
                    }
                    else
                    {
                        inCorridor = false;
                    }
                }
            }
        }
    }
    // generates the map  
    void generate()
    {
        // generate random number of rooms
        int numRooms = Min_Rooms + rand() % (Max_Rooms - Min_Rooms + 1);
        // generate non-overlapping rooms
        int attempts = 0;
        while(rooms.size() < numRooms && attempts < 1000)
        {
            // random width
            int w = Min_Rooms_Size + rand() % (Max_Rooms_Size - Min_Rooms_Size + 1);
            // random height
            int h = Min_Rooms_Size + rand() % (Max_Rooms_Size - Min_Rooms_Size + 1);
            // random position in x
            int x = 1 + rand() % (Width - w - 2);
            // random position in y
            int y = 1 + rand() % (Height - h - 2);
            // create a new room
            Room newRoom(x, y, w, h);
            // check if it can be placed
            if(canplaceRoom(newRoom))
            {
                rooms.push_back(newRoom);
                createRoom(newRoom);
            }
            attempts++;

            // connect rooms with corridors
            for(size_t i=1; i<rooms.size(); i++)
            {
                connectRooms(rooms[i], rooms[i-1]);
            }

            placeDoors();
        }
    }
    // display the map
    void display()
    {
        for(int y=0; y<Height; y++)
        {
            for(int x=0; x<Width; x++)
            {
                cout << map[y][x];
            }
            cout << endl;
        }

        cout << "  # = Muro\n";
        cout << "  . = Suelo de habitación\n";
        cout << "    = Pasillo\n";
        cout << "  + = Puerta\n";
    }

};

int main()
{

    srand(time(0));
    Map dungeon;

    dungeon.generate();
    dungeon.display();
    
    return 0;
}