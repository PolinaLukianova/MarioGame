#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <windows.h>

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 25;
const int MAX_LEVEL = 3;

class GameObject {
public:
    float x, y;
    float width, height;
    char type;

    GameObject() : x(0), y(0), width(0), height(0), type(' ') {}

    virtual ~GameObject() = default;

    void set_position(float x_pos, float y_pos) {
        x = x_pos;
        y = y_pos;
    }

    void init(float x_pos, float y_pos, float obj_width, float obj_height, char obj_type) {
        set_position(x_pos, y_pos);
        width = obj_width;
        height = obj_height;
        type = obj_type;
    }

    virtual void draw(char map[MAP_HEIGHT][MAP_WIDTH + 1]) const {
        int ix = (int)round(x);
        int iy = (int)round(y);
        int i_width = (int)round(width);
        int i_height = (int)round(height);

        for (int i = ix; i < (ix + i_width); i++)
            for (int j = iy; j < (iy + i_height); j++)
                if (is_pos_inside_map(i, j))
                    map[j][i] = type;
    }

    bool is_collision(const GameObject& other) const {
        return ((x + width) > other.x) && (x < (other.x + other.width)) &&
               ((y + height) > other.y) && (y < (other.y + other.height));
    }

    static bool is_pos_inside_map(int x, int y) {
        return ((x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
    }
};

class MovingObject : public GameObject {
public:
    float vertical_speed;
    float horizontal_speed;
    bool is_flying;

    MovingObject() : vertical_speed(0), horizontal_speed(0.2), is_flying(false) {}

    void init(float x_pos, float y_pos, float obj_width, float obj_height, char obj_type) {
        GameObject::init(x_pos, y_pos, obj_width, obj_height, obj_type);
        vertical_speed = 0;
        horizontal_speed = 0.2;
        is_flying = false;
    }
};

class Player : public MovingObject {
public:
    Player() {
        init(39, 10, 3, 3, '@');
    }
};

class Enemy : public MovingObject {
public:
    Enemy() {
        init(0, 0, 3, 2, 'o');
    }
};

class Coin : public MovingObject {
public:
    Coin() {
        init(0, 0, 3, 2, '$');
    }
};

class Brick : public GameObject {
public:
    Brick() {
        horizontal_speed = 0;
        vertical_speed = 0;
        is_flying = false;
    }

    void init(float x_pos, float y_pos, float obj_width, float obj_height, char obj_type) {
        GameObject::init(x_pos, y_pos, obj_width, obj_height, obj_type);
        horizontal_speed = 0;
        vertical_speed = 0;
        is_flying = false;
    }

    float horizontal_speed;
    float vertical_speed;
    bool is_flying;
};

class Map {
private:
    char map[MAP_HEIGHT][MAP_WIDTH + 1];

public:
    Map() {
        clear();
    }

    void clear() {
        for (int i = 0; i < MAP_WIDTH; i++)
            map[0][i] = ' ';

        map[0][MAP_WIDTH] = '\0';

        for (int j = 1; j < MAP_HEIGHT; j++)
            std::strcpy(map[j], map[0]);
    }

    void draw() const {
        for (int j = 0; j < MAP_HEIGHT; j++)
            std::cout << map[j];
    }

    void draw_score(int score) {
        std::string s = "Score: " + std::to_string(score);
        int len = static_cast<int>(s.size());
        for (int i = 0; i < len && (i + 5) < MAP_WIDTH; ++i) {
            map[1][i + 5] = s[i];
        }
    }

    void draw_object(const GameObject& obj) {
        obj.draw(map);
    }

    char* operator[](int index) {
        return map[index];
    }
};

class Game {
private:
    Map game_map;
    Brick* bricks;
    int bricks_count;
    MovingObject* moving_objects;
    int moving_objects_count;
    Player* mario;
    int current_level;
    int score;

public:
    Game() : bricks(nullptr), bricks_count(0), 
             moving_objects(nullptr), moving_objects_count(0),
             mario(nullptr), current_level(1), score(0) {
        create_level(current_level);
    }

    ~Game() {
        delete[] bricks;
        delete[] moving_objects;
        delete mario;
    }

    Brick* create_brick() {
        bricks_count++;
        Brick* new_bricks = new Brick[bricks_count];
        
        for (int i = 0; i < bricks_count - 1; i++) {
            new_bricks[i] = bricks[i];
        }
        
        delete[] bricks;
        bricks = new_bricks;
        return &bricks[bricks_count - 1];
    }

    MovingObject* create_moving_object() {
        moving_objects_count++;
        MovingObject* new_objects = new MovingObject[moving_objects_count];
        
        for (int i = 0; i < moving_objects_count - 1; i++) {
            new_objects[i] = moving_objects[i];
        }
        
        delete[] moving_objects;
        moving_objects = new_objects;
        return &moving_objects[moving_objects_count - 1];
    }

    void delete_moving_object(int index) {
        if (moving_objects_count <= 0) return;
        
        for (int i = index; i < moving_objects_count - 1; i++) {
            moving_objects[i] = moving_objects[i + 1];
        }
        
        moving_objects_count--;
        
        if (moving_objects_count > 0) {
            MovingObject* new_objects = new MovingObject[moving_objects_count];
            for (int i = 0; i < moving_objects_count; i++) {
                new_objects[i] = moving_objects[i];
            }
            delete[] moving_objects;
            moving_objects = new_objects;
        } else {
            delete[] moving_objects;
            moving_objects = nullptr;
        }
    }

    void move_object_vertically(MovingObject* obj) {
        obj->is_flying = true;
        obj->vertical_speed += 0.05;

        obj->set_position(obj->x, obj->y + obj->vertical_speed);

        for (int i = 0; i < bricks_count; i++) {
            if (obj->is_collision(bricks[i])) {
                if (obj->vertical_speed > 0)
                    obj->is_flying = false;

                if ((bricks[i].type == '?') && (obj->vertical_speed < 0) && (obj == mario)) {
                    bricks[i].type = '-';
                    MovingObject* new_coin = create_moving_object();
                    new_coin->init(bricks[i].x, bricks[i].y - 3, 3, 2, '$');
                    new_coin->vertical_speed = -0.7;
                }

                obj->set_position(obj->x, obj->y - obj->vertical_speed);
                obj->vertical_speed = 0;

                if (bricks[i].type == '+') {
                    current_level++;
                    if (current_level > MAX_LEVEL)
                        current_level = 1;

                    std::system("color 2F");
                    Sleep(500);
                    create_level(current_level);
                }

                break;
            }
        }
    }

    void move_object_horizontally(MovingObject* obj) {
        obj->set_position(obj->x + obj->horizontal_speed, obj->y);

        for (int i = 0; i < bricks_count; i++) {
            if (obj->is_collision(bricks[i])) {
                obj->set_position(obj->x - obj->horizontal_speed, obj->y);
                obj->horizontal_speed = -obj->horizontal_speed;
                return;
            }
        }
        
        if (obj->type == 'o') {
            MovingObject tmp = *obj;
            move_object_vertically(&tmp);

            if (tmp.is_flying == true) {
                obj->set_position(obj->x - obj->horizontal_speed, obj->y);
                obj->horizontal_speed = -obj->horizontal_speed;
            }
        }
    }

    void move_map_horizontally(float delta_x) {
        mario->set_position(mario->x - delta_x, mario->y);

        for (int i = 0; i < bricks_count; i++) {
            if (mario->is_collision(bricks[i])) {
                mario->set_position(mario->x + delta_x, mario->y);
                return;
            }
        }

        mario->set_position(mario->x + delta_x, mario->y);

        for (int i = 0; i < bricks_count; i++) {
            bricks[i].set_position(bricks[i].x + delta_x, bricks[i].y);
        }

        for (int i = 0; i < moving_objects_count; i++) {
            moving_objects[i].set_position(moving_objects[i].x + delta_x, moving_objects[i].y);
        }
    }

    void check_mario_collision() {
        for (int i = 0; i < moving_objects_count; i++) {
            if (mario->is_collision(moving_objects[i])) {
                if (moving_objects[i].type == 'o') {
                    if ((mario->is_flying == true) && (mario->vertical_speed > 0) &&
                        (mario->y + mario->height < moving_objects[i].y + moving_objects[i].height * 0.5f)) {
                        score += 50;
                        delete_moving_object(i);
                        i--;
                        continue;
                    }
                    else {
                        player_died();
                    }
                }

                if (moving_objects[i].type == '$') {
                    score += 100;
                    delete_moving_object(i);
                    i--;
                    continue;
                }
            }
        }
    }

    void player_died() {
        std::system("color 4F");
        Sleep(500);
        create_level(current_level);
    }

    void create_level(int level) {
        std::system("color 9F");

        delete[] bricks;
        bricks = nullptr;
        bricks_count = 0;

        delete[] moving_objects;
        moving_objects = nullptr;
        moving_objects_count = 0;

        delete mario;
        mario = new Player();
        score = 0;

        if (level == 1) {
            create_brick()->init(20, 20, 40, 5, '#');
            create_brick()->init(30, 10, 5, 3, '?');
            create_brick()->init(50, 10, 5, 3, '?');
            create_brick()->init(60, 15, 40, 10, '#');
            create_brick()->init(60, 5, 10, 3, '-');
            create_brick()->init(70, 5, 5, 3, '?');
            create_brick()->init(75, 5, 5, 3, '-');
            create_brick()->init(80, 5, 5, 3, '?');
            create_brick()->init(85, 5, 10, 3, '-');
            create_brick()->init(100, 20, 20, 5, '#');
            create_brick()->init(120, 15, 10, 10, '#');
            create_brick()->init(150, 20, 40, 5, '#');
            create_brick()->init(210, 15, 10, 10, '+');

            create_moving_object()->init(25, 10, 3, 2, 'o');
            create_moving_object()->init(80, 10, 3, 2, 'o');
        }
        if (level == 2) {
            create_brick()->init(20, 20, 40, 5, '#');
            create_brick()->init(60, 15, 10, 10, '#');
            create_brick()->init(80, 20, 20, 5, '#');
            create_brick()->init(120, 15, 10, 10, '#');
            create_brick()->init(150, 20, 40, 5, '#');
            create_brick()->init(210, 15, 10, 10, '+');

            create_moving_object()->init(25, 10, 3, 2, 'o');
            create_moving_object()->init(80, 10, 3, 2, 'o');
            create_moving_object()->init(65, 10, 3, 2, 'o');
            create_moving_object()->init(120, 10, 3, 2, 'o');
            create_moving_object()->init(160, 10, 3, 2, 'o');
            create_moving_object()->init(175, 10, 3, 2, 'o');
        }
        if (level == 3) {
            create_brick()->init(20, 20, 40, 5, '#');
            create_brick()->init(80, 20, 15, 5, '#');
            create_brick()->init(120, 15, 15, 10, '#');
            create_brick()->init(160, 10, 15, 15, '+');

            create_moving_object()->init(25, 10, 3, 2, 'o');
            create_moving_object()->init(50, 10, 3, 2, 'o');
            create_moving_object()->init(80, 10, 3, 2, 'o');
            create_moving_object()->init(90, 10, 3, 2, 'o');
            create_moving_object()->init(120, 10, 3, 2, 'o');
            create_moving_object()->init(130, 10, 3, 2, 'o');
        }
    }

    void update() {
        game_map.clear();

        if ((mario->is_flying == false) && (GetKeyState(VK_SPACE) < 0))
            mario->vertical_speed = -1;

        if (GetKeyState('A') < 0)
            move_map_horizontally(1);

        if (GetKeyState('D') < 0)
            move_map_horizontally(-1);

        if (mario->y > MAP_HEIGHT)
            player_died();

        move_object_vertically(mario);
        check_mario_collision();

        for (int i = 0; i < bricks_count; i++)
            game_map.draw_object(bricks[i]);

        for (int i = 0; i < moving_objects_count; i++) {
            move_object_vertically(&moving_objects[i]);
            move_object_horizontally(&moving_objects[i]);

            if (moving_objects[i].y > MAP_HEIGHT) {
                delete_moving_object(i);
                i--;
                continue;
            }

            game_map.draw_object(moving_objects[i]);
        }

        game_map.draw_object(*mario);
        game_map.draw_score(score);
    }

    void render() {
        set_cursor_position(0, 0);
        game_map.draw();
    }

    void set_cursor_position(int x, int y) {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    void run() {
        do {
            update();
            render();
            Sleep(10);
        } while (GetKeyState(VK_ESCAPE) >= 0);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}