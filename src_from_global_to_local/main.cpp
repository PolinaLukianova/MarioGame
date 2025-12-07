#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <windows.h>

const int MAP_WIDTH  = 100;
const int MAP_HEIGHT = 25;

struct game_object
{
    float x, y;
    float width, height;

    float vertical_speed;
    float horizontal_speed;

    bool is_flying;
    char type;
};

void clear_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]);
void draw_map(char map[MAP_HEIGHT][MAP_WIDTH + 1]);
void set_object_position(struct game_object *obj, float x_pos, float y_pos);
void init_object(struct game_object *obj, float x_pos, float y_pos, float obj_width, float obj_height, char type);

void create_level(int level, char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                  game_object **bricks, int *bricks_count,
                  game_object **moving_objects, int *moving_objects_count);
void player_died(char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                 game_object **bricks, int *bricks_count,
                 game_object **moving_objects, int *moving_objects_count);

bool is_collision(struct game_object obj_1, struct game_object obj_2);
bool is_pos_inside_map(int x, int y);

struct game_object *create_brick(game_object **bricks, int *bricks_count);
struct game_object *create_moving_object(game_object **moving_objects, int *moving_objects_count);

void move_object_vertically(struct game_object *obj, char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                            game_object *bricks, int bricks_count,
                            game_object **moving_objects, int *moving_objects_count);
void move_object_horizontally(struct game_object *obj, char map[MAP_HEIGHT][MAP_WIDTH + 1],
                              game_object *bricks, int bricks_count,
                              game_object **moving_objects, int *moving_objects_count);
void move_map_horizontally(float delta_x, char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                           game_object *bricks, int bricks_count,
                           game_object *moving_objects, int moving_objects_count);

void check_mario_collision(char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                           game_object *moving_objects, int *moving_objects_count,
                           game_object **bricks, int *bricks_count);
void draw_object_on_map(struct game_object obj, char map[MAP_HEIGHT][MAP_WIDTH + 1]);
void draw_score_on_map(char map[MAP_HEIGHT][MAP_WIDTH + 1], int score);

void set_cursor_position(int x, int y);
void delete_moving_object(int i, game_object **moving_objects, int *moving_objects_count);


int current_level = 1;
int score = 0;
int max_level = 0;

void clear_map(char map[MAP_HEIGHT][MAP_WIDTH + 1])
{
    for (int i = 0; i < MAP_WIDTH; i++)
        map[0][i] = ' ';

    map[0][MAP_WIDTH] = '\0';

    for (int j = 1; j < MAP_HEIGHT; j++)
        std::strcpy(map[j], map[0]);
}

void draw_map(char map[MAP_HEIGHT][MAP_WIDTH + 1])
{
    for (int j = 0; j < MAP_HEIGHT; j++)
        std::cout << map[j];
}

void set_object_position(struct game_object *obj, float x_pos, float y_pos)
{
    obj->x = x_pos;
    obj->y = y_pos;
}

void init_object(struct game_object *obj, float x_pos, float y_pos, float obj_width, float obj_height, char type)
{
    set_object_position(obj, x_pos, y_pos);
    obj->width = obj_width;
    obj->height = obj_height;
    obj->vertical_speed = 0;
    obj->type = type;
    obj->horizontal_speed = 0.2;
    obj->is_flying = false;
}

void player_died(char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                 game_object **bricks, int *bricks_count,
                 game_object **moving_objects, int *moving_objects_count)
{
    std::system("color 4F");
    Sleep(500);
    create_level(current_level, map, mario, bricks, bricks_count, moving_objects, moving_objects_count);
}

bool is_collision(struct game_object obj_1, struct game_object obj_2)
{
    return ((obj_1.x + obj_1.width) > obj_2.x) && (obj_1.x < (obj_2.x + obj_2.width)) &&
           ((obj_1.y + obj_1.height) > obj_2.y) && (obj_1.y < (obj_2.y + obj_2.height));
}

bool is_pos_inside_map(int x, int y)
{
    return ((x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
}

struct game_object *create_brick(game_object **bricks, int *bricks_count)
{
    (*bricks_count)++;
    *bricks = (game_object*)realloc(*bricks, sizeof(**bricks) * (*bricks_count));
    return *bricks + (*bricks_count) - 1;
}

struct game_object *create_moving_object(game_object **moving_objects, int *moving_objects_count)
{
    (*moving_objects_count)++;
    *moving_objects = (game_object*)realloc(*moving_objects, sizeof(**moving_objects) * (*moving_objects_count));
    return *moving_objects + (*moving_objects_count) - 1;
}

void move_object_vertically(struct game_object *obj, char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                            game_object *bricks, int bricks_count,
                            game_object **moving_objects, int *moving_objects_count)
{
    obj->is_flying = true;
    obj->vertical_speed +=0.05;

    set_object_position(obj, obj->x, obj->y + obj->vertical_speed);

    for (int i = 0; i < bricks_count; i++)
    {
        if (is_collision(*obj, bricks[i]))
        {   
            if (obj->vertical_speed > 0)
                obj->is_flying = false;
            
            if ((bricks[i].type == '?') && (obj->vertical_speed < 0) && (obj == mario))
            {
                bricks[i].type = '-';
                init_object(create_moving_object(moving_objects, moving_objects_count),
                           bricks[i].x, bricks[i].y - 3, 3, 2, '$');
                (*moving_objects)[*moving_objects_count - 1].vertical_speed = -0.7;
            }

            obj->y -= obj->vertical_speed;
            obj->vertical_speed = 0; 

            if (bricks[i].type == '+')
            {
                current_level++;
                if (current_level > max_level)
                    current_level = 1;

                std::system("color 2F");
                Sleep(500);
                create_level(current_level, map, mario, &bricks, &bricks_count, moving_objects, moving_objects_count);
            }

            break;
        }
    }    
}

void move_object_horizontally(struct game_object *obj, char map[MAP_HEIGHT][MAP_WIDTH + 1],
                              game_object *bricks, int bricks_count,
                              game_object **moving_objects, int *moving_objects_count)
{
    obj->x += obj->horizontal_speed;

    for (int i = 0; i < bricks_count; i++)
    {
        if (is_collision(*obj, bricks[i]))
        {
            obj->x -= obj->horizontal_speed;
            obj->horizontal_speed = -obj->horizontal_speed;
            return;
        }
    }
    
    if (obj->type == 'o')
    {
        struct game_object tmp = *obj;
        move_object_vertically(&tmp, map, NULL, bricks, bricks_count, moving_objects, moving_objects_count);

        if (tmp.is_flying == true)
        {
            obj->x -= obj->horizontal_speed;
            obj->horizontal_speed = -obj->horizontal_speed;
        }
    }
}

void move_map_horizontally(float delta_x, char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                           game_object *bricks, int bricks_count,
                           game_object *moving_objects, int moving_objects_count)
{
    mario->x -= delta_x;

    for (int i = 0; i < bricks_count; i++)
    {
        if (is_collision(*mario, bricks[i]))
        {
            mario->x += delta_x;
            return;
        }
    }

    mario->x += delta_x;

    for (int i = 0; i < bricks_count; i++)
        bricks[i].x += delta_x;

    for (int i = 0; i < moving_objects_count; i++)
        moving_objects[i].x += delta_x;
}

void check_mario_collision(char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                           game_object *moving_objects, int *moving_objects_count,
                           game_object **bricks, int *bricks_count)
{
    for (int i = 0; i < *moving_objects_count; i++)
    {
        if (is_collision(*mario, moving_objects[i]))
        {   
            if (moving_objects[i].type == 'o')
            {
                if ((mario->is_flying == true) && (mario->vertical_speed > 0) &&
                    (mario->y + mario->height < moving_objects[i].y + moving_objects[i].height * 0.5f))
                {
                    score += 50;
                    delete_moving_object(i, &moving_objects, moving_objects_count);
                    i--;
                    continue;
                }
                else
                    player_died(map, mario, bricks, bricks_count, &moving_objects, moving_objects_count);
            }

            if (moving_objects[i].type == '$')
            {
                score += 100;
                delete_moving_object(i, &moving_objects, moving_objects_count);
                i--;
                continue;
            }
        }
    }
}

void draw_object_on_map(struct game_object obj, char map[MAP_HEIGHT][MAP_WIDTH + 1])
{
    int ix = (int)round(obj.x);
    int iy = (int)round(obj.y);
    int i_width = (int)round(obj.width);
    int i_height = (int)round(obj.height);

    for (int i = ix; i < (ix + i_width); i++)
        for (int j = iy; j < (iy + i_height); j++)
            if (is_pos_inside_map(i, j))
                map[j][i] = obj.type;
}

void draw_score_on_map(char map[MAP_HEIGHT][MAP_WIDTH + 1], int score)
{
    std::string s = "Score: " + std::to_string(score);
    int len = static_cast<int>(s.size());
    for (int i = 0; i < len && (i + 5) < MAP_WIDTH; ++i)
    {
        map[1][i + 5] = s[i];
    }
}

void set_cursor_position(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void delete_moving_object(int i, game_object **moving_objects, int *moving_objects_count)
{
    if (*moving_objects_count <= 0) return;
    (*moving_objects_count)--;
    (*moving_objects)[i] = (*moving_objects)[*moving_objects_count];
    *moving_objects = (struct game_object*)realloc(*moving_objects, sizeof(**moving_objects) * (*moving_objects_count));
}

void create_level(int level, char map[MAP_HEIGHT][MAP_WIDTH + 1], game_object *mario,
                  game_object **bricks, int *bricks_count,
                  game_object **moving_objects, int *moving_objects_count)
{   
    std::system("color 9F");

    *bricks_count = 0;
    *bricks = (struct game_object*)realloc(*bricks, 0);

    *moving_objects_count = 0;
    *moving_objects = (struct game_object*)realloc(*moving_objects, 0);

    init_object(mario, 39, 10, 3, 3, '@');
    score = 0;

    if (level == 1)
    {
        init_object(create_brick(bricks, bricks_count), 20, 20, 40, 5, '#');
        init_object(create_brick(bricks, bricks_count), 30, 10, 5, 3, '?');
        init_object(create_brick(bricks, bricks_count), 50, 10, 5, 3, '?');
        init_object(create_brick(bricks, bricks_count), 60, 15, 40, 10, '#');
        init_object(create_brick(bricks, bricks_count), 60, 5, 10, 3, '-');
        init_object(create_brick(bricks, bricks_count), 70, 5, 5, 3, '?');
        init_object(create_brick(bricks, bricks_count), 75, 5, 5, 3, '-');
        init_object(create_brick(bricks, bricks_count), 80, 5, 5, 3, '?');
        init_object(create_brick(bricks, bricks_count), 85, 5, 10, 3, '-');
        init_object(create_brick(bricks, bricks_count), 100, 20, 20, 5, '#');
        init_object(create_brick(bricks, bricks_count), 120, 15, 10, 10, '#');
        init_object(create_brick(bricks, bricks_count), 150, 20, 40, 5, '#');
        init_object(create_brick(bricks, bricks_count), 210, 15, 10, 10, '+');

        init_object(create_moving_object(moving_objects, moving_objects_count), 25, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 80, 10, 3, 2, 'o');
    }
    if (level == 2)
    {
        init_object(create_brick(bricks, bricks_count), 20, 20, 40, 5, '#');
        init_object(create_brick(bricks, bricks_count), 60, 15, 10, 10, '#');
        init_object(create_brick(bricks, bricks_count), 80, 20, 20, 5, '#');
        init_object(create_brick(bricks, bricks_count), 120, 15, 10, 10, '#');
        init_object(create_brick(bricks, bricks_count), 150, 20, 40, 5, '#');
        init_object(create_brick(bricks, bricks_count), 210, 15, 10, 10, '+');

        init_object(create_moving_object(moving_objects, moving_objects_count), 25, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 80, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 65, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 120, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 160, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 175, 10, 3, 2, 'o');
    }
    if (level == 3)
    {
        init_object(create_brick(bricks, bricks_count), 20, 20, 40, 5, '#');
        init_object(create_brick(bricks, bricks_count), 80, 20, 15, 5, '#');
        init_object(create_brick(bricks, bricks_count), 120, 15, 15, 10, '#');
        init_object(create_brick(bricks, bricks_count), 160, 10, 15, 15, '+');

        init_object(create_moving_object(moving_objects, moving_objects_count), 25, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 50, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 80, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 90, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 120, 10, 3, 2, 'o');
        init_object(create_moving_object(moving_objects, moving_objects_count), 130, 10, 3, 2, 'o');
    }
    
    max_level = 3;
}

int main() 
{
    char map[MAP_HEIGHT][MAP_WIDTH + 1];
    game_object mario;
    
    game_object *bricks = nullptr;
    int bricks_count = 0;
    
    game_object *moving_objects = nullptr;
    int moving_objects_count = 0;
    
    create_level(current_level, map, &mario, &bricks, &bricks_count, &moving_objects, &moving_objects_count);

    do
    {
        clear_map(map);

        if ((mario.is_flying == false) && (GetKeyState(VK_SPACE)   < 0)) 
            mario.vertical_speed = -1;

        if (GetKeyState('A') < 0)
            move_map_horizontally(1, map, &mario, bricks, bricks_count, moving_objects, moving_objects_count);

        if (GetKeyState('D') < 0)
            move_map_horizontally(-1, map, &mario, bricks, bricks_count, moving_objects, moving_objects_count);

        if (mario.y > MAP_HEIGHT)
            player_died(map, &mario, &bricks, &bricks_count, &moving_objects, &moving_objects_count);
        
        move_object_vertically(&mario, map, &mario, bricks, bricks_count, &moving_objects, &moving_objects_count);
        check_mario_collision(map, &mario, moving_objects, &moving_objects_count, &bricks, &bricks_count);

        for (int i = 0; i < bricks_count; i++)
            draw_object_on_map(bricks[i], map);

        for (int i = 0; i < moving_objects_count; i++)
        {
            move_object_vertically(moving_objects + i, map, &mario, bricks, bricks_count, &moving_objects, &moving_objects_count);
            move_object_horizontally(moving_objects + i, map, bricks, bricks_count, &moving_objects, &moving_objects_count);

            if (moving_objects[i].y > MAP_HEIGHT)
            {
                delete_moving_object(i, &moving_objects, &moving_objects_count);
                i--;
                continue;
            }

            draw_object_on_map(moving_objects[i], map);
        }

        draw_object_on_map(mario, map);
        draw_score_on_map(map, score);

        set_cursor_position(0, 0);
        draw_map(map);

        Sleep(10);
    } 
    while (GetKeyState(VK_ESCAPE) >= 0);
    
    return 0;
}