#include <raylib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

const int unit = 10;
const int screenWidth = 640;
const int screenHeight = 360;
const int fps = 60;

struct vectorxy {
    float x;
    float y;

    friend std::ostream& operator<<(std::ostream& os, const vectorxy& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }
};

///class Background {};

///class Foreground {};

class Item {
private:
    vectorxy pos;
    Color col;
    float hitboxCir;
    bool collected = false;
public:
    Item() {
        this->hitboxCir = unit;
        this->pos.x = screenWidth / 2.0f - this->hitboxCir / 2.0f;
        this->pos.y = screenHeight / 2.0f - this->hitboxCir / 2.0f;
        this->col = YELLOW;
    }
    Item(vectorxy pos, Color col, const float hitbox) {
        this->pos = pos;
        this->col = col;
        this->hitboxCir = hitbox;
    }

    void draw() {
        DrawCircle(pos.x, pos.y, hitboxCir, this->col);
    }

    friend std::ostream& operator<<(std::ostream& os, const Item& item) {
        os << "Item { pos: " << item.pos
           << ", col: (" << (int)item.col.r << ", " << (int)item.col.g << ", " << (int)item.col.b << ", " << (int)item.col.a << ")"
           << ", hitboxCir: " << item.hitboxCir
           << ", collected: " << (item.collected ? "true" : "false") << " }";
        return os;
    }
};

class Entity {
private:
    vectorxy pos;
    Color col;
    float weight;
    vectorxy hitboxRect;
    vectorxy vel;
    float hp;
    Item* inventory;
    int inventoryCount;
    int inventoryCapacity;

public:
    Entity() {
        this->hitboxRect = {unit, unit};
        this->pos.x = screenWidth / 2.0f - hitboxRect.x / 2;
        this->pos.y = screenHeight / 2.0f - hitboxRect.y / 2;
        this->col = BLUE;
        this->weight = 1;
        this->vel.x = 0;
        this->vel.y = 0;
        this->hp = 1;
        this->inventoryCount = 0;
        this->inventoryCapacity = 5;
        this->inventory = new Item[this->inventoryCapacity];
    }

    Entity(vectorxy pos, Color color, float weight, vectorxy hitbox, vectorxy vel, float hp, int invCapacity) {
        this->pos = pos;
        this->col = color;
        this->vel = vel;
        this->weight = weight;
        this->hitboxRect = hitbox;
        this->hp = hp;
        this->inventoryCount = 0;
        if (invCapacity > 0) {
            this->inventoryCapacity = invCapacity;
            this->inventory = new Item[this->inventoryCapacity];
        } else {
            this->inventoryCapacity = 0;
            this->inventory = nullptr;
        }
    }

    Entity(const Entity& other) {
        this->pos = other.pos;
        this->col = other.col;
        this->vel = other.vel;
        this->weight = other.weight;
        this->hitboxRect = other.hitboxRect;
        this->hp = other.hp;
        this->inventoryCount = other.inventoryCount;
        this->inventoryCapacity = other.inventoryCapacity;

        if (this->inventoryCapacity > 0 && other.inventory != nullptr) {
            this->inventory = new Item[this->inventoryCapacity];
            for (int i = 0; i < this->inventoryCount; i++) {
                this->inventory[i] = other.inventory[i];
            }
        } else {
            this->inventory = nullptr;
            this->inventoryCount = 0;
            this->inventoryCapacity = 0;
        }
    }

    ~Entity() {
        delete[] this->inventory;
        this->inventory = nullptr;
        this->inventoryCount = 0;
        this->inventoryCapacity = 0;
    }

    Entity& operator=(const Entity& other) {
        if (this == &other) {
            return *this;
        }

        delete[] this->inventory;
        this->inventory = nullptr;

        this->pos = other.pos;
        this->col = other.col;
        this->vel = other.vel;
        this->weight = other.weight;
        this->hitboxRect = other.hitboxRect;
        this->hp = other.hp;
        this->inventoryCount = other.inventoryCount;
        this->inventoryCapacity = other.inventoryCapacity;

        if (this->inventoryCapacity > 0 && other.inventory != nullptr) {
            this->inventory = new Item[this->inventoryCapacity];
            for (int i = 0; i < this->inventoryCount; i++) {
                this->inventory[i] = other.inventory[i];
            }
        } else {
            this->inventory = nullptr;
            this->inventoryCount = 0;
            this->inventoryCapacity = 0;
        }

        return *this;
    }

    // void insertInventory(Item item) {
    //     if (this->inventoryCount >= this->inventoryCapacity) {
    //         int newCapacity;
    //         if (this->inventoryCapacity > 0) {
    //             newCapacity = this->inventoryCapacity * 2;
    //         } else {
    //             newCapacity = 1;
    //         }
    //
    //         Item* newBuffer = new Item[newCapacity];
    //         for (int i = 0; i < this->inventoryCount; i++) {
    //             newBuffer[i] = this->inventory[i];
    //         }
    //
    //         delete[] this->inventory;
    //         this->inventory = newBuffer;
    //         this->inventoryCapacity = newCapacity;
    //     }
    //
    //     this->inventory[this->inventoryCount] = item;
    //     this->inventoryCount++;
    // }

    void draw() {
        DrawRectangle(pos.x, pos.y, hitboxRect.x, hitboxRect.y, this->col);
    }

    friend std::ostream& operator<<(std::ostream& os, const Entity& entity) {
        os << "Entity { pos: " << entity.pos
           << ", col: (" << (int)entity.col.r << ", " << (int)entity.col.g << ", " << (int)entity.col.b << ", " << (int)entity.col.a << ")"
           << ", weight: " << entity.weight
           << ", hitboxRect: " << entity.hitboxRect
           << ", vel: " << entity.vel
           << ", hp: " << entity.hp
           << ", inventory (" << entity.inventoryCount << "/" << entity.inventoryCapacity << "): [";
        for (int i = 0; i < entity.inventoryCount; i++) {
            os << entity.inventory[i];
            if (i < entity.inventoryCount - 1) {
                os << ", ";
            }
        }
        os << "] }";
        return os;
    }
};

class Block {
private:
    vectorxy pos;
    Color col;
    vectorxy hitboxRect;
public:
    Block() {
        this->hitboxRect = {unit, unit};
        this->pos.x = screenWidth / 2.0f - this->hitboxRect.x / 2.0f;
        this->pos.y = screenHeight / 2.0f - this->hitboxRect.y / 2.0f;
        this->col = BLUE;
    }
    Block(vectorxy pos, Color color, vectorxy hitbox) {
        this->pos = pos;
        this->col = color;
        this->hitboxRect = hitbox;
    }
    void draw() {
        DrawRectangle(pos.x, pos.y, hitboxRect.x, hitboxRect.y, this->col);
    }

    friend std::ostream& operator<<(std::ostream& os, const Block& block) {
        os << "Block { pos: " << block.pos
           << ", col: (" << (int)block.col.r << ", " << (int)block.col.g << ", " << (int)block.col.b << ", " << (int)block.col.a << ")"
           << ", hitboxRect: " << block.hitboxRect << " }";
        return os;
    }
};

int main() {
    int game = 1;
    std::ifstream gin("tastatura.txt");
    if (gin.is_open())
        gin >> game;
    else if (!(std::cin >> game)) {
        game = 1;
    }
    if (game == 0) return 0;

    InitWindow(screenWidth, screenHeight, "Escape from Aether - raylib test");
    SetTargetFPS(fps);

    Entity player;
    Block thing({0, screenHeight / 2.0f + 60}, BROWN, {screenWidth, 120});
    Item coin({screenWidth / 2.0f - 3 * unit, screenHeight / 2.0f}, YELLOW, unit);

    while (!WindowShouldClose()) {
        BeginDrawing();
        player.draw();
        thing.draw();
        coin.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}