// main.cpp
#include <GL/glut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <cmath>
#include <iostream>
#include <deque>

// ─────────────────────── Window ───────────────────────
const int windowWidth = 800;
const int windowHeight = 600;

// ───────────────────── Game Constants ─────────────────────
const int POWERUP_CHANCE = 15;  // 1 in 15 chance for enemy to drop powerup
const int MAX_LEVEL = 10;
const float BULLET_SPEED = 12.0f;
const float ROCKET_SPEED = 7.0f;
const float ENEMY_BASE_SPEED = 2.0f;

// ───────────────── GameObject Types ─────────────────────
enum PowerUpType { MULTI_SHOT, SHIELD, SPEED_BOOST, NONE };

struct GameObject {
    float x, y, width, height;
    GameObject(float _x, float _y, float _w, float _h)
        : x(_x), y(_y), width(_w), height(_h) {
    }
    virtual ~GameObject() {}
};

struct Bullet : GameObject {
    float angle;
    Bullet(float x, float y, float angle = 0.0f)
        : GameObject(x, y, 5, 15), angle(angle) {
    }
};

struct Enemy : GameObject {
    int health;
    int type;
    float speedMultiplier;
    Enemy(float x, float y, int _type = 0)
        : GameObject(x, y, 40, 20), health(1 + _type), type(_type),
        speedMultiplier(1.0f + _type * 0.2f) {
    }
};

struct Explosion {
    float x, y, size, alpha;
    float r, g, b;
    Explosion(float _x, float _y, float _size = 30.0f,
        float _r = 1.0f, float _g = 0.5f, float _b = 0.0f)
        : x(_x), y(_y), size(_size), alpha(1.0f), r(_r), g(_g), b(_b) {
    }
};

struct PowerUp : GameObject {
    PowerUpType type;
    float spawnTime;
    PowerUp(float x, float y, PowerUpType _type)
        : GameObject(x, y, 20, 20), type(_type),
        spawnTime(glutGet(GLUT_ELAPSED_TIME) * 0.001f) {
    }
};

struct Rocket : GameObject {
    float spawnTime;
    Rocket(float x, float y, float t)
        : GameObject(x, y, 12, 30), spawnTime(t) {
    }
};

struct Star {
    float x, y, baseBright, size;
    Star(float _x, float _y, float b, float s = 2.0f)
        : x(_x), y(_y), baseBright(b), size(s) {
    }
};

struct Particle {
    float x, y, vx, vy, lifetime, maxLife, size;
    float r, g, b, alpha;
    Particle(float _x, float _y, float _vx, float _vy,
        float _life, float _size, float _r, float _g, float _b)
        : x(_x), y(_y), vx(_vx), vy(_vy), lifetime(_life), maxLife(_life),
        size(_size), r(_r), g(_g), b(_b), alpha(1.0f) {
    }
};

// ──────────────────── Game State Globals ────────────────────
GameObject player(windowWidth / 2 - 25, 50, 50, 20);
std::vector<Bullet> bullets;
std::vector<Enemy> enemies;
std::vector<Rocket> rockets;
std::vector<Star> stars;
std::vector<Explosion> explosions;
std::vector<PowerUp> powerUps;
std::vector<Particle> particles;
std::deque<std::string> messageLog;
// Add with other game state globals

bool specialKeys[256] = { false }; // For arrow keys
int score = 0;
int level = 1;
int lives = 3;
int enemiesDefeated = 0;
int enemiesForNextLevel = 10;
bool gameOver = false;
bool keys[256] = { false };
bool playerShield = false;
bool multiShot = false;
float playerSpeedBoost = 1.0f;
float shieldTime = 0.0f;
float multiShotTime = 0.0f;
float speedBoostTime = 0.0f;
float playerInvulnerableTime = 0.0f;
bool moveRight = false;
bool moveLeft = false;

// ──────────────── Function Prototypes ────────────────
void display();
void update(int value);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void createEnemy();
void enemySpawner(int value);
void drawText(float x, float y, const std::string& txt);
void levelUp();
void addMessage(const std::string& msg);
void spawnPowerUp(float x, float y);
void createParticles(float x, float y, int count, float r, float g, float b);

void specialKey(int key, int x, int y) {
    specialKeys[key] = true;
}

void specialKeyUp(int key, int x, int y) {
    specialKeys[key] = false;
}
// ─────────────── Rendering Helper Functions ───────────────
void drawRect(float x, float y, float w, float h,
    float r, float g, float b, float a = 1.0f) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float x, float y, float radius,
    float r, float g, float b, float a = 1.0f, int segments = 20) {
    glColor4f(r, g, b, a);
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * 3.1415926f * i / segments;
        float cx = radius * cosf(theta);
        float cy = radius * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawText(float x, float y, const std::string& txt) {
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (const char c : txt) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glPopMatrix();
}

void drawSmallText(float x, float y, const std::string& txt) {
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (const char c : txt) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
    glPopMatrix();
}

bool isColliding(const GameObject& a, const GameObject& b) {
    return !(a.x + a.width < b.x ||
        a.x > b.x + b.width ||
        a.y + a.height < b.y ||
        a.y > b.y + b.height);
}

void drawRocket(const Rocket& r) {
    glPushMatrix();
    glTranslatef(r.x, r.y, 0);
    // Body triangle
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_TRIANGLES);
    glVertex2f(r.width / 2, r.height);
    glVertex2f(0, 0);
    glVertex2f(r.width, 0);
    glEnd();
    // Fins
    drawRect(-4, 4, 4, 12, 0.7f, 0.1f, 0.1f);
    drawRect(r.width, 4, 4, 12, 0.7f, 0.1f, 0.1f);
    // Window
    drawRect(r.width / 2 - 5, r.height * 0.6f, 10, 8, 0.1f, 0.1f, 0.7f);
    // Animated flame
    float t = (glutGet(GLUT_ELAPSED_TIME) * 0.005f) + r.spawnTime;
    float flameLen = 8 + 4 * std::sin(t * 10);
    glColor3f(1, 0.5f, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(r.width / 2, 0);
    glVertex2f(r.width / 2 - 6, -flameLen);
    glVertex2f(r.width / 2 + 6, -flameLen);
    glEnd();
    glPopMatrix();
}

void drawEnemy(const Enemy& e) {
    float r, g, b;
    switch (e.type) {
    case 0: // Basic enemy
        r = 1.0f; g = 0.2f; b = 0.2f;
        break;
    case 1: // Advanced enemy
        r = 0.2f; g = 0.7f; b = 0.2f;
        break;
    case 2: // Elite enemy
        r = 0.2f; g = 0.2f; b = 1.0f;
        break;
    default:
        r = 0.8f; g = 0.8f; b = 0.0f;
    }

    drawRect(e.x, e.y, e.width, e.height, r, g, b);

    // Draw health bar
    if (e.health > 1) {
        float healthPercentage = e.health / float(e.type + 1);
        drawRect(e.x, e.y + e.height + 5, e.width * healthPercentage, 3,
            0.0f, 1.0f, 0.0f);
    }

    // Draw enemy design details
    if (e.type == 1) {
        // Advanced enemy has a center orb
        drawCircle(e.x + e.width / 2, e.y + e.height / 2, 5, 1.0f, 1.0f, 0.0f);
    }
    else if (e.type == 2) {
        // Elite enemy has two side orbs
        drawCircle(e.x + 10, e.y + e.height / 2, 4, 0.7f, 0.7f, 1.0f);
        drawCircle(e.x + e.width - 10, e.y + e.height / 2, 4, 0.7f, 0.7f, 1.0f);
    }
}

void drawPlayer() {
    // Base ship
    drawRect(player.x, player.y, player.width, player.height, 0.2f, 0.7f, 1.0f);

    // Ship details
    drawRect(player.x + player.width / 2 - 5, player.y + player.height - 5, 10, 8, 0.1f, 0.3f, 0.8f);
    drawRect(player.x + 5, player.y, 10, player.height / 2, 0.3f, 0.5f, 0.9f);
    drawRect(player.x + player.width - 15, player.y, 10, player.height / 2, 0.3f, 0.5f, 0.9f);

    // Thruster flames
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.005f;
    float flameLen = 5 + 3 * std::sin(t * 8);

    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(player.x + 10, player.y);
    glVertex2f(player.x + 5, player.y - flameLen);
    glVertex2f(player.x + 15, player.y - flameLen);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(player.x + player.width - 10, player.y);
    glVertex2f(player.x + player.width - 15, player.y - flameLen);
    glVertex2f(player.x + player.width - 5, player.y - flameLen);
    glEnd();

    // Draw shield if active
    if (playerShield) {
        float pulseScale = 0.8f + 0.2f * std::sin(t * 5);
        drawCircle(player.x + player.width / 2, player.y + player.height / 2,
            player.width / 1.5f * pulseScale, 0.4f, 0.8f, 1.0f, 0.5f);
    }

    // Draw speed boost effect if active
    if (playerSpeedBoost > 1.0f) {
        glColor4f(0.0f, 1.0f, 0.5f, 0.7f);
        glBegin(GL_TRIANGLES);
        glVertex2f(player.x, player.y + player.height / 2);
        glVertex2f(player.x - 15, player.y + player.height);
        glVertex2f(player.x - 15, player.y);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex2f(player.x + player.width, player.y + player.height / 2);
        glVertex2f(player.x + player.width + 15, player.y + player.height);
        glVertex2f(player.x + player.width + 15, player.y);
        glEnd();
    }

    // Invulnerability blinking
    if (playerInvulnerableTime > 0 && int(playerInvulnerableTime * 10) % 2 == 0) {
        drawRect(player.x, player.y, player.width, player.height, 1.0f, 1.0f, 1.0f, 0.7f);
    }
}

void drawPowerUp(const PowerUp& p) {
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f - p.spawnTime;
    float floatOffset = 5 * sin(t * 3);
    float rotation = t * 90;

    glPushMatrix();
    glTranslatef(p.x + p.width / 2, p.y + p.height / 2 + floatOffset, 0);
    glRotatef(rotation, 0, 0, 1);

    switch (p.type) {
    case MULTI_SHOT:
        drawRect(-p.width / 2, -p.height / 2, p.width, p.height, 1.0f, 0.8f, 0.0f);
        // Draw a triple bullet symbol
        drawRect(-6, -5, 4, 10, 1.0f, 1.0f, 1.0f);
        drawRect(-1, -5, 4, 10, 1.0f, 1.0f, 1.0f);
        drawRect(4, -5, 4, 10, 1.0f, 1.0f, 1.0f);
        break;

    case SHIELD:
        drawRect(-p.width / 2, -p.height / 2, p.width, p.height, 0.0f, 0.8f, 1.0f);
        // Draw shield symbol
        drawCircle(0, 0, 8, 1.0f, 1.0f, 1.0f, 0.6f);
        drawCircle(0, 0, 5, 0.0f, 0.8f, 1.0f, 1.0f);
        break;

    case SPEED_BOOST:
        drawRect(-p.width / 2, -p.height / 2, p.width, p.height, 0.0f, 1.0f, 0.4f);
        // Draw speed symbol
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-6, -6);
        glVertex2f(8, 0);
        glVertex2f(-6, 6);
        glEnd();
        break;

    default:
        drawRect(-p.width / 2, -p.height / 2, p.width, p.height, 0.8f, 0.8f, 0.8f);
    }

    glPopMatrix();
}

// ───────────────────── Game Logic ─────────────────────
void fireBullet() {
    if (multiShot) {
        // Triple shot pattern
        bullets.emplace_back(player.x + player.width / 2 - 2.5f,
            player.y + player.height);
        bullets.emplace_back(player.x + player.width / 2 - 2.5f,
            player.y + player.height, -0.2f);
        bullets.emplace_back(player.x + player.width / 2 - 2.5f,
            player.y + player.height, 0.2f);
    }
    else {
        // Regular shot
        bullets.emplace_back(player.x + player.width / 2 - 2.5f,
            player.y + player.height);
    }

    // Sound effects and visual flair would go here in a full game
}

void fireRocket() {
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    rockets.emplace_back(player.x + player.width / 2 - 6,
        player.y + player.height,
        t);

    // Create exhaust particles
    createParticles(
        player.x + player.width / 2,
        player.y + player.height,
        10,
        1.0f, 0.5f, 0.0f
    );
}

void createEnemy() {
    float ex = std::rand() % (windowWidth - 40);

    // Enemy type determination based on level
    int type = 0;
    int roll = rand() % 100;

    if (level >= 3 && roll < 20 + level * 5) {
        type = 1; // Advanced enemy appears more as level increases
    }

    if (level >= 5 && roll < 10 + level * 2) {
        type = 2; // Elite enemy has small chance in higher levels
    }

    enemies.emplace_back(ex, windowHeight, type);
}

void enemySpawner(int value) {
    if (!gameOver) {
        createEnemy();
        // Spawn rate increases with level
        int delay = std::max(300, 1500 - level * 100);
        glutTimerFunc(delay, enemySpawner, 0);
    }
}

void levelUp() {
    level++;

    // Display level up message
    std::stringstream ss;
    ss << "LEVEL " << level << "!";
    addMessage(ss.str());

    if (level <= MAX_LEVEL) {
        // Increase enemies needed for next level
        enemiesForNextLevel = 10 + 5 * level;
        enemiesDefeated = 0;

        // Bonus for leveling up
        if (level % 2 == 0) {
            lives++;
            addMessage("Extra life awarded!");
        }
    }
    else if (level == MAX_LEVEL + 1) {
        addMessage("MAXIMUM LEVEL REACHED!");
    }
}

void addMessage(const std::string& msg) {
    messageLog.push_front(msg);
    if (messageLog.size() > 4) {
        messageLog.pop_back();
    }
}

void spawnPowerUp(float x, float y) {
    if (rand() % POWERUP_CHANCE != 0) return;

    PowerUpType type = static_cast<PowerUpType>(rand() % 3);
    powerUps.emplace_back(x, y, type);
}

void createParticles(float x, float y, int count, float r, float g, float b) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 628) / 100.0f;
        float speed = 1.0f + (rand() % 200) / 100.0f;
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        float lifetime = 0.5f + (rand() % 100) / 100.0f;
        float size = 1.0f + (rand() % 30) / 10.0f;

        particles.emplace_back(x, y, vx, vy, lifetime, size, r, g, b);
    }
}

void update(int) {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    // Always keep updating the display
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);

    if (gameOver) {
        return;
    }

    // Update timer-based power-ups
    if (shieldTime > 0) {
        shieldTime -= 0.016f;
        if (shieldTime <= 0) {
            playerShield = false;
            addMessage("Shield deactivated");
        }
    }

    if (multiShotTime > 0) {
        multiShotTime -= 0.016f;
        if (multiShotTime <= 0) {
            multiShot = false;
            addMessage("Multi-shot deactivated");
        }
    }

    if (speedBoostTime > 0) {
        speedBoostTime -= 0.016f;
        if (speedBoostTime <= 0) {
            playerSpeedBoost = 1.0f;
            addMessage("Speed boost deactivated");
        }
    }

    if (playerInvulnerableTime > 0) {
        playerInvulnerableTime -= 0.016f;
    }

    // — Move bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        float vx = sin(it->angle) * BULLET_SPEED;
        float vy = cos(it->angle) * BULLET_SPEED;
        it->x += vx;
        it->y += vy;

        if (it->y > windowHeight || it->x < 0 || it->x > windowWidth) {
            it = bullets.erase(it);
        }
        else {
            ++it;
        }
    }

    // — Move rockets
    for (auto it = rockets.begin(); it != rockets.end();) {
        it->y += ROCKET_SPEED;
        if (it->y > windowHeight) {
            it = rockets.erase(it);
        }
        else {
            ++it;
        }
    }

    // — Move enemies
    for (auto it = enemies.begin(); it != enemies.end();) {
        float speed = ENEMY_BASE_SPEED * it->speedMultiplier * (1.0f + level * 0.1f);
        it->y -= speed;

        // Advanced enemies move in patterns
        if (it->type == 1) {
            it->x += sin(currentTime * 2 + it->y * 0.01f) * 2;
        }
        else if (it->type == 2) {
            it->x += sin(currentTime * 3 + it->y * 0.02f) * 3;
        }

        // Keep enemies within screen bounds
        it->x = std::max(0.0f, std::min(it->x, float(windowWidth - it->width)));

        if (it->y < 0) {
            it = enemies.erase(it);
            if (--lives <= 0) {
                gameOver = true;
                return;
            }
            addMessage("Enemy reached the base! Life lost.");
        }
        else {
            ++it;
        }
    }

    // — Move power-ups
    for (auto it = powerUps.begin(); it != powerUps.end();) {
        it->y -= 1.0f;
        if (it->y < 0) {
            it = powerUps.erase(it);
        }
        else {
            ++it;
        }
    }

    // — Update explosions
    for (auto it = explosions.begin(); it != explosions.end();) {
        it->alpha -= 0.04f;
        it->size += 2.0f;
        if (it->alpha <= 0) {
            it = explosions.erase(it);
        }
        else {
            ++it;
        }
    }

    // — Update particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->x += it->vx;
        it->y += it->vy;
        it->lifetime -= 0.016f;
        it->alpha = it->lifetime / it->maxLife;

        if (it->lifetime <= 0) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }

    // — Collisions: bullets vs enemies
    for (auto b = bullets.begin(); b != bullets.end();) {
        bool hit = false;
        for (auto e = enemies.begin(); e != enemies.end();) {
            if (isColliding(*b, *e)) {
                b = bullets.erase(b);

                e->health--;
                if (e->health <= 0) {
                    // Create explosion
                    explosions.emplace_back(
                        e->x + e->width / 2,
                        e->y + e->height / 2,
                        30.0f + e->type * 10.0f
                    );

                    // Create particles
                    createParticles(
                        e->x + e->width / 2,
                        e->y + e->height / 2,
                        10 + e->type * 5,
                        1.0f, 0.5f, 0.0f
                    );

                    // Check for powerup drop
                    spawnPowerUp(e->x, e->y);

                    // Increase score based on enemy type
                    score += 10 * (e->type + 1);
                    enemiesDefeated++;

                    // Level up check
                    if (enemiesDefeated >= enemiesForNextLevel && level < MAX_LEVEL) {
                        levelUp();
                    }

                    e = enemies.erase(e);
                }
                else {
                    ++e;
                }

                hit = true;
                break;
            }
            else {
                ++e;
            }
        }
        if (!hit) ++b;
    }

    // — Collisions: rockets vs enemies
    for (auto r = rockets.begin(); r != rockets.end();) {
        bool hit = false;
        for (auto e = enemies.begin(); e != enemies.end();) {
            GameObject blast(r->x - 10, r->y - 10,
                r->width + 20, r->height + 20);
            if (isColliding(blast, *e)) {
                r = rockets.erase(r);

                // Create explosion
                explosions.emplace_back(
                    e->x + e->width / 2,
                    e->y + e->height / 2,
                    50.0f
                );

                // Create particles
                createParticles(
                    e->x + e->width / 2,
                    e->y + e->height / 2,
                    20,
                    1.0f, 0.3f, 0.0f
                );

                // Check for powerup drop (higher chance from rockets)
                if (rand() % (POWERUP_CHANCE / 2) == 0) {
                    spawnPowerUp(e->x, e->y);
                }

                // Rockets always destroy enemies regardless of health
                score += 30 * (e->type + 1);
                enemiesDefeated++;

                // Level up check
                if (enemiesDefeated >= enemiesForNextLevel && level < MAX_LEVEL) {
                    levelUp();
                }

                e = enemies.erase(e);
                hit = true;
                break;
            }
            else {
                ++e;
            }
        }
        if (!hit) ++r;
    }

    // — Collisions: player vs enemies
    if (playerInvulnerableTime <= 0) {
        for (auto e = enemies.begin(); e != enemies.end();) {
            if (isColliding(player, *e)) {
                explosions.emplace_back(
                    e->x + e->width / 2,
                    e->y + e->height / 2,
                    40.0f,
                    1.0f, 0.0f, 0.0f
                );

                createParticles(
                    e->x + e->width / 2,
                    e->y + e->height / 2,
                    15,
                    1.0f, 0.2f, 0.2f
                );

                e = enemies.erase(e);

                if (playerShield) {
                    // Shield absorbs the hit
                    playerShield = false;
                    shieldTime = 0;
                    addMessage("Shield absorbed a collision!");
                    playerInvulnerableTime = 1.0f;
                }
                else {
                    // Player loses a life
                    lives--;
                    if (lives <= 0) {
                        gameOver = true;

                        // Big explosion for player death
                        explosions.emplace_back(
                            player.x + player.width / 2,
                            player.y + player.height / 2,
                            80.0f,
                            1.0f, 0.0f, 0.0f
                        );

                        createParticles(
                            player.x + player.width / 2,
                            player.y + player.height / 2,
                            40,
                            1.0f, 0.5f, 0.2f
                        );

                        return;
                    }

                    addMessage("Ship damaged! Life lost.");
                    playerInvulnerableTime = 3.0f;
                }
            }
            else {
                ++e;
            }
        }
    }

    // — Collisions: player vs powerups
    // — Collisions: player vs powerups
    for (auto p = powerUps.begin(); p != powerUps.end();) {
        if (isColliding(player, *p)) {
            switch (p->type) {
            case MULTI_SHOT:
                multiShot = true;
                multiShotTime = 10.0f;
                addMessage("Multi-shot activated!");
                break;

            case SHIELD:
                playerShield = true;
                shieldTime = 15.0f;
                addMessage("Shield activated!");
                break;

            case SPEED_BOOST:
                playerSpeedBoost = 2.0f;
                speedBoostTime = 8.0f;
                addMessage("Speed boost activated!");
                break;
            }

            // Create powerup pickup effect
            createParticles(
                p->x + p->width / 2,
                p->y + p->height / 2,
                15,
                0.5f, 1.0f, 1.0f
            );

            p = powerUps.erase(p);
        }
        else {
            ++p;
        }
    }

    // — Player movement
    // — Player movement
    float playerSpeed = 5.0f * playerSpeedBoost;
    if (moveLeft || specialKeys[GLUT_KEY_LEFT]) {
        player.x -= playerSpeed;
    }
    if (moveRight || specialKeys[GLUT_KEY_RIGHT]) {
        player.x += playerSpeed;
    }
    if (keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP]) {
        player.y += playerSpeed;
    }
    if (keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN]) {
        player.y -= playerSpeed;
    }

    // Keep player in bounds
    player.x = std::max(0.0f, std::min(player.x, float(windowWidth - player.width)));
    player.y = std::max(0.0f, std::min(player.y, float(windowHeight - player.height)));
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;
    if (key == 'a' || key == 'A') moveLeft = true;
    if (key == 'd' || key == 'D') moveRight = true;
    // Immediate actions
    switch (key) {
    case ' ': // Fire bullet
        if (!gameOver) fireBullet();
        break;

    case 'r':
    case 'R': // Fire rocket
        if (!gameOver) fireRocket();
        break;

    case 27: // ESC - quit
        exit(0);
        break;

    case 'p':
    case 'P': // Start a new game if game over
        if (gameOver) {
            // Reset game state
            player.x = windowWidth / 2 - 25;
            player.y = 50;
            bullets.clear();
            enemies.clear();
            rockets.clear();
            explosions.clear();
            powerUps.clear();
            particles.clear();
            messageLog.clear();

            score = 0;
            level = 1;
            lives = 3;
            enemiesDefeated = 0;
            enemiesForNextLevel = 10;
            gameOver = false;
            playerShield = false;
            multiShot = false;
            playerSpeedBoost = 1.0f;
            shieldTime = 0.0f;
            multiShotTime = 0.0f;
            speedBoostTime = 0.0f;
            playerInvulnerableTime = 3.0f;

            // Start enemy spawning
            glutTimerFunc(1000, enemySpawner, 0);

            addMessage("Game started! Good luck!");
        }
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
    if (key == 'a' || key == 'A') moveLeft = false;
    if (key == 'd' || key == 'D') moveRight = false;
}

void initStars() {
    // Create a starfield background
    for (int i = 0; i < 100; i++) {
        float x = rand() % windowWidth;
        float y = rand() % windowHeight;
        float brightness = 0.3f + (rand() % 70) / 100.0f;
        float size = 1.0f + (rand() % 30) / 10.0f;
        stars.emplace_back(x, y, brightness, size);
    }
}

void drawStars() {
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    for (const auto& star : stars) {
        // Twinkle effect
        float brightness = star.baseBright * (0.7f + 0.3f * sin(t * 2 + star.x * 0.01f));
        drawCircle(star.x, star.y, star.size, brightness, brightness, brightness);
    }
}

void drawGameInterface() {
    // Score display
    std::stringstream ss;
    ss << "SCORE: " << score;
    drawText(10, windowHeight - 30, ss.str());

    // Lives display
    ss.str("");
    ss << "LIVES: " << lives;
    drawText(10, windowHeight - 60, ss.str());

    // Level display
    ss.str("");
    ss << "LEVEL: " << level;
    drawText(10, windowHeight - 90, ss.str());

    // Progress to next level
    if (level < MAX_LEVEL) {
        ss.str("");
        ss << "NEXT LEVEL: " << enemiesDefeated << " / " << enemiesForNextLevel;
        drawText(windowWidth - 250, windowHeight - 30, ss.str());
    }
    else {
        drawText(windowWidth - 250, windowHeight - 30, "MAX LEVEL REACHED!");
    }

    // Active power-ups display
    float y = 120;
    if (multiShot) {
        ss.str("");
        ss << "Multi-shot: " << int(multiShotTime) << "s";
        drawSmallText(10, windowHeight - y, ss.str());
        y += 20;
    }

    if (playerShield) {
        ss.str("");
        ss << "Shield: " << int(shieldTime) << "s";
        drawSmallText(10, windowHeight - y, ss.str());
        y += 20;
    }

    if (playerSpeedBoost > 1.0f) {
        ss.str("");
        ss << "Speed Boost: " << int(speedBoostTime) << "s";
        drawSmallText(10, windowHeight - y, ss.str());
    }

    // Message log display
    y = 50;
    for (const auto& msg : messageLog) {
        drawSmallText(windowWidth - 250, y, msg);
        y += 20;
    }
}

void drawParticles() {
    for (const auto& p : particles) {
        drawCircle(p.x, p.y, p.size, p.r, p.g, p.b, p.alpha);
    }
}

void drawExplosions() {
    for (const auto& e : explosions) {
        drawCircle(e.x, e.y, e.size, e.r, e.g, e.b, e.alpha);
        // Inner glow
        drawCircle(e.x, e.y, e.size * 0.7f, 1.0f, 1.0f, 0.5f, e.alpha * 0.8f);
        // Core
        drawCircle(e.x, e.y, e.size * 0.3f, 1.0f, 1.0f, 1.0f, e.alpha * 0.9f);
    }
}

void drawGameOverScreen() {
    // Semi-transparent overlay
    drawRect(0, 0, windowWidth, windowHeight, 0.0f, 0.0f, 0.0f, 0.7f);

    // Game over text
    drawText(windowWidth / 2 - 60, windowHeight / 2, "GAME OVER\n");
   

    // Final score
    std::stringstream ss;
    ss << "Final Score: " << score;
    drawText(windowWidth / 2 - 70, windowHeight / 2 - 40, ss.str());

    // Level reached
    ss.str("");
    ss << "Highest Level: " << level;
    drawText(windowWidth / 2 - 70, windowHeight / 2 - 80, ss.str());

    // Restart instructions
    drawText(windowWidth / 2 - 120, windowHeight / 2 - 120, " Press 'P' to play again");

    drawText(windowWidth / 2 - 160, windowHeight / 2-160, "  Made By-Ria , Shaurya ");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Background - dark space color
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    // Draw background stars
    drawStars();

    // Draw game objects
    for (const auto& bullet : bullets) {
        drawRect(bullet.x, bullet.y, bullet.width, bullet.height, 1.0f, 1.0f, 0.0f);
    }

    for (const auto& rocket : rockets) {
        drawRocket(rocket);
    }

    for (const auto& enemy : enemies) {
        drawEnemy(enemy);
    }

    for (const auto& powerUp : powerUps) {
        drawPowerUp(powerUp);
    }

    // Draw player
    drawPlayer();

    // Draw particles and explosions
    drawParticles();
    drawExplosions();

    // Draw game interface
    drawGameInterface();

    // Draw game over screen if applicable
    if (gameOver) {
        drawGameOverScreen();
    }

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Space Shooter");

    // Set up 2D projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize random seed
    std::srand(std::time(nullptr));

    // Create starfield
    initStars();

    // Register callbacks
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    // Start enemy spawning
    glutTimerFunc(1000, enemySpawner, 0);

    // Initial game message
    addMessage("Use WASD to move, SPACE to shoot");
    addMessage("R for rockets, ESC to quit");

    // Start main loop
    glutMainLoop();
    return 0;
}
