// Project:     Hexagram Editor
// Filename:    hexeditor.cpp
// Author:      Ted Gress
// Platform:    DOS - Windows XP/2000

//********************CONSTANTS**************************//

// Level Map Identifier Constants

#define LEVEL_BOTTOM  1
#define LEVEL_MIDDLE  2
#define LEVEL_MIDDLE2 3
#define LEVEL_TOP     4
#define LEVEL_ENTITIES 5
#define LEVEL_COLLISION 6
#define LEVEL_ELEMENTS 7

// Entity Struct Identifier Constants
#define ENTITY_PLAYERSTART 1
#define ENTITY_WARP 2

// Number of entities possible
#define ENTITIES_NUM 50 

//Tile id defines for element blocks
#define TILE_EARTH 1
#define TILE_WATER 2
#define TILE_WIND 3
#define TILE_FIRE 4
#define TILE_ELECTRIC 5
#define TILE_WOOD 6

//********************INCLUDES***************************//

#include <iostream>
#include <allegro.h>

using namespace std;

//********************PROTOTYPES*************************//



// Main functions

int init(void);
int destroy(void);
int main_loop(void);

// Bitmap loading functions

int load_floor_tiles(void);
int load_entities(void);
int load_colblocks(void);
int load_elementicons(void);


// Display Functions
int display_main_menu(void);
int display_bottom_menu(int tileid, int tilex, int tily);
int display_entity_menu(int tileid, int tilex, int tiley);
int display_entity_struct_menu(int entityindex);
int display_collision_menu(int tilex, int tiley);
int display_element_menu(int tileid, int tilex, int tiley);


// Placement Functions
int place_bottom(void);
int place_entity(void);
int place_elementblock(void);
int place_colblock(void);

int edit_entity(void);

// Entity Editing Functions
int change_entity_type(int entityindex);

// Blitting Functions
int main_blit(void);
int buffer_blit(int scrollx, int scrolly);
int renderbottom(void);
int rendermid(void);
int rendermid2(void);
int renderhigh(void);
int renderentities(void);
int rendercursor(int level, int tileid, int tilex, int tiley);
int rendercollision(void);
int renderelementblocks(void);

//Save and Load Functions
int save_map(void);
int load_map(void);
int save_file(char savefile[80]);
int load_file(char loadfile[80]);


//********************GLOBALS****************************//

BITMAP *buffer;
BITMAP *bufferx;
BITMAP *entity_graphic;
BITMAP *colblock;
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


//Element Icons
BITMAP *watericon;
BITMAP *windicon;
BITMAP *earthicon;
BITMAP *fireicon;
BITMAP *electricicon;
BITMAP *woodicon;


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

//Tile Control Values
int tileid = 1;
int tilex = 0;
int tiley = 0;

//Scroll Control Values
int scrollx = 0;
int scrolly = 0;

//******************************MAIN***********************************//

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
        clear(bufferx);
        clear(screen);

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
        bool  control = true;
    

        while (control)
        {
                clear(buffer);
                clear(bufferx);

                renderbottom();
                rendermid();
                rendermid2();
                renderhigh();
                renderentities();
                buffer_blit(0, 0);
                display_main_menu();
                //Blit buffer to screen
                main_blit();

                if (key[KEY_1])
                {
                        place_bottom();
                }
                if (key[KEY_5])
                {
                        place_entity();
                }

                if (key[KEY_6])
                {
                        edit_entity();
                }
                if (key[KEY_7])
                {
                        place_colblock();
                }
                if (key[KEY_8])
                {
                        place_elementblock();
                }
                if (key[KEY_S])
                {
                        save_map();
                }
                if (key[KEY_L])
                {
                        load_map();
                }
                if (key[KEY_ESC])
                {
                        control = false;
                }
        }

        return 0;
}

//*************************DISPLAY MENUS***********************************//

//********************
//Name:         display_main_menu()
//Parameters:   void
//Return:       int
//*********************

int display_main_menu(void)
{
        textout_ex(bufferx, font, "1. Place a new floor tile", 10, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "2. Place a new mid tile", 10, 20, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "3. Place a new mid2 tile", 10, 30, makecol(0,0, 255), -1);
        textout_ex(bufferx, font, "4. Place a new ceiling tile", 10, 40, makecol(0, 0, 255), -1);
        textout_ex(bufferx, font, "5. Place an entity", 10, 50, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "6. Edit an entity", 10, 60, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "7. Place a collision block", 10, 70, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "8. Place an element block", 10, 80, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "(S) Saves a map", 10, 90, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "(L) Loads a map", 10, 100, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "Escape to exit", 10, 110, makecol(0,0,255),-1);
}

//*********************
//Name:         display_bottom_menu()
//Parameters:   int tileid, int tilex, int tiley
//Return:       int
//**********************

int display_bottom_menu(int tileid, int tilex, int tiley)
{
        char tmp[80];
        textout_ex(bufferx, font, "-Bottom Tile Layer-", 5, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "-M/N cycles through tiles, Q to quit-", 5, 20, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "ENTER places tile in location, C clears the location", 5, 30, makecol(0,0,255), -1);
        sprintf(tmp, "Tile id is %d", tileid);
        textout_ex(bufferx, font, tmp, 5, 200, makecol(0,0,255), -1);
        sprintf(tmp, "Tile X is %d, Tile Y is %d", tilex, tiley);
        textout_ex(bufferx, font, tmp, 5, 210, makecol(0,0,255), -1);

        return 0;
}

//*********************
//Name:         display_entity_menu()
//Parameters:   int tileid, int tilex, int tiley
//Return:       int
//**********************
int display_entity_menu(int tileid, int tilex, int tiley)
{
        char tmp[80];
        textout_ex(bufferx, font, "-Entity Layer-", 5, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "-M/N cycles through entities, Q to quit-", 5, 20, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "ENTER places tile in location, C clears the location", 5, 30, makecol(0,0,255), -1);
        sprintf(tmp, "Entity id is %d", tileid);
        textout_ex(bufferx, font, tmp, 5, 200, makecol(0,0,255), -1);
        sprintf(tmp, "Entity X is %d, Entity Y is %d", tilex, tiley);
        textout_ex(bufferx, font, tmp, 5, 210, makecol(0,0,255), -1);

        return 0;
}

//*********************
//Name:         display_entity_struct_menu()
//Parameters:   int entityindex
//Return:       int
//**********************


int display_entity_struct_menu(int entityindex)
{
        char tmp[80];
        textout_ex(bufferx, font, "-Entity Struct Menu-", 5, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "1. Change Type", 5, 20, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "2. Change WarpTo Coordinates", 5, 30, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "3. Change Pickupable", 5, 40, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "4. Change Activatable", 5, 50, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "4. Select Tile to represent the entity", 5, 60, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "-M/N cycles through entities, Q to quit-", 5, 70, makecol(0,0,255), -1);
        sprintf(tmp, "Current Entity indx   %d", entityindex);
        textout_ex(bufferx, font, tmp, 5, 90, makecol(0,0,255),-1);
        sprintf(tmp, "Saved Entity index is %d", entities[entityindex].index);
        textout_ex(bufferx, font, tmp, 5, 100, makecol(0,0,255),-1);
        sprintf(tmp, "X Coordinate is %d      ", entities[entityindex].x);
        textout_ex(bufferx, font, tmp, 5, 110, makecol(0,0,255),-1);
        sprintf(tmp, "Y Coordinate is %d      ", entities[entityindex].y);
        textout_ex(bufferx, font, tmp, 5, 120, makecol(0,0,255),-1);
        sprintf(tmp, "Type is %d              ", entities[entityindex].type);
        textout_ex(bufferx, font, tmp, 5, 130, makecol(0,0,255),-1);
        sprintf(tmp, "Warpto X is %d          ", entities[entityindex].warptox);
        textout_ex(bufferx, font, tmp, 5, 140, makecol(0,0,255),-1);
        sprintf(tmp, "Warpto Y is %d          ", entities[entityindex].warptoy);
        textout_ex(bufferx, font, tmp, 5, 150, makecol(0,0,255),-1);
        sprintf(tmp, "Pickupable is %d        ", entities[entityindex].pickupable);
        textout_ex(bufferx, font, tmp, 5, 160, makecol(0,0,255),-1);
        sprintf(tmp, "Activatable is %d       ", entities[entityindex].activatable);
        textout_ex(bufferx, font, tmp, 5, 170, makecol(0,0,255),-1);
        sprintf(tmp, "Tile Bitmap ID is %d    ", entities[entityindex].tileid);
        textout_ex(bufferx, font, tmp, 5, 180, makecol(0,0,255),-1);
        return 0;
}


//*********************
//Name:         display_collision_menu()
//Parameters:   void
//Return:       int
//**********************



int display_collision_menu(int tilex, int tiley)
{
        char tmp[80];
        textout_ex(bufferx, font, "-Collision Block Layer-", 5, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "Press Q to quit-", 5, 20, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "ENTER places tile in location, C clears the location", 5, 30, makecol(0,0,255), -1);

        sprintf(tmp, "Entity X is %d, Entity Y is %d", tilex, tiley);
        textout_ex(bufferx, font, tmp, 5, 40, makecol(0,0,255), -1);

        return 0;
}

//*********************
//Name:         display_element_menu()
//Parameters:   int tileid, int tilex, int tiley
//Return:       int
//**********************


int display_element_menu(int tileid, int tilex, int tiley)
{
        char tmp[80];
        textout_ex(bufferx, font, "-Element Tile Layer-", 5, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "-M/N cycles through elements, Q to quit-", 5, 20, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "ENTER places tile in location, C clears the location", 5, 30, makecol(0,0,255), -1);
        sprintf(tmp, "Tile id is %d", tileid);
        textout_ex(bufferx, font, tmp, 5, 200, makecol(0,0,255), -1);
        sprintf(tmp, "Tile X is %d, Tile Y is %d", tilex, tiley);
        textout_ex(bufferx, font, tmp, 5, 210, makecol(0,0,255), -1);
}

//******************************INITIALIZATION AND LOADING******************//

//*******************
//Name:         init()
//Parameters:   void
//Return:       int
//*******************


int init(void)
{
        cout << "HEXAGRAM Editor Alpha Version 1.0" << endl;
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

        load_floor_tiles();
        load_entities();
        load_colblocks();
        load_elementicons();
        create_trans_table(&trans_table, the_palette, 128, 128, 128, NULL);
        buffer = create_bitmap(1280, 960);
        bufferx = create_bitmap(640, 480);
        return 0;
}

//*******************
//Name:         destroy()
//Parameters:   void
//Return:       int
//*******************


int destroy(void)
{
        destroy_bitmap(floor1);
        destroy_bitmap(floor2);
        destroy_bitmap(floor3);
        destroy_bitmap(floor4);
        destroy_bitmap(floor5);

        destroy_bitmap(entity_graphic);
        destroy_bitmap(colblock);
        destroy_bitmap(watericon);
        destroy_bitmap(earthicon);
        destroy_bitmap(windicon);
        destroy_bitmap(fireicon);
        destroy_bitmap(electricicon);
        destroy_bitmap(woodicon);


        destroy_bitmap(buffer);
        destroy_bitmap(bufferx);
        return 0;
}

//*******************
//Name:         load_colblocks(void)
//Parameters:   void
//Return:       int
//*******************


int load_colblocks(void)
{
        colblock = load_bitmap("colblock.bmp", the_palette);
        if (!colblock)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading collision block bitmap\n");
                return 1;
        }
}

//*******************
//Name:         load_floor_tiles(void)
//Parameters:   void
//Return:       int
//*******************


int load_floor_tiles(void)
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

}

//********************
//Name:         load_entities()
//Parameters:   void
//Return:       int
//********************


int load_entities(void)
{
        entity_graphic = load_bitmap("entity.bmp", the_palette);
        if (!entity_graphic)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading entity bitmap\n");
                return 1;
        }
}



//********************
//Name:         load_elementicons()
//Parameters:   void
//Return:       int
//********************

int load_elementicons(void)
{
        watericon = load_bitmap("waterediticon.bmp", the_palette);
        if (!watericon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading water icon bitmap\n");
                return 1;
        }

        windicon = load_bitmap("windediticon.bmp", the_palette);
        if (!windicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading wind icon bitmap\n");
                return 1;
        }

        earthicon = load_bitmap("earthediticon.bmp", the_palette);
        if (!earthicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading earth icon bitmap\n");
                return 1;
        }

        fireicon = load_bitmap("fireediticon.bmp", the_palette);
        if (!fireicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading fire icon bitmap\n");
                return 1;
        }

        electricicon = load_bitmap("electricediticon.bmp", the_palette);
        if (!electricicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading electric icon bitmap\n");
                return 1;
        }

        woodicon = load_bitmap("woodediticon.bmp", the_palette);
        if (!woodicon)
        {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error loading wood icon bitmap\n");
                return 1;
        }


        return 0;

}
//******************************PLACEMENT FUNCTIONS*************************//

//********************
//Name:         place_bottom()
//Parameters:   void
//Return:       int
//********************

int place_bottom(void)
{
 bool control = true;

 while (control)
 {
        if (key[KEY_N])
        {
                if (tileid > 1)
                {
                        tileid--;
                }
        }
        if (key[KEY_M])
        {
              tileid++;
        }
        if (key[KEY_LEFT])
        {
                if (tilex != 0)
                {
                        tilex--;
                }
                if (tilex < scrollx)
                {
                        scrollx--;
                }
        }
        if (key[KEY_RIGHT])
        {
                if (tilex != 62)
                {
                        tilex++;
                        if (tilex > (30+scrollx))
                        {
                                scrollx++;
                        }
                 }
        }
        if (key[KEY_UP])
        {
                if (tiley > 0)
                {
                        tiley--;
                }
                if (tiley < scrolly)
                {
                        scrolly--;
                }
        }
        if (key[KEY_DOWN])
        {
                if (tiley != 92)
                {
                        tiley++;
                        if (tiley >= 44+scrolly)
                        {
                                scrolly++;
                        }
                }
        }
        if (key[KEY_ENTER])
        {
                bottombmp[tilex][tiley] = tileid;
        }
        if (key[KEY_C])
        {
                bottombmp[tilex][tiley] = 0;
        }
        if (key[KEY_Q])
        {
                control = false;
        }
        clear(buffer);
        clear(bufferx);
        renderbottom();
        rendermid();
        rendermid2();
        renderhigh();
        renderentities();
        rendercursor(LEVEL_BOTTOM, tileid, tilex, tiley);
        buffer_blit(scrollx, scrolly);
        display_bottom_menu(tileid, tilex, tiley);
        main_blit();
 }
 return 0;
}

//********************
//Name:         place_entity()
//Parameters:   void
//Return:       int
//********************


int place_entity(void)
{
 bool control = true;

 while (control)
 {
        if (key[KEY_N])
        {
                if (tileid > 1)
                {
                        tileid--;
                }
        }
        if (key[KEY_M])
        {
              tileid++;
        }
        if (key[KEY_LEFT])
        {
                if (tilex != 0)
                {
                        tilex--;
                }
                if (tilex < scrollx)
                {
                        scrollx--;
                }
        }
        if (key[KEY_RIGHT])
        {
                if (tilex != 62)
                {
                        tilex++;
                        if (tilex > (30+scrollx))
                        {
                                scrollx++;
                        }
                 }
        }
        if (key[KEY_UP])
        {
                if (tiley > 0)
                {
                        tiley--;
                }
                if (tiley < scrolly)
                {
                        scrolly--;
                }
        }
        if (key[KEY_DOWN])
        {
                if (tiley != 92)
                {
                        tiley++;
                        if (tiley >= 44+scrolly)
                        {
                                scrolly++;
                        }
                }
        }
        if (key[KEY_ENTER])
        {
                entitymap[tilex][tiley] = tileid;
                entities[tileid].index = tileid;
                entities[tileid].x = tilex;
                entities[tileid].y = tiley;
        }
        if (key[KEY_C])
        {
                entitymap[tilex][tiley] = 0;
                entities[tileid].index = 0;
                entities[tileid].x = 0;
                entities[tileid].y = 0;
        }
        if (key[KEY_Q])
        {
                control = false;
        }
        clear(buffer);
        clear(bufferx);
        renderbottom();
        rendermid();
        rendermid2();
        renderhigh();
        renderentities();
        rendercursor(LEVEL_ENTITIES, tileid, tilex, tiley);
        buffer_blit(scrollx, scrolly);
        display_bottom_menu(tileid, tilex, tiley);
        main_blit();
 }
 return 0;


}

//********************
//Name:         edit_entity()
//Parameters:   void
//Return:       int
//********************


int edit_entity(void)
{
 bool control = true;

 while (control)
 {
        if (key[KEY_N])
        {
                if (tileid > 1)
                {
                        tileid--;
                }
        }
        if (key[KEY_M])
        {
                tileid++;
        }
        if (key[KEY_1])
        {
                change_entity_type(tileid);
        }
        if (key[KEY_Q])
        {
                control = false;
        }

        clear(buffer);
        clear(bufferx);
        renderbottom();
        rendermid();
        rendermid2();
        renderhigh();
        renderentities();
        rendercursor(LEVEL_ENTITIES, tileid, tilex, tiley);
        buffer_blit(scrollx, scrolly);
        display_entity_struct_menu(tileid);
        main_blit();
 }
 return 0;
}


//********************
//Name:         place_colblock()
//Parameters:   void
//Return:       int
//********************


int place_colblock(void)
{
 bool control = true;

 while (control)
 {
        if (key[KEY_LEFT])
        {
                if (tilex != 0)
                {
                        tilex--;
                }
                if (tilex < scrollx)
                {
                        scrollx--;
                }
        }
        if (key[KEY_RIGHT])
        {
                if (tilex != 62)
                {
                        tilex++;
                        if (tilex > (30+scrollx))
                        {
                                scrollx++;
                        }
                 }
        }
        if (key[KEY_UP])
        {
                if (tiley > 0)
                {
                        tiley--;
                }
                if (tiley < scrolly)
                {
                        scrolly--;
                }
        }
        if (key[KEY_DOWN])
        {
                if (tiley != 92)
                {
                        tiley++;
                        if (tiley >= 44+scrolly)
                        {
                                scrolly++;
                        }
                }
        }
        if (key[KEY_ENTER])
        {
                collision[tilex][tiley] = true;
        }
        if (key[KEY_C])
        {
                collision[tilex][tiley] = false;
        }
        if (key[KEY_Q])
        {
                control = false;
        }
        clear(buffer);
        clear(bufferx);
        renderbottom();
        rendermid();
        rendermid2();
        renderhigh();
        renderentities();
        rendercollision();
        rendercursor(LEVEL_COLLISION, tileid, tilex, tiley);
        buffer_blit(scrollx, scrolly);
        display_collision_menu(tilex, tiley);
        main_blit();
 }
 return 0;


}

int place_elementblock(void)
{

 bool control = true;

 while (control)
 {
        if (key[KEY_N])
        {
                if (tileid > 1)
                {
                        tileid--;
                }
        }
        if (key[KEY_M])
        {
                if (tileid < 6)
                {
                        tileid++;
                }
        }
        if (key[KEY_LEFT])
        {
                if (tilex != 0)
                {
                        tilex--;
                }
                if (tilex < scrollx)
                {
                        scrollx--;
                }
        }
        if (key[KEY_RIGHT])
        {
                if (tilex != 62)
                {
                        tilex++;
                        if (tilex > (30+scrollx))
                        {
                                scrollx++;
                        }
                 }
        }
        if (key[KEY_UP])
        {
                if (tiley > 0)
                {
                        tiley--;
                }
                if (tiley < scrolly)
                {
                        scrolly--;
                }
        }
        if (key[KEY_DOWN])
        {
                if (tiley != 92)
                {
                        tiley++;
                        if (tiley >= 44+scrolly)
                        {
                                scrolly++;
                        }
                }
        }
        if (key[KEY_ENTER])
        {

                if (tileid == TILE_EARTH)
                {
                        earthmap[tilex][tiley] = true;
                }
                else if (tileid == TILE_WATER)
                {
                        watermap[tilex][tiley] = true;
                }
                else if (tileid == TILE_WIND)
                {
                        windmap[tilex][tiley] = true;
                }
                else if (tileid == TILE_FIRE)
                {
                        firemap[tilex][tiley] = true;
                }
                else if (tileid == TILE_ELECTRIC)
                {
                        electricmap[tilex][tiley] = true;
                }
                else if (tileid == TILE_WOOD)
                {
                        woodmap[tilex][tiley] = true;
                }
        }
        if (key[KEY_C])
        {
                if (tileid == TILE_EARTH)
                {
                        earthmap[tilex][tiley] = false;
                }
                else if (tileid == TILE_WATER)
                {
                        watermap[tilex][tiley] = false;
                }
                else if (tileid == TILE_WIND)
                {
                        windmap[tilex][tiley] = false;
                }
                else if (tileid == TILE_FIRE)
                {
                        firemap[tilex][tiley] = false;
                }
                else if (tileid == TILE_ELECTRIC)
                {
                        electricmap[tilex][tiley] = false;
                }
                else if (tileid == TILE_WOOD)
                {
                        woodmap[tilex][tiley] = false;
                }

        }
        if (key[KEY_Q])
        {
                control = false;
        }
        clear(buffer);
        clear(bufferx);
        renderbottom();
        rendermid();
        rendermid2();
        renderhigh();
        renderentities();
        renderelementblocks();
        rendercursor(LEVEL_ELEMENTS, tileid, tilex, tiley);
        buffer_blit(scrollx, scrolly);
        display_element_menu(tileid, tilex, tiley);
        main_blit();
 }
 return 0;


}


//******************************RENDERING FUNCTIONS************************//

//********************
//Name:         main_blit(void)
//Parameters:   void
//Return:       int
//********************

int main_blit(void)
{
        blit(bufferx, screen, 0, 0, 0, 0, 640, 480);       
}

//********************
//Name:         buffer_blit()
//Parameters:   int scrollx, int scrolly
//Return:       int
//********************

int buffer_blit(int scrollx, int scrolly)
{
        blit(buffer, bufferx, scrollx*20, scrolly*10, 0, 0, 640, 480);
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
//Name:         renderentities()
//Parameters:   void
//Return:       int
//********************


int renderentities(void)
{
        int x = 0;
        int y = 0;
        int tile = 0;

        for (x=0;x<100;x++)
        {
                for (y=0;y<100;y++)
                {
                        tile = entitymap[x][y];
                        if (tile == 1)
                        {
                                textout_ex(bufferx, font, "1", x*20+1, y*10+1, makecol(0,0,255), -1);
                                stretch_sprite(buffer, entity_graphic, x*20+1, y*10+1, 40, 40);
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
//Name:         rendercollision()
//Parameters:   void
//Return:       int
//********************



int rendercollision(void)
{
        int x = 0;
        int y = 0;


        for (x=0;x<100;x++)
        {
                for (y=0;y<100;y++)
                {
                        if (collision[x][y] == true)
                        {
                                stretch_sprite(buffer, colblock, x*20+1, y*10+1, 40, 40);
                        }
                }

        } 

}

//********************
//Name:         rendercursor()
//Parameters:   int level, int tileid, int tilex, int tiley
//Return:       int
//********************


int rendercursor(int level, int tileid, int tilex, int tiley)
{
        int x = tilex*20+1;
        int y = tiley*10+1;

        if (level == LEVEL_BOTTOM)
        {
                if (tileid == 1)
                {
                        stretch_sprite(buffer, floor1, x, y, 40, 40);
                }
                if (tileid == 2)
                {
                        stretch_sprite(buffer, floor2, x, y, 40, 40);
                }
                if (tileid == 3)
                {
                        stretch_sprite(buffer, floor3, x, y, 40, 40);
                }
                if (tileid == 4)
                {
                        stretch_sprite(buffer, floor4, x, y, 40, 40);
                }
                if (tileid == 5)
                {
                        stretch_sprite(buffer, floor5, x, y, 40, 40);
                }
        }
        if (level == LEVEL_ENTITIES)
        {
                stretch_sprite(buffer, entity_graphic, x, y, 40, 40);
        }
        if (level == LEVEL_COLLISION)
        {
                stretch_sprite(buffer, colblock, x, y, 40, 40);
        }
        if (level == LEVEL_ELEMENTS)
        {
                if (tileid == TILE_EARTH)
                {
                        stretch_sprite(buffer, earthicon, x, y, 40, 40);
                }
                else if (tileid == TILE_WATER)
                {
                        stretch_sprite(buffer, watericon, x, y, 40, 40);
                }
                else if (tileid == TILE_WIND)
                {
                        stretch_sprite(buffer, windicon, x, y, 40, 40);
                }
                else if (tileid == TILE_FIRE)
                {
                        stretch_sprite(buffer, fireicon, x, y, 40, 40);
                }
                else if (tileid == TILE_ELECTRIC)
                {
                        stretch_sprite(buffer, electricicon, x, y, 40, 40);
                }
                else if (tileid == TILE_WOOD)
                {
                        stretch_sprite(buffer, woodicon, x, y, 40, 40);
                }
        }
}




//********************
//Name:         renderelementblocks()
//Parameters:   void
//Return:       int
//********************


int renderelementblocks(void)
{
        int x = 0;
        int y = 0;

        for (x=0;x<100;x++)
        {
                for (y=0;y<100;y++)
                {

                     if (watermap[x][y] == true)
                     {
                        stretch_sprite(buffer, watericon, x*20+1, y*10+1, 40, 40);
                     }

                     if (windmap[x][y] == true)
                     {
                        stretch_sprite(buffer, windicon, x*20+1, y*10+1, 40, 40);
                     }


                     if (earthmap[x][y] == true)
                     {
                        stretch_sprite(buffer, earthicon, x*20+1, y*10+1, 40, 40);
                     }

                     if (firemap[x][y] == true)
                     {
                        stretch_sprite(buffer, fireicon, x*20+1, y*10+1, 40, 40);
                     }

                     if (electricmap[x][y] == true)
                     {
                        stretch_sprite(buffer, electricicon, x*20+1, y*10+1, 40, 40);
                     }

                     if (woodmap[x][y] == true)
                     {
                        stretch_sprite(buffer, woodicon, x*20+1, y*10+1, 40, 40);
                     }

                }


        } 

}

//***************************ENTITY EDITING********************************//

//********************
//Name:         change_entity_type()
//Parameters:   int entityindex
//Return:       int
//********************



int change_entity_type(int entityindex)
{
 char tmp[80];
 bool control = true;
 clear_keybuf();

 while (control)
 {
        if (key[KEY_1])
        {
                entities[entityindex].type = ENTITY_PLAYERSTART;
        }
        if (key[KEY_2])
        {
                entities[entityindex].type = ENTITY_WARP;
        }
        if (key[KEY_ESC])
        {
                control = false;
        }

        clear(buffer);
        clear(bufferx);
        renderbottom();
        rendermid();
        rendermid2();
        renderhigh();
        renderentities();
        rendercursor(LEVEL_ENTITIES, tileid, tilex, tiley);
        buffer_blit(scrollx, scrolly);

        textout_ex(bufferx, font, "-Change Entity Type Menu-", 5, 10, makecol(0,0,255), -1);
        textout_ex(bufferx, font, "-Press Escape to Quit-", 5, 20, makecol(0,0,255),-1);
        sprintf(tmp, "-Entity Type is %d-", entities[entityindex].type);
        textout_ex(bufferx, font, tmp, 5, 30, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "Select Entity Type:", 5, 40, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "1. ENTITY_PLAYERSTART", 5, 50, makecol(0,0,255),-1);
        textout_ex(bufferx, font, "2. ENTITY_WARP", 5, 60, makecol(0,0,255),-1);

        main_blit();
 }
        return 0;
}

//***************************SAVE AND LOAD FUNCTIONS************************//

//********************
//Name:         save_map()
//Parameters:   void
//Return:       int
//********************

int save_map(void)
{
        char savefile[80];
        char letter;
        int temp = 0;


        clear_keybuf();
        clear(screen);
        for (int i=0;i<13;i++)
        {
                savefile[i] = 0;
        }
        textout_ex(screen, font, "Save- Please enter a filename. Press Enter to finish.", 5, 10, makecol(0,0,255), -1);
        textout_ex(screen, font, "The suggested extension for a map file is .hxm", 5, 20, makecol(0,0,255),-1);
        for (int i=0;i<12;i++)
        {
                temp = readkey();
                if ((temp >> 8) == KEY_ENTER)
                {
                        i = 13;
                }
                savefile[i] = temp & 0xff;
                textout_ex(screen,font,savefile,5, 30, makecol(0,0,255),-1);
        }

        savefile[13] = '\0';
        textout_ex(screen, font, savefile, 5, 30, makecol(0,0,255),-1);
        textout_ex(screen, font, "Are you sure? (y/n)", 5, 40, makecol(0,0,255),-1);
        temp = readkey();
        letter = temp & 0xff;
        if (letter == 'y')
        {
                textout_ex(screen, font, "Saving...", 5, 50, makecol(0,0,255),-1);
                save_file(savefile);
        }

}


//********************
//Name:         save_file()
//Parameters:   void
//Return:       int
//********************

int save_file(char savefile[80])
{
        FILE *level;
        level = fopen(savefile, "w");

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&bottombmp[x][y], sizeof(int), 1, level);
                }
        }
        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&midbmp[x][y], sizeof(int), 1, level);
                }
        }
        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {

                        fwrite(&mid2bmp[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&hibmp[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&earthmap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&watermap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&windmap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&firemap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&electricmap[x][y], sizeof(int), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&woodmap[x][y], sizeof(int), 1, level);
                }
        }


        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&collision[x][y], sizeof(bool), 1, level);
                }
        }

        for (int y=0;y<100;y++)
        {
                for (int x=0;x<100;x++)
                {
                        fwrite(&entitymap[x][y], sizeof(int), 1, level);
                }
        }

        for (int i=0;i<ENTITIES_NUM;i++)
        {
                fwrite(&entities[i].index, sizeof(int), 1, level);
                fwrite(&entities[i].x, sizeof(int), 1, level);
                fwrite(&entities[i].y, sizeof(int), 1, level);
                fwrite(&entities[i].type, sizeof(int), 1, level);
                fwrite(&entities[i].warptox, sizeof(int), 1, level);
                fwrite(&entities[i].warptoy, sizeof(int), 1, level);
                fwrite(&entities[i].pickupable, sizeof(bool), 1, level);
                fwrite(&entities[i].activatable, sizeof(bool), 1, level);
                fwrite(&entities[i].activated, sizeof(bool), 1, level);
                fwrite(&entities[i].tileid, sizeof(int), 1, level);
        }


        fclose(level);
        return 0;
}

//********************
//Name:         load_map()
//Parameters:   void
//Return:       int
//********************

int load_map(void)
{
        char loadfile[80];
        char letter;
        int temp = 0;


        clear_keybuf();
        clear(screen);
        for (int i=0;i<13;i++)
        {
                loadfile[i] = 0;
        }
        textout_ex(screen, font, "Load- Please enter a filename. Press Enter to finish.", 5, 10, makecol(0,0,255), -1);
        textout_ex(screen, font, "The suggested extension for a map file is .hxm", 5, 20, makecol(0,0,255),-1);
        for (int i=0;i<12;i++)
        {
                temp = readkey();
                if ((temp >> 8) == KEY_ENTER)
                {
                        i = 13;
                }
                loadfile[i] = temp & 0xff;
                textout_ex(screen,font,loadfile,5, 30, makecol(0,0,255),-1);
        }

        loadfile[13] = '\0';
        textout_ex(screen, font, loadfile, 5, 30, makecol(0,0,255),-1);
        textout_ex(screen, font, "Are you sure? (y/n)", 5, 40, makecol(0,0,255),-1);
        temp = readkey();
        letter = temp & 0xff;
        if (letter == 'y')
        {
                textout_ex(screen, font, "Loading...", 5, 50, makecol(0,0,255),-1);
                load_file(loadfile);
        }

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

