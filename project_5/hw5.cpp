//---------------------------------------
// Program: hw5.cpp
// Purpose: Implement player movement and treasure collection
// Author:  Laney Pennington
// Date:    4/10/24
//---------------------------------------
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "src/libim/im_color.h"
using namespace std;

// Constants
const float VIEW_SIZE = 2.0;
const float GROUND_Z_MIN = -1.5;
const float GROUND_X_MIN = -1.5;
const float GROUND_Z_MAX = 1.5;
const float GROUND_X_MAX = 1.5;
const int NUM_GEMS = 3;
const int NUM_GOLD = 3;
const int TOTAL_TREASURES = NUM_GEMS + NUM_GOLD;

// Global variables 
int xangle = -90;
int yangle = 0;
int zangle = 0;
int numRows, numCols, playerRow, playerCol = 0;
char **maze;
float blockWidth, blockHeight = 0.0f;
float playerWidth, playerHeight = 0.0f;
float smallBlockDiff = 0.05;
int treasuresCollected = 0;

// Textures
int xdim, ydim;
unsigned char *brick_texture;
unsigned char *grass_texture;
unsigned char *rock_texture;
unsigned char *wood_texture;
unsigned char *gold_texture;
unsigned char *gems_texture;
unsigned char *yellow_texture;

//---------------------------------------
// Initialize texture image - from JGauch texture3.cpp
//---------------------------------------
void init_texture(char *name, unsigned char *&texture, int &xdim, int &ydim)
{
   // Read jpg image
   im_color image;
   image.ReadJpg(name);
   // printf("Reading image %s\n", name);
   xdim = 1; while (xdim < image.R.Xdim) xdim*=2; xdim /=2;
   ydim = 1; while (ydim < image.R.Ydim) ydim*=2; ydim /=2;
   image.Interpolate(xdim, ydim);
   // printf("Interpolating to %d by %d\n", xdim, ydim);

   // Copy image into texture array
   texture = (unsigned char *)malloc((unsigned int)(xdim*ydim*3));
   int index = 0;
   for (int y = 0; y < ydim; y++)
      for (int x = 0; x < xdim; x++)
      {
         texture[index++] = (unsigned char)(image.R.Data2D[y][x]);
         texture[index++] = (unsigned char)(image.G.Data2D[y][x]);
         texture[index++] = (unsigned char)(image.B.Data2D[y][x]);
      }
}

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-VIEW_SIZE, VIEW_SIZE, -VIEW_SIZE, VIEW_SIZE, -VIEW_SIZE, VIEW_SIZE);
    glEnable(GL_DEPTH_TEST);

    // Init textures
    init_texture((char *)"src/textures/brick.jpg", brick_texture, xdim, ydim);
    init_texture((char *)"src/textures/grass.jpg", grass_texture, xdim, ydim);
    init_texture((char *)"src/textures/rock.jpg", rock_texture, xdim, ydim);
    init_texture((char *)"src/textures/wood.jpg", wood_texture, xdim, ydim);
    init_texture((char *)"src/textures/gold.jpg", gold_texture, xdim, ydim);
    init_texture((char *)"src/textures/gems.jpg", gems_texture, xdim, ydim);
    init_texture((char *)"src/textures/yellow.jpg", yellow_texture, xdim, ydim);

    glEnable(GL_TEXTURE_2D);

    // set parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

//---------------------------------------
// Changes the current texture being used
//---------------------------------------
void swap_texture(const char ch)
{
    // printf("texture char is: '%c'\n", ch);
    switch (ch)
    {
        // block
        case 'b': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, brick_texture);
                  break;
        // grass
        case 'a': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, grass_texture);
                  break;
        // rock
        case 'r': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, rock_texture);
                  break;
        // wood
        case 'w': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, wood_texture);
                  break;
        // player
        case 'p': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, yellow_texture);
                  break;
        // gold
        case 'g': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, gold_texture);
                  break;
        // gems
        case 'G': glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, gems_texture);
                  break;
        default: cout << "ERROR IN SWAP_TEXTURE" << endl;
    }
}

//---------------------------------------
// Prints a map
//---------------------------------------
void print_map(const char* filename)
{
    ifstream maze_file;

    // open depth info file
    maze_file.open(filename);

    // if there is an error opening it, quit the program
    if (!maze_file.is_open())
    {
        printf("Error opening %s", filename);
        exit(1);
    }

    // ignore rows, cols, and player positions
    maze_file.ignore(10, '\n');
    maze_file.ignore(10, '\n');

    string currRow = "";
    while (!maze_file.eof())
    {
        getline(maze_file, currRow);
        cout << currRow << endl;
    }

    maze_file.close();
}

//---------------------------------------
// Allows player to choose a map
//---------------------------------------
string map_selection()
{
    int userChoice = 0;
    cout << "Which map would you like to play?" << endl;
    
    cout << "Map 1" << endl;
    cout << "------------------------" << endl;
    print_map("maze.txt");
    cout << endl;

    cout << "Map 2" << endl;
    cout << "------------------------" << endl;
    print_map("maze2.txt");
    cout << endl;

    cout << "Map 3" << endl;
    cout << "------------------------" << endl;
    print_map("maze3.txt");
    cout << endl;

    cout << "Map 4" << endl;
    cout << "------------------------" << endl;
    print_map("maze4.txt");
    cout << endl;

    cout << ">> "; cin >> userChoice;
    while (userChoice <= 0 || userChoice > 4)
    {
        cout << "Invalid choice. Please try again." << endl;
        cout << ">> "; cin >> userChoice;
    }

    if (userChoice == 1)
        return "maze.txt";
    else if (userChoice == 2)
        return "maze2.txt";
    else if (userChoice == 3)
        return "maze3.txt";
    else if (userChoice == 4)
        return "maze4.txt";
    else
        return NULL;
}

//---------------------------------------
// Read in info for maze
//---------------------------------------
void read_maze_info(const char* filename)
{
    ifstream maze_file;

    // open depth info file
    maze_file.open(filename);

    // if there is an error opening it, quit the program
    if (!maze_file.is_open())
    {
        printf("Error opening %s", filename);
        exit(1);
    }

    // get number of rows & columns and where the player starts
    maze_file >> numRows >> numCols >> playerRow >> playerCol;
    // printf("numRows = %d | numCols = %d | playerRow = %d | playerCol = %d\n", numRows, numCols, playerRow, playerCol);
    // ignore newline char
    maze_file.ignore(10, '\n');

    // set block width and height to fill map
    blockWidth = ((2 * VIEW_SIZE) - 1.0) / numCols;
    blockHeight = ((2 * VIEW_SIZE) - 1.0) / numRows;
    // printf("blockWidth = %f, blockHeight = %f\n", blockWidth, blockHeight);

    // allocate space for maze
    maze = new char*[numRows];
    for (int i = 0; i < numRows; i++) {
        // Declare a memory block of size numCols
        maze[i] = new char[numCols];
    }

    // read in from maze_file
    for (int r = 0; r < numRows; r++)
    {
        for (int c = 0; c < numCols; c++)
        {
            // get next char and save to maze
            maze_file.get(maze[r][c]);
        }
        // ignore newline char
        maze_file.ignore(10,'\n');
    }

    // close file
    maze_file.close();

    // testing
    // for (int r = 0; r < numRows; r++)
    // {
    //     for (int c = 0; c < numCols; c++)
    //     {
    //         // get next char and save to maze
    //         printf("maze[%d][%d] = '%c'\n", r, c, maze[r][c]);
    //     }
    // }
}

//---------------------------------------
// Init positions for player, gems, and gold
//---------------------------------------
void init_positions()
{
    // set player position
    maze[playerRow][playerCol] = 'p';

    // seed rand()
    srand(time(nullptr));
    int row = 0;
    int col = 0;

    // for each gem we want to create...
    for (int i = 0; i < NUM_GEMS; i++)
    {
        // get random row and column
        row = rand() % numRows;
        col = rand() % numCols;

        // if the maze is empty at that spot
        if (maze[row][col] == ' ')
        {
            // put a gem there
            maze[row][col] = 'G';
        }
        // otherwise...
        else
        {
            // decrement (so the loop will 'redo' this gem)
            i--;
        }
    }

    // for each gold we want to create...
    for (int i = 0; i < NUM_GOLD; i++)
    {
        // get random row and column
        row = rand() % numRows;
        col = rand() % numCols;

        // if the maze is empty at that spot
        if (maze[row][col] == ' ')
        {
            // put a gold there
            maze[row][col] = 'g';
        }
        // otherwise...
        else
        {
            // decrement (so the loop will 'redo' this gold)
            i--;
        }
    }
}

//---------------------------------------
// Checks win condition
//---------------------------------------
void check_win()
{
    // if the player has collected all the treasures...
    if (treasuresCollected == TOTAL_TREASURES)
    {
        // tell them they win!
        cout << "You collected all the treasures! You win!" << endl;
        // exit program
        exit(0);
    }
}

//---------------------------------------
// Function to draw 3D block - augmented from JGauch texture3.cpp
//---------------------------------------
void block(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax, char texture)
{
    // Define 8 vertices
    float ax = xmin, ay = ymin, az = zmax;
    float bx = xmax, by = ymin, bz = zmax;
    float cx = xmax, cy = ymax, cz = zmax;
    float dx = xmin, dy = ymax, dz = zmax;

    float ex = xmin, ey = ymin, ez = zmin;
    float fx = xmax, fy = ymin, fz = zmin;
    float gx = xmax, gy = ymax, gz = zmin;
    float hx = xmin, hy = ymax, hz = zmin;

    // change texture to current block's texture
    swap_texture(texture);

    // Draw 6 faces
    glBegin(GL_POLYGON);  // Max texture coord 1.0
    glTexCoord2f(0.0, 0.0);
    glVertex3f(ax, ay, az);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(bx, by, bz);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(cx, cy, cz);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(dx, dy, dz);
    glEnd();

    glBegin(GL_POLYGON);  // Max texture coord 1.0
    glTexCoord2f(0.0, 0.0);
    glVertex3f(ex, ey, ez);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(ax, ay, az);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(dx, dy, dz);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(hx, hy, hz);
    glEnd();

    glBegin(GL_POLYGON);  // Max texture coord 1.0
    glTexCoord2f(0.0, 0.0);
    glVertex3f(fx, fy, fz);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(ex, ey, ez);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(hx, hy, hz);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(gx, gy, gz);
    glEnd();

    glBegin(GL_POLYGON);  // Max texture coord 1.0
    glTexCoord2f(0.0, 0.0);
    glVertex3f(bx, by, bz);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(fx, fy, fz);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(gx, gy, gz);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(cx, cy, cz);
    glEnd();

    glBegin(GL_POLYGON);  // Max texture coord 3.0
    glTexCoord2f(0.0, 0.0);
    glVertex3f(ax, ay, az);
    glTexCoord2f(2.0, 0.0);
    glVertex3f(ex, ey, ez);
    glTexCoord2f(2.0, 2.0);
    glVertex3f(fx, fy, fz);
    glTexCoord2f(0.0, 2.0);
    glVertex3f(bx, by, bz);
    glEnd();

    glBegin(GL_POLYGON);  // Max texture coord 3.0
    glTexCoord2f(0.0, 0.0);
    glVertex3f(gx, gy, gz);
    glTexCoord2f(3.0, 0.0);
    glVertex3f(cx, cy, cz);
    glTexCoord2f(3.0, 3.0);
    glVertex3f(dx, dy, dz);
    glTexCoord2f(0.0, 3.0);
    glVertex3f(hx, hy, hz);
    glEnd();
}

//---------------------------------------
// Keyboard callback for OpenGL - pulled from JGauch's surface2.cpp
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    // Update angles
    if (key == 'x')
        xangle -= 5;
    else if (key == 'y')
        yangle -= 5;
    else if (key == 'z')
        zangle -= 5;
    else if (key == 'X')
        xangle += 5;
    else if (key == 'Y')
        yangle += 5;
    else if (key == 'Z')
        zangle += 5;
    // player movement
    else if (key == 'w')
    {
        // if the player is NOT in the top-most row and the next space empty or a prize...
        if ((playerRow != numRows -1) && ((maze[playerRow+1][playerCol] == ' ') || maze[playerRow+1][playerCol] == 'g' || maze[playerRow+1][playerCol] == 'G'))
        {
            // if they collected a treasure...
            if ((maze[playerRow+1][playerCol] == 'g') || (maze[playerRow+1][playerCol] == 'G'))
            {
                printf("%d out of %d treasures collected!\n", ++treasuresCollected, TOTAL_TREASURES);
            }
            // delete them from their current spot
            maze[playerRow][playerCol] = ' ';
            // put them in a spot one row up
            maze[++playerRow][playerCol] = 'p';
        }
    }
    else if (key == 'a')
    {
        // if the player is NOT in the left-most col and the next space empty or a prize...
        if ((playerCol != 0) && ((maze[playerRow][playerCol-1] == ' ') || maze[playerRow][playerCol-1] == 'g' || maze[playerRow][playerCol-1] == 'G'))
        {
            // if they collected a treasure...
            if ((maze[playerRow][playerCol-1] == 'g') || (maze[playerRow][playerCol-1] == 'G'))
            {
                printf("%d out of %d treasures collected!\n", ++treasuresCollected, TOTAL_TREASURES);
            }
            // delete them from their current spot
            maze[playerRow][playerCol] = ' ';
            // put them in a spot one col left
            maze[playerRow][--playerCol] = 'p';

        }
    }
    else if (key == 's')
    {
        // if the player is NOT in the bottom-most col and the next space empty or a prize...
        if ((playerRow != 0) && ((maze[playerRow-1][playerCol] == ' ') || maze[playerRow-1][playerCol] == 'g' || maze[playerRow-1][playerCol] == 'G'))
        {
            // if they collected a treasure...
            if ((maze[playerRow-1][playerCol] == 'g') || (maze[playerRow-1][playerCol] == 'G'))
            {
                printf("%d out of %d treasures collected!\n", ++treasuresCollected, TOTAL_TREASURES);
            }
            // delete them from their current spot
            maze[playerRow][playerCol] = ' ';
            // put them in a spot one col left
            maze[--playerRow][playerCol] = 'p';
        }
    }
    else if (key == 'd')
    {
        // if the player is NOT in the left-most col and the next space empty or a prize...
        if ((playerCol != (numCols -1)) && ((maze[playerRow][playerCol+1] == ' ') || maze[playerRow][playerCol+1] == 'g' || maze[playerRow][playerCol+1] == 'G'))
        {
            // if they collected a treasure...
            if ((maze[playerRow][playerCol+1] == 'g') || (maze[playerRow][playerCol+1] == 'G'))
            {
                printf("%d out of %d treasures collected!\n", ++treasuresCollected, TOTAL_TREASURES);
            }
            // delete them from their current spot
            maze[playerRow][playerCol] = ' ';
            // put them in a spot one col left
            maze[playerRow][++playerCol] = 'p';
        }
    }
    // Quit program
    else if (key == 'q')
        exit(0);

    // check if the player won
    check_win();

    // Redraw objects
    glutPostRedisplay();
}

//---------------------------------------
// Draws the blocks for the map
//---------------------------------------
void draw_map()
{
    // corresponding indeces for the next loop
    int rowIndex = 0;
    int colIndex = 0;

    // draw blocks from top left to bottom right
    for (float z = GROUND_Z_MIN; z < GROUND_Z_MAX && rowIndex < numRows; z += blockHeight)
    {
        for (float x = GROUND_X_MIN; x < GROUND_X_MAX && colIndex < numCols; x += blockWidth)
        {
            // printf("z = %f | x = %f\n", z, x);
            // printf("maze[%d][%d] = '%c'\n\n", rowIndex, colIndex, maze[rowIndex][colIndex]);
            // if a block needs to be drawn
            if (maze[rowIndex][colIndex] != ' ')
            {
                // if it's a gold or gems
                if (maze[rowIndex][colIndex] == 'g' || maze[rowIndex][colIndex] == 'G' || maze[rowIndex][colIndex] == 'p')
                {
                    // draw small block
                    block(x + smallBlockDiff, 0 + smallBlockDiff, z + smallBlockDiff, x + blockWidth - smallBlockDiff, blockHeight - smallBlockDiff, z + blockHeight - smallBlockDiff, maze[rowIndex][colIndex]);
                }
                // otherwise it's a map block
                else
                {
                    // draw big block
                    block(x, 0, z, x + blockWidth, blockHeight, z + blockHeight, maze[rowIndex][colIndex]);
                }
            }
            // increment maze array index
            colIndex++;
        }
        // increment maze array index
        rowIndex++;
        // reset col index
        colIndex = 0;
    }

    // draw ground
    swap_texture('a');
    glBegin(GL_POLYGON);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(GROUND_X_MIN, blockHeight + 0.01, GROUND_Z_MIN); // top left corner
    glTexCoord2f(3.0, 0.0);
    glVertex3f(GROUND_X_MAX, blockHeight + 0.01, GROUND_Z_MIN); // top right corner
    glTexCoord2f(3.0, 3.0);
    glVertex3f(GROUND_X_MAX, blockHeight + 0.01, GROUND_Z_MAX); // bottom right corner
    glTexCoord2f(0.0, 3.0);
    glVertex3f(GROUND_X_MIN, blockHeight + 0.01, GROUND_Z_MAX); // bottom left corner
    glEnd();
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
    // Incrementally rotate objects
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(xangle, 1.0, 0.0, 0.0);
    glRotatef(yangle, 0.0, 1.0, 0.0);

    // draws blocks for map and draws ground
    draw_map();

    glFlush();
}

//---------------------------------------
// Deletes allocated array memory
//---------------------------------------
void clean()
{
    for (int i = 0; i < numRows; i++)
    {
        delete[] maze[i];
    }
    delete[] maze;
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(250, 250);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
    glutCreateWindow("Maze");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    string map = map_selection();
    read_maze_info(map.c_str());
    init_positions();
    init();
    printf("Keyboard commands:\n");
    printf("   'w' - move forward\n");
    printf("   'a' - move left\n");
    printf("   's' - move backward\n");
    printf("   'd' - move right\n");
    printf("   'x' - rotate x-axis -5 degrees\n");
    printf("   'X' - rotate x-axis +5 degrees\n");
    printf("   'y' - rotate y-axis -5 degrees\n");
    printf("   'Y' - rotate y-axis +5 degrees\n");
    printf("   'z' - rotate z-axis -5 degrees\n");
    printf("   'Z' - rotate z-axis +5 degrees\n");
    printf("   'q' - quit program\n");
    printf("------------------------------------------------\n");
    glutMainLoop();
    clean();
    return 0;
}