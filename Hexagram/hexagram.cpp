//********************************************************//
// Project:     Hexagram
// Filename:    hexagram.cpp
// Author:      Ted Gress
// Platform:    DOS - Windows XP/2000

//********************DEFINES****************************//

// Entity Struct Identifier Constants
#define ENTITY_PLAYERSTART 1
#define ENTITY_WARP 2

// Interface Highlight Constants

#define HIGHLIGHT_MAINMENU 1
#define HIGHLIGHT_STATUS 2
#define HIGHLIGHT_INVENTORY 3
#define HIGHLIGHT_GRIMOIRE 4
#define HIGHLIGHT_SACRIFICE 5
#define HIGHLIGHT_SUMMON 6
#define HIGHLIGHT_TRANSMUTATION 7

// Number of entities possible
#define ENTITIES_NUM 50

// Sprite Status Defines
#define SPRITE_STATUS_WAIT_NORTH 1
#define SPRITE_STATUS_WAIT_SOUTH 2
#define SPRITE_STATUS_WAIT_WEST 3
#define SPRITE_STATUS_WAIT_EAST 4
#define SPRITE_STATUS_MOVE_NORTH 5
#define SPRITE_STATUS_MOVE_SOUTH 6
#define SPRITE_STATUS_MOVE_WEST 7
#define SPRITE_STATUS_MOVE_EAST 8

// also add attack and spell casting




//********************INCLUDES***************************//

#include <iostream>
#include <allegro.h>

using namespace std;

//********************PROTOTYPES*************************//

//Initialization functions
int init(void);
int destroy(void);

//Rendering Functions
int main_blit(void);
int buffer_blit(int scrollx, int scrolly);

int renderbottom(void);
int rendermid(void);
int rendermid2(void);
int renderhigh(void);
int renderplayer(void);

int main_menu_blit(int main_menu_active);
int transmute_menu_blit(int transmute_menu_active);
int element_menu_icon_blit(void);
int highlight_blit(int highlight_id);
int element_icon_blit(void);

//Main Functions
int main_loop(void);

//Loading Functions
int load_bitmaps(void);
int load_file(char loadfile[80]);
int load_floortiles(void);
int load_player_sprites(void);

//Player Functions
int center_on_player(void);
int find_player(void);

//Input Functions
int keyboard_input(void);


//Animation Functions
int player_animation(void);


//Collision Detection Functions
bool no_collision(int x, int y);


//Element Availabity Functions
int check_element_avail(void);

//********************GLOBALS****************************//

BITMAP *title;
BITMAP *interface;
BITMAP *buffer;
BITMAP *bufferx;
PALETTE the_palette;
COLOR_MAP trans_table;

//*************
// Tiles
//*************

//Floor Tiles
BITMAP *floor1;
BITMAP *floor2;
BITMAP *floor3;
BITMAP *floor4;
BITMAP *floor5;

//*************
// Sprites
//*************

// Lucius
BITMAP *lucius1;
BITMAP *lucius2;
BITMAP *lucius3;
BITMAP *lucius4;
BITMAP *lucius5;
BITMAP *lucius6;
BITMAP *lucius7;
BITMAP *lucius8;
BITMAP *lucius9;
BITMAP *lucius10;
BITMAP *lucius11;
BITMAP *lucius12;

//**************
// Menu Graphics
//**************

BITMAP *mainmenugraphic;

BITMAP *widehighlight;
BITMAP *smallhighlight;
BITMAP *elementradar;

BITMAP *transmutemenu;
//***************
// Element Icons
//***************

BITMAP *watericon;
BITMAP *fireicon;
BITMAP *windicon;
BITMAP *earthicon;
BITMAP *electricicon;
BITMAP *woodicon;
BITMAP *greyedout;


//****************
// Arrays
//*****************

//Map Format - Maps are Isometric with support for animated tiles

int bottombmp[100][100];        //bottom layer bitmap (floor), under player
int midbmp[100][100];           //middle layer bitmap (walls), overlap players body
int mid2bmp[100][100];          //second mid layer (walls), below player but above mid
int hibmp[100][100];            //hi layer bitmap (ceiling)

//Element Tables
//When an element exists, that map square is set to true

bool earthmap[100][100];
bool watermap[100][100];
bool windmap[100][100];
bool firemap[100][100];
bool electricmap[100][100];
bool woodmap[100][100];

//Misc. Tables
bool collision[100][100];       //collision blocks
int entitymap[100][100];         //index of entity map

//Entities
//Entities consist of npcs, enemies, warps, items, objects, and interactive objects such as switches
typedef struct entity{
        int index;              // look up index from entities table
        int x;                  // xcoordinate in entities table
        int y;                  // ycoordinate in entities table
        int type;               // a constant identifier from ENTITY defines
        int warptox;            // coordinates to warp to on the new map (x)
        int warptoy;            // coordinates to warp to on the new map (y)
        bool pickupable;        // the item can be picked up and put in inventory
        bool activatable;       // the object can be used by double clicking it
        bool activated;         // the object has or hasn't been used
        int tileid;             // the identifier that represents the tile being used
                                // for the entity. (everything but warps and start points)
        
}entity;

entity entities[ENTITIES_NUM];


//***********************
// Miscellaneous Globals
//***********************

//Scroll Control Values
int scrollx = 0;        //used to control the scrolling on screen
int scrolly = 0;        //used to control the scrolling on screen

//Player Rendering Values
int playerx = 0;                         // the players x coordinate on screen
int playery = 0;                         // the players y coordinate on screen
int status = SPRITE_STATUS_WAIT_SOUTH;   // the player current animation frame type
                                         // from defines SPRITE_STATUS
int frame = 0;          //frame number of current animation

//Element Availability
bool water_avail = false;
bool earth_avail = false;
bool wind_avail = false;
bool fire_avail = false;
bool electric_avail = false;
bool wood_avail = false;


//Element Window

int element_page = 0; //current page of the element transmutation window


//************************
// Player Abilities
//************************


// Player Transmutation Abilities
bool transmutations[10];
bool summoning[10];


//*************************************************************************//
//********************************SOURCE CODE******************************//
//*************************************************************************//

//********************************MAIN BLOCK*******************************//

//*******************
//Name:         main()
//Parameters:   void
//Return:       int
//*******************

int main(void)
{
        int init_value = 0;
        init_value = init();
        if (init_value != 0)
        {
                return init_value;
        }

        clear(buffer);
        show_mouse(NULL);
        blit(title, screen, 0, 0, 0, 0, 640, 480);
        show_mouse(screen);
        readkey();
        clear(screen);

        load_file("map1.hxm");

        main_loop();

        destroy();

        return 0;
}



//*******************
//Name:         main_loop()
//Parameters:   void
//Return:       int
//*******************

int main_loop(void)
{
        int highlight_id = 0;
        bool main_menu_active = false;
        bool transmute_menu_active = false;
        char tmp[80];
        bool control = true;


        //***********DEMO ONLY*************

        for (int i = 0;i<10;i++)
        {
                transmutations[i] = true;
                summoning[i] = true;
        }


        //************END DEMO*************

        center_on_player();
        while (control)
        {
                clear(buffer);
                clear(bufferx);
                renderbottom();
                rendermid();
                renderplayer();
                rendermid2();
                renderhigh();

                buffer_blit(scrollx, scrolly);

                //*debug routines******************************

                sprintf(tmp, "Mouse X is %d", mouse_x);
                textout_ex(bufferx, font, tmp, 5, 10, makecol(0,0,255),-1);
                sprintf(tmp, "Mouse Y is %d", mouse_y);
                textout_ex(bufferx, font, tmp, 5, 20, makecol(0,0,255),-1);

                if (mouse_b & 1)
                        textout_ex(bufferx, font, "Left button is pressed", 5, 30, makecol(0,0,255),-1);
                else
                        textout_ex(bufferx, font, "Left button is not pressed", 5, 30, makecol(0,0,255),-1);

                if (mouse_b & 2)
                        textout_ex(bufferx, font, "Right button is pressed", 5, 40, makecol(0,0,255),-1);
                else
                        textout_ex(bufferx, font, "Right button is not pressed", 5, 40, makecol(0,0,255),-1);

                sprintf(tmp, "Scroll X is %d          ", scrollx);
                textout_ex(bufferx, font, tmp, 5, 50, makecol(0,0,255),-1);
                sprintf(tmp, "Scroll Y is %d          ", scrolly);
                textout_ex(bufferx, font, tmp, 5, 60, makecol(0,0,255),-1);


                //**********************************************
                //**************INTERFACE HANDLER***************
                if (mouse_b & 1)
                {
                        // main menu button
                        if ((mouse_x > 413) && (mouse_x < 500))
                        {
                                if (( mouse_y > 313) && (mouse_y < 337))
                                {
                                        if (main_menu_active == false)
                                                main_menu_active = true;
                                        else
                                                main_menu_active = false;                       
                                }
                        }
                        // main menu quit button
                        if ((mouse_x > 263) && (mouse_x < 353))
                        {
                                if ((mouse_y > 172) && (mouse_y < 196))
                                {
                                        if (main_menu_active == true)
                                                control = false;
                                }
                         }
                         // main menu close button
                         if ((mouse_x > 263) && (mouse_x < 353))
                         {
                                if ((mouse_y > 213) && (mouse_y < 235))
                                {
                                        if (main_menu_active == true)
                                                main_menu_active = false;
                                 }
                         }
                }
                //*******************END INTERFACE HANDLER************
                //*******************BEGIN HIGHLIGHT HANDLER**********
                if ((mouse_y > 305) && (mouse_y < 337))
                {
                        if ((mouse_x > 413) && (mouse_x < 498))
                        {
                                highlight_id = HIGHLIGHT_MAINMENU;
                        }
                        else if ((mouse_x > 332) && (mouse_x < 379))
                        {
                                highlight_id = HIGHLIGHT_STATUS;
                        }
                        else if ((mouse_x > 230) && (mouse_x < 305))
                        {
                                highlight_id = HIGHLIGHT_INVENTORY;
                        }
                        else if ((mouse_x > 135) && (mouse_x < 203))
                        {
                                highlight_id = HIGHLIGHT_GRIMOIRE;
                        }
                        else if ((mouse_x > 97) && (mouse_x < 118))
                        {
                                highlight_id = HIGHLIGHT_SACRIFICE;
                        }
                        else if ((mouse_x > 43) && (mouse_x < 83))
                        {
                                highlight_id = HIGHLIGHT_SUMMON;
                        }
                        else if ((mouse_x > 7) && (mouse_x < 42))
                        {
                                highlight_id = HIGHLIGHT_TRANSMUTATION;
                        }
                        else
                        {
                                highlight_id = 0;
                        }
                } // end if
                else
                        highlight_id = 0;

                //*******************END HIGHLIGHT HANDLER************
                //*******************TRANSMUTATON MENU HANDLER********


                if (mouse_b & 1)
                {
                        // transmutation button
                        if ((mouse_x > 7) && (mouse_x < 42))
                        {
                                if (( mouse_y > 305) && (mouse_y < 337))
                                {
                                        if (transmute_menu_active == false)
                                                transmute_menu_active = true;
                                        else
                                                transmute_menu_active = false;                       
                                }
                         }
                }

                //*******************END TRANSMUATION HANDLER*********

                highlight_blit(highlight_id);
                main_menu_blit(main_menu_active);
                transmute_menu_blit(transmute_menu_active);


                if (transmute_menu_active)
                {
                        element_menu_icon_blit();
                }


                element_icon_blit();
                main_blit();

                keyboard_input();
                player_animation();
                check_element_avail();

        } // end while loop

        return 0;
}

//****************************INITIALIZATION FUNCTIONS*********************//

//*******************
//Name:         init()
//Parameters:   void
//Return:       int
//*******************


int init(void)
{
        cout << "HEXAGRAM Alpha Version 1.0" << endl;
        cout << "Created by Ted Gress" << endl;
        cout << "Initializing Allegro Graphics Library" << endl;

        
        if (allegro_init() != 0)
        {
                cout << "Allegro Initialization Failed!" << endl;
                return 1;
        }

        cout << "Installing keyboard..." << endl;
        install_keyboard();
        install_mouse();

        cout << "Setting color depth" << endl;
        set_color_depth(16);
        set_color_conversion(COLORCONV_TOTAL);

        cout << "Press Any Key..." << endl;
        readkey();
        cout << "Setting graphics mode..." << endl;
        if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) != 0)
        {
                if (set_gfx_mode(GFX_SAFE, 640, 480, 0, 0) != 0)
                {
                        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                        allegro_message("Unable to set any graphic mode");
                        return 1;
                }
        }

 
        create_trans_table(&trans_table, the_palette, 128, 128, 128, NULL);
        set_trans_blender(0, 0, 0, 100);
        bufferx = create_bitmap(640, 480);
        buffer = create_bitmap(1280, 960);

        load_bitmaps();
        load_floortiles();
        load_player_sprites();

        show_mouse(screen);
        return 0;
}

//*******************
//Name:         destroy()
//Parameters:   void
//Return:       int
//*******************


int destroy(void)
{
        show_mouse(NULL);

        destroy_bitmap(title);
        destroy_bitmap(interface);
        destroy_bitmap(buffer);
        destroy_bitmap(bufferx);

        destroy_bitmap(floor1);
        destroy_bitmap(floor2);
        destroy_bitmap(floor3);
        destroy_bitmap(floor4);
        destroy_bitmap(floor5);

        destroy_bitmap(lucius1);
        destroy_bitmap(lucius2);
        destroy_bitmap(lucius3);
        destroy_bitmap(lucius4);
        destroy_bitmap(lucius5);
        destroy_bitmap(lucius6);
        destroy_bitmap(lucius7);
        destroy_bitmap(lucius8);
        destroy_bitmap(lucius9);
        destroy_bitmap(lucius10);
        destroy_bitmap(lucius11);
        destroy_bitmap(lucius12);

        destroy_bitmap(fireicon);
        destroy_bitmap(watericon);
        destroy_bitmap(earthicon);
        destroy_bitmap(windicon);
        destroy_bitmap(electricicon);
        destroy_bitmap(woodicon);
        destroy_bitmap(greyedout);


        destroy_bitmap(transmutemenu);
        destroy_bitmap(mainmenugraphic);
        destroy_bitmap(elementradar);

        destroy_bitmap(widehighlight);
        destroy_bitmap(smallhighlight);
       
        return 0;
}


//***************************LOADING FUNCTIONS*******************************//

//*******************
//Name:         load_bitmaps()
//Parameters:   void
//Return:       int
//*******************


int load_bitmaps(void)
{

        // INTRO BITMAPS
        title = load_bitmap("title.bmp", the_palette);
        if (!title)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading title screen bitmap\n");
                return 1;
        }

        mainmenugraphic = load_bitmap("mainmenu.bmp", the_palette);
        if (!mainmenugraphic)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading main menu bitmap\n");
                return 1;
        }
        // INTERFACE BITMAPS
        interface = load_bitmap("interface.bmp", the_palette);
        if (!interface)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading interface bitmap\n");
                return 1;
        }
  
        widehighlight = load_bitmap("widehighlight.bmp", the_palette);
        if (!widehighlight)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading wide highlight bitmap\n");
                return 1;
        }
        smallhighlight = load_bitmap("smallhighlight.bmp", the_palette);
        if (!smallhighlight)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading small highlight bitmap\n");
                return 1;
        }

        elementradar = load_bitmap("elementradar.bmp", the_palette);
        if (!elementradar)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element radar bitmap\n");
                return 1;
        }


        fireicon = load_bitmap("fireediticon.bmp", the_palette);
        if (!fireicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element icon bitmap\n");
                return 1;
        }


        watericon = load_bitmap("waterediticon.bmp", the_palette);
        if (!watericon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element icon bitmap\n");
                return 1;
        }


        windicon = load_bitmap("windediticon.bmp", the_palette);
        if (!windicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element icon bitmap\n");
                return 1;
        }
        earthicon = load_bitmap("earthediticon.bmp", the_palette);
        if (!earthicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element icon bitmap\n");
                return 1;
        }
        electricicon = load_bitmap("electricediticon.bmp", the_palette);
        if (!electricicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element icon bitmap\n");
                return 1;
        }

        woodicon = load_bitmap("woodediticon.bmp", the_palette);
        if (!woodicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading element icon bitmap\n");
                return 1;
        }


        transmutemenu = load_bitmap("transmutemenu.bmp", the_palette);
        if (!transmutemenu)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading transmutation menu bitmap\n");
                return 1;
        }
        
        greyedout = load_bitmap("greyedout.bmp", the_palette);
        if (!greyedout)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading greyout bitmap\n");
                return 1;
        }


}

//*******************
//Name:         load_floortiles()
//Parameters:   void
//Return:       int
//*******************
int load_floortiles(void)
{
        floor1 = load_bitmap("floor1.bmp", the_palette);
        if (!floor1)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading floor tile bitmap\n");
                return 1;
        }
        /*
        floor2 = load_bitmap("floor2.bmp", the_palette);
        if (!floor2)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading floor tile bitmap\n");
                return 1;
        }

        floor3 = load_bitmap("floor3.bmp", the_palette);
        if (!floor3)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading floor tile bitmap\n");
                return 1;
        }

        floor4 = load_bitmap("floor4.bmp", the_palette);
        if (!floor4)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading floor tile bitmap\n");
                return 1;
        }

        floor5 = load_bitmap("floor5.bmp", the_palette);
        if (!floor5)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading floor tile bitmap\n");
                return 1;
        }
        */
        return 0;
}

//*******************
//Name:         load_player_sprites()
//Parameters:   void
//Return:       int
//*******************

int load_player_sprites(void)
{
        lucius1 = load_bitmap("lucius1.bmp", the_palette);
        if (!lucius1)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }


        lucius2 = load_bitmap("lucius2.bmp", the_palette);
        if (!lucius2)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius3 = load_bitmap("lucius3.bmp", the_palette);
        if (!lucius3)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius4 = load_bitmap("lucius4.bmp", the_palette);
        if (!lucius4)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius5 = load_bitmap("lucius5.bmp", the_palette);
        if (!lucius5)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }


        lucius6 = load_bitmap("lucius6.bmp", the_palette);
        if (!lucius6)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius7 = load_bitmap("lucius7.bmp", the_palette);
        if (!lucius7)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }


        lucius8 = load_bitmap("lucius8.bmp", the_palette);
        if (!lucius8)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius9 = load_bitmap("lucius9.bmp", the_palette);
        if (!lucius9)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius10 = load_bitmap("lucius10.bmp", the_palette);
        if (!lucius10)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        lucius11 = load_bitmap("lucius11.bmp", the_palette);
        if (!lucius11)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }


        lucius12 = load_bitmap("lucius12.bmp", the_palette);
        if (!lucius12)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading player sprite bitmap\n");
                return 1;
        }

        return 0;

}


//********************
//Name:         load_file()
//Parameters:   void
//Return:       int
//********************

int load_file(char loadfile[80])
{
        FILE *level;
        level = fopen(loadfile, "r");

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&bottombmp[x][y], sizeof(int), 1, level);
                }
        }
        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&midbmp[x][y], sizeof(int), 1, level);
                }
        }
        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {

                        fread(&mid2bmp[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&hibmp[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&earthmap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&watermap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&windmap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&firemap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&electricmap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&woodmap[x][y], sizeof(int), 1, level);
                }
        }


        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&collision[x][y], sizeof(bool), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fread(&entitymap[x][y], sizeof(int), 1, level);
                }
        }

        for (int i=0;i<ENTITIES_NUM;i++)
        {
                fread(&entities[i].index, sizeof(int), 1, level);
                fread(&entities[i].x, sizeof(int), 1, level);
                fread(&entities[i].y, sizeof(int), 1, level);
                fread(&entities[i].type, sizeof(int), 1, level);
                fread(&entities[i].warptox, sizeof(int), 1, level);
                fread(&entities[i].warptoy, sizeof(int), 1, level);
                fread(&entities[i].pickupable, sizeof(bool), 1, level);
                fread(&entities[i].activatable, sizeof(bool), 1, level);
                fread(&entities[i].activated, sizeof(bool), 1, level);
                fread(&entities[i].tileid, sizeof(int), 1, level);
        }


        fclose(level);
        return 0;
}

//********************************RENDERING FUNCTIONS***********************//

//********************
//Name:         main_blit()
//Parameters:   void
//Return:       int
//********************


int main_blit(void)
{
        show_mouse(NULL);
        blit(bufferx, screen, 0, 0, 0, 0, 640, 480);
        show_mouse(screen);
}

//********************
//Name:         buffer_blit()
//Parameters:   int scrollx, int scrolly
//Return:       int
//********************

int buffer_blit(int scrollx, int scrolly)
{
        // Draw Interface
        blit(buffer, bufferx, scrollx*20, scrolly*10, 0, 0, 640, 480);
        masked_blit(interface, bufferx, 0, 0, 0, 0, 640, 480);
        masked_blit(elementradar, bufferx, 0, 0, 0, 0, 640, 480);

}

//********************
//Name:         main_menu_blit()
//Parameters:   int main_menu_active
//Return:       int
//********************


int main_menu_blit(int main_menu_active)
{
        if (main_menu_active)
        {
                masked_blit(mainmenugraphic, bufferx, 0, 0, 0, 0, 640, 480);
        }
}


//********************
//Name:         transmute_menu_blit()
//Parameters:   int transmute_menu_active
//Return:       int
//********************
int transmute_menu_blit(int transmute_menu_active)
{
        if (transmute_menu_active)
        {

                masked_blit(transmutemenu, bufferx, 0, 0, 0, 0, 640, 480);
        }

}

//********************
//Name:         highlight_blit()
//Parameters:   int highlight_id
//Return:       int
//********************


int highlight_blit(int highlight_id)
{
        if (highlight_id == HIGHLIGHT_MAINMENU)
                draw_trans_sprite(bufferx, widehighlight, 413, 305);      
        else if (highlight_id == HIGHLIGHT_STATUS)
                draw_trans_sprite(bufferx, widehighlight, 317, 305);
        else if (highlight_id == HIGHLIGHT_INVENTORY)
                draw_trans_sprite(bufferx, widehighlight, 230, 305);
        else if (highlight_id == HIGHLIGHT_GRIMOIRE)
                draw_trans_sprite(bufferx, widehighlight, 135, 305);
        else if (highlight_id == HIGHLIGHT_SACRIFICE)
                draw_trans_sprite(bufferx, smallhighlight, 89, 305);
        else if (highlight_id == HIGHLIGHT_SUMMON)
                draw_trans_sprite(bufferx, smallhighlight, 43, 305);
        else if (highlight_id == HIGHLIGHT_TRANSMUTATION)
                draw_trans_sprite(bufferx, smallhighlight, 7, 305);
}


//********************
//Name:         renderbottom()
//Parameters:   void
//Return:       int
//********************
int renderbottom(void)
{
        int x = 0;
        int y = 0;
        int tile = 0;

        for (x=0;x<100;x++)
        {
                for (y=0;y<100;y++)
                {
                     tile = bottombmp[x][y];
                     if (tile == 1)
                     {
                        stretch_sprite(buffer, floor1, x*20+1, y*10+1, 40, 40);
                     }
                     if (tile == 2)
                     {
                        stretch_sprite(buffer, floor2, x*20+1, y*10+1, 40, 40);
                     }
                     if (tile == 3)
                     {
                        stretch_sprite(buffer, floor3, x*20+1, y*10+1, 40, 40);
                     }
                     if (tile == 4)
                     {
                        stretch_sprite(buffer, floor4, x*20+1, y*10+1, 40, 40);
                     }
                     if (tile == 5)
                     {
                        stretch_sprite(buffer, floor5, x*20+1, y*10+1, 40, 40);
                     }
                }

        } 
}


//********************
//Name:         rendermid()
//Parameters:   void
//Return:       int
//********************


int rendermid(void)
{
}

//********************
//Name:         rendermid2()
//Parameters:   void
//Return:       int
//********************


int rendermid2(void)
{
}

//********************
//Name:         renderhigh()
//Parameters:   void
//Return:       int
//********************


int renderhigh(void)
{
}



//********************
//Name:         renderplayer()
//Parameters:   void
//Return:       int
//********************


int renderplayer(void)
{
       
        if (status == SPRITE_STATUS_WAIT_SOUTH)
        {
                stretch_sprite(buffer, lucius8, playerx*20+1, playery*10+1, 40, 40);
        }

        if (status == SPRITE_STATUS_WAIT_NORTH)
        {
                stretch_sprite(buffer, lucius2, playerx*20+1, playery*10+1, 40, 40);
        }
        if (status == SPRITE_STATUS_WAIT_WEST)
        {
                stretch_sprite(buffer, lucius11, playerx*20+1, playery*10+1, 40, 40);
        }
        if (status == SPRITE_STATUS_WAIT_EAST)
        {
                stretch_sprite(buffer, lucius5, playerx*20+1, playery*10+1, 40, 40);
        }


        if (status == SPRITE_STATUS_MOVE_NORTH)
        {
                if (frame == 0)
                {
                        stretch_sprite(buffer, lucius2, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 1)
                {
                        stretch_sprite(buffer, lucius1, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 2)
                {
                        stretch_sprite(buffer, lucius2, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 3)
                {
                        stretch_sprite(buffer, lucius3, playerx*20+1, playery*10+1, 40, 40);
                }
        }        

        if (status == SPRITE_STATUS_MOVE_SOUTH)
        {
                if (frame == 0)
                {
                        stretch_sprite(buffer, lucius8, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 1)
                {
                        stretch_sprite(buffer, lucius7, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 2)
                {
                        stretch_sprite(buffer, lucius8, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 3)
                {
                        stretch_sprite(buffer, lucius9, playerx*20+1, playery*10+1, 40, 40);
                }
        }
        if (status == SPRITE_STATUS_MOVE_WEST)
        {
                if (frame == 0)
                {
                        stretch_sprite(buffer, lucius11, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 1)
                {
                        stretch_sprite(buffer, lucius10, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 2)
                {
                        stretch_sprite(buffer, lucius11, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 3)
                {
                        stretch_sprite(buffer, lucius12, playerx*20+1, playery*10+1, 40, 40);
                }
        }
        if (status == SPRITE_STATUS_MOVE_EAST)
        {
                if (frame == 0)
                {
                        stretch_sprite(buffer, lucius5, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 1)
                {
                        stretch_sprite(buffer, lucius4, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 2)
                {
                        stretch_sprite(buffer, lucius5, playerx*20+1, playery*10+1, 40, 40);
                }
                else if (frame == 3)
                {
                        stretch_sprite(buffer, lucius6, playerx*20+1, playery*10+1, 40, 40);
                }
        }        




        return 0;
}


//********************
//Name:         element_icon_blit()
//Parameters:   void
//Return:       int
//********************


int element_icon_blit(void)
{
        //Elemental Availability Icons
        if (water_avail)
        {
                stretch_sprite(bufferx, watericon, 530, 23, 15, 15);
        }
        if (wind_avail)
        {
                stretch_sprite(bufferx, windicon, 558, 23, 15, 15);
        }
        if (earth_avail)
        {
                stretch_sprite(bufferx, earthicon, 588, 23, 15, 15);
        }
        if (fire_avail)
        {
                stretch_sprite(bufferx, fireicon, 530, 56, 15, 15);
        }
        if (electric_avail)
        {
                stretch_sprite(bufferx, electricicon, 558, 56, 15, 15);
        }
        if (wood_avail)
        {
                stretch_sprite(bufferx, woodicon, 588, 56, 15, 15);
        }


        //Transmute Menu Availability Icons


}


//********************
//Name:         element_menu_icon_blit()
//Parameters:   void
//Return:       int
//********************


int element_menu_icon_blit(void)
{
        set_trans_blender(0, 0, 0, 200);
        if (!fire_avail)
                draw_trans_sprite(bufferx, greyedout, 40, 120);
        if (!earth_avail)
                draw_trans_sprite(bufferx, greyedout, 75, 120);
        if (!water_avail)
                draw_trans_sprite(bufferx, greyedout, 108, 120);
        if (!wind_avail)
                draw_trans_sprite(bufferx, greyedout, 141, 120);
        if (!electric_avail)
                draw_trans_sprite(bufferx, greyedout, 174, 120);
        if (!wood_avail)
                draw_trans_sprite(bufferx, greyedout, 206, 120);
                

        set_trans_blender(0, 0, 0, 100);

         return 0;
}

//************************PLAYER FUNCTIONS********************************//

//********************
//Name:         center_on_player()
//Parameters:   void
//Return:       int
//********************


int center_on_player(void)
{

        int playerindex = 0;

        playerindex = find_player();
        scrollx = entities[playerindex].x - 15;
        scrolly = entities[playerindex].y - 20;

        playerx = entities[playerindex].x;
        playery = entities[playerindex].y;

        return 0;
}

//********************
//Name:         find_player()
//Parameters:   void
//Return:       int
//********************
int find_player(void)
{
        for (int i=0;i<ENTITIES_NUM;i++)
        {
                if (entities[i].type == ENTITY_PLAYERSTART)
                        return i;

        }
        return 0;
}



//**********************INPUT FUNCTIONS**********************************//


//********************
//Name:         keyboard_input()
//Parameters:   void
//Return:       int
//********************


int keyboard_input(void)
{

        if (key[KEY_W])
        {
                status = SPRITE_STATUS_MOVE_NORTH;                                
        }
        if (key[KEY_S])
        {
                status = SPRITE_STATUS_MOVE_SOUTH;                                
        }
        if (key[KEY_A])
        {
                status = SPRITE_STATUS_MOVE_WEST;
        }
        if (key[KEY_D])
        {
                status = SPRITE_STATUS_MOVE_EAST;
        }


        return 0;
}


//***********************ANIMATION FUNCTIONS******************************//


//********************
//Name:         player_animation()
//Parameters:   void
//Return:       int
//********************


int player_animation(void)
{

 if (status == SPRITE_STATUS_MOVE_NORTH)
 {
  if (frame < 3)
  {
        frame++;
        if (no_collision(playerx, playery-1))
        {
                playery--;
                scrolly--;
        }

  }
  else
  {
        frame = 0;
        status = SPRITE_STATUS_WAIT_NORTH;
  }
 }

 if (status == SPRITE_STATUS_MOVE_SOUTH)
 {
  if (frame < 3)
  {
        frame++;
        if (no_collision(playerx, playery+1))
        {
                playery++;
                scrolly++;
        }
  }
  else
  {
        frame = 0;
        status = SPRITE_STATUS_WAIT_SOUTH;
  }
 }
 if (status == SPRITE_STATUS_MOVE_WEST)
 {
  if (frame < 3)
  {
        frame++;
        if (no_collision(playerx-1, playery))
        {
                playerx--;
                scrollx--;
        }
  }
  else
  {
        frame = 0;
        status = SPRITE_STATUS_WAIT_WEST;
  }
 
 }
 if (status == SPRITE_STATUS_MOVE_EAST)
 {
  if (frame < 3)
  {
        frame++;
        if (no_collision(playerx+1, playery))
        {

                playerx++;
                scrollx++;
        }
  }
  else
  {
        frame = 0;
        status = SPRITE_STATUS_WAIT_EAST;
  }
 }


}



//**************************COLLISION DETECTION FUNCTIONS****************//

//********************
//Name:         no_collision()
//Parameters:   void
//Return:       bool
//********************


bool no_collision(int x, int y)
{
        if (collision[x][y])
                return false;

        else
                return true;
}


//****************************ELEMENT AVAILBILITY FUNCTIONS****************//

//********************
//Name:         check_element_avail()
//Parameters:   void
//Return:       int
//********************


int check_element_avail(void)
{

 //water
 if (watermap[playerx][playery] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx+1][playery] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx+1][playery+1] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx][playery+1] == true)
 {
        water_avail = true;
 }

 else if (watermap[playerx-1][playery] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx-1][playery-1] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx][playery-1] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx-1][playery+1] == true)
 {
        water_avail = true;
 }
 else if (watermap[playerx][playery-1] == true)
 {
        water_avail = true;
 }
 else
 {
        water_avail = false;
 }
 

 //wind
 if (windmap[playerx][playery] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx+1][playery] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx+1][playery+1] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx][playery+1] == true)
 {
        wind_avail = true;
 }

 else if (windmap[playerx-1][playery] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx-1][playery-1] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx][playery-1] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx-1][playery+1] == true)
 {
        wind_avail = true;
 }
 else if (windmap[playerx+1][playery-1] == true)
 {
        wind_avail = true;
 }
 else
 {
        wind_avail = false;
 }


 //earth
 if (earthmap[playerx][playery] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx+1][playery] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx+1][playery+1] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx][playery+1] == true)
 {
        earth_avail = true;
 }

 else if (earthmap[playerx-1][playery] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx-1][playery-1] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx][playery-1] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx-1][playery+1] == true)
 {
        earth_avail = true;
 }
 else if (earthmap[playerx+1][playery-1] == true)
 {
        earth_avail = true;
 }
 else
 {
        earth_avail = false;
 }


 //fire
 if (firemap[playerx][playery] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx+1][playery] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx+1][playery+1] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx][playery+1] == true)
 {
        fire_avail = true;
 }

 else if (firemap[playerx-1][playery] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx-1][playery-1] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx][playery-1] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx-1][playery+1] == true)
 {
        fire_avail = true;
 }
 else if (firemap[playerx+1][playery-1] == true)
 {
        fire_avail = true;
 }
 else
 {
        fire_avail = false;
 }


 //electric
 if (electricmap[playerx][playery] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx+1][playery] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx+1][playery+1] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx][playery+1] == true)
 {
        electric_avail = true;
 }

 else if (electricmap[playerx-1][playery] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx-1][playery-1] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx][playery-1] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx-1][playery+1] == true)
 {
        electric_avail = true;
 }
 else if (electricmap[playerx+1][playery-1] == true)
 {
        electric_avail = true;
 }
 else
 {
        electric_avail = false;
 }


 //wood
 if (woodmap[playerx][playery] == true)
 {
        wood_avail = true;
 }
 else if (woodmap[playerx+1][playery] == true)
 {
        wood_avail = true;
 }
 else if (woodmap[playerx+1][playery+1] == true)
 {
        wood_avail = true;
 }
 else if (woodmap[playerx][playery+1] == true)
 {
        wood_avail = true;
 }      
 else if (woodmap[playerx-1][playery] == true)
 {
        wood_avail = true;
 }
 else if (woodmap[playerx-1][playery-1] == true)
 {
       wood_avail = true;
 }
 else if (woodmap[playerx][playery-1] == true)
 {
        wood_avail = true;
 }
 else if (woodmap[playerx-1][playery+1] == true)
 {
        wood_avail = true;
 }
 else if (woodmap[playerx+1][playery-1] == true)
 {
        wood_avail = true;
 }
 else
 {
        wood_avail = false;
 }

        return 0;
}



