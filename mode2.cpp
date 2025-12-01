#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <ctime>
#include <GL/freeglut.h>
#include <iostream>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

const float PI = 3.14159265358979323846f;


const DWORD SHOOT_SOUND_DURATION = 200;
const DWORD DIE_SOUND_DURATION = 500;

struct Bullet {
    float x, y, z;
    float dx, dy, dz;
};

enum EnemyType { BAYMAX, CATERPILLAR };

struct Enemy {
    float x, y, z;
    int bodyHits = 0;
    bool isDead = false;
    bool isCloseToPlayer = false;
    EnemyType type;
};

std::vector<Bullet> bullets;
std::vector<Enemy> enemies;

const int MAX_BULLETS = 6;
int bulletsLeft = MAX_BULLETS;
int mouseX = 400;
int mouseY = 400;
bool fullScreen = true;
int windowWidth = 800;
int windowHeight = 800;
int score = 0;
float globalTime = 0.0f;
int totalKills = 0;
int lives = 3;
float enemySpawnTime = 0.0f;
bool playedWadySound = false;
bool playedAshSound = false;

// Sound timing variables
DWORD lastShootSoundTime = 0; // Last time shoot sound was played
DWORD lastDieSoundTime = 0;   // Last time die sound was played

// Level system variables
int currentLevel = 1;
int killsToNextLevel = 10;
float enemySpeed = 0.02f;
float enemySpawnInterval = 2.0f;
const float BULLET_SPEED = 0.8f;
const float PLAYER_HIT_RADIUS = 3.0f;
const float PLAYER_Z_POSITION = 5.0f;

bool gameOver = false;

// Sound functions
void playShootSound() {
    DWORD currentTime = GetTickCount();
    // Only play if enough time has passed since last shoot sound
    if (currentTime - lastShootSoundTime >= SHOOT_SOUND_DURATION) {
        if (PlaySound(TEXT("shoot.wav"), NULL, SND_FILENAME | SND_ASYNC)) {
            lastShootSoundTime = currentTime;
        }
        else {
            DWORD errorCode = GetLastError();
            std::cerr << "Error: Failed to play shoot.wav, Windows error code: " << errorCode << std::endl;
        }
    }
}

void playDieSound() {
    DWORD currentTime = GetTickCount();
    // Only play if shoot sound is not playing and enough time has passed since last die sound
    if (currentTime - lastShootSoundTime >= SHOOT_SOUND_DURATION &&
        currentTime - lastDieSoundTime >= DIE_SOUND_DURATION) {
        if (PlaySound(TEXT("die.wav"), NULL, SND_FILENAME | SND_ASYNC)) {
            lastDieSoundTime = currentTime;
        }
        else {
            DWORD errorCode = GetLastError();
            std::cerr << "Error: Failed to play die.wav, Windows error code: " << errorCode << std::endl;
        }
    }
}

void playWadySound() {
    if (!PlaySound(TEXT("wady.wav"), NULL, SND_FILENAME | SND_ASYNC)) {
        DWORD errorCode = GetLastError();
        std::cerr << "Error: Failed to play wady.wav, Windows error code: " << errorCode << std::endl;
    }
}

void playAshSound() {
    if (!PlaySound(TEXT("ash.wav"), NULL, SND_FILENAME | SND_ASYNC)) {
        DWORD errorCode = GetLastError();
        std::cerr << "Error: Failed to play ash.wav, Windows error code: " << errorCode << std::endl;
    }
}

void normalize(float& x, float& y, float& z) {
    float len = sqrt(x * x + y * y + z * z);
    if (len > 0) {
        x /= len;
        y /= len;
        z /= len;
    }
}

void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void updateMousePosition(int x, int y) {
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}

void drawCrosshair() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0, 0.0, 0.0);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(mouseX - 10, windowHeight - mouseY);
    glVertex2f(mouseX + 10, windowHeight - mouseY);
    glVertex2f(mouseX, windowHeight - mouseY - 10);
    glVertex2f(mouseX, windowHeight - mouseY + 10);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(20, windowHeight - 30, "Score: " + std::to_string(score));
    drawText(windowWidth - 120, windowHeight - 30, "Kills: " + std::to_string(totalKills));
    drawText(windowWidth / 2 - 60, windowHeight - 30, "Level: " + std::to_string(currentLevel));
    drawText(20, windowHeight - 60, "Lives: " + std::to_string(lives));
    drawText(20, windowHeight - 90, "Bullets: " + std::to_string(bulletsLeft));

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawBullets() {
    glColor3f(1.0f, 1.0f, 0.0f);
    for (auto& bullet : bullets) {
        glPushMatrix();
        glTranslatef(bullet.x, bullet.y, bullet.z);
        glutSolidSphere(0.1, 8, 8);
        glPopMatrix();
    }
}

void drawLives() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < bulletsLeft; ++i) {
        float x = 20 + i * 30;
        float y = windowHeight - 90;
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + 20, y);
        glVertex2f(x + 20, y + 20);
        glVertex2f(x, y + 20);
        glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawBaymaxEnemy(const Enemy& enemy) {
    glPushMatrix();
    glTranslatef(enemy.x, enemy.y, enemy.z);

    float limbAngle = 30.0f * sin(globalTime * 6);

    // Body
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glScalef(1.0, 1.5, 0.5);
    glutSolidSphere(0.6, 20, 20);
    glPopMatrix();

    // Arms
    for (float sign : {-1.0f, 1.0f}) {
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glTranslatef(0.6f * sign, 0.2f, 0.0f);
        glRotatef(limbAngle * sign, 1.0f, 0.0f, 0.0f);
        glScalef(0.3, 0.6, 0.3);
        glutSolidSphere(0.5, 20, 20);
        glPopMatrix();
    }

    // Head
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(0.0f, 1.3f, 0.0f);
    glutSolidSphere(0.4, 20, 20);

    // Eyes
    glColor3f(0.0, 0.0, 0.0);
    glPushMatrix();
    glTranslatef(-0.1f, 0.05f, 0.35f);
    glutSolidSphere(0.05, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.1f, 0.05f, 0.35f);
    glutSolidSphere(0.05, 10, 10);
    glPopMatrix();

    // Line between eyes
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex3f(-0.1f, 0.05f, 0.35f);
    glVertex3f(0.1f, 0.05f, 0.35f);
    glEnd();
    glPopMatrix();

    // Legs
    for (float sign : {-1.0f, 1.0f}) {
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glTranslatef(0.2f * sign, -1.1f, 0.0f);
        glRotatef(-limbAngle * sign, 1.0f, 0.0f, 0.0f);
        glScalef(0.3, 0.5, 0.3);
        glutSolidSphere(0.5, 20, 20);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawCaterpillarEnemy(const Enemy& enemy) {
    glPushMatrix();
    glTranslatef(enemy.x, enemy.y, enemy.z);

    float segmentRadius = 0.3f;
    float yOffset = 0.6f;
    float legAngle = 30.0f * sin(globalTime * 6);

    // Body segments
    glColor3f(0.2f, 0.8f, 0.2f);
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(0.0f, -1.0f + i * yOffset, 0.0f);
        glutSolidSphere(segmentRadius, 20, 20);
        glPopMatrix();
    }

    // Head
    glColor3f(0.3f, 0.9f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, -1.0f + 4 * yOffset, 0.0f);
    glutSolidSphere(segmentRadius * 1.2f, 20, 20);

    // Eyes
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.15f, 0.2f, 0.0f);
    glutSolidSphere(0.05f, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.15f, 0.2f, 0.0f);
    glutSolidSphere(0.05f, 10, 10);
    glPopMatrix();

    // Smile
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 20; ++i) {
        float t = static_cast<float>(i) / 20;
        float x = (1 - t) * (1 - t) * (-0.2f) + 2 * (1 - t) * t * 0.0f + t * t * 0.2f;
        float y = (1 - t) * (1 - t) * (-0.1f) + 2 * (1 - t) * t * (-0.3f) + t * t * (-0.1f);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();
    glPopMatrix();

    // Antennae
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(-0.25f, -1.0f + 4 * yOffset + 0.4f, 0.0f);
    glVertex3f(-0.4f, -1.0f + 4 * yOffset + 0.6f, 0.0f);
    glVertex3f(0.25f, -1.0f + 4 * yOffset + 0.4f, 0.0f);
    glVertex3f(0.4f, -1.0f + 4 * yOffset + 0.6f, 0.0f);
    glEnd();

    // Legs
    glColor3f(0.2f, 0.8f, 0.2f);
    for (int i = 0; i < 4; ++i) {
        for (float sign : {-1.0f, 1.0f}) {
            glPushMatrix();
            glTranslatef(0.3f * sign, -1.1f + i * yOffset, 0.0f);
            glRotatef(legAngle * sign, 1.0f, 0.0f, 0.0f);
            glScalef(0.2f, 0.4f, 0.2f);
            glutSolidSphere(0.3f, 10, 10);
            glPopMatrix();
        }
    }

    glPopMatrix();
}

void drawScene() {
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, -1.0f, -20.0f);
    glVertex3f(10.0f, -1.0f, -20.0f);
    glVertex3f(10.0f, -1.0f, 5.0f);
    glVertex3f(-10.0f, -1.0f, 5.0f);
    glEnd();

    for (const auto& enemy : enemies) {
        if (!enemy.isDead) {
            if (enemy.type == BAYMAX) {
                drawBaymaxEnemy(enemy);
            }
            else {
                drawCaterpillarEnemy(enemy);
            }
        }
    }
}

void drawGameOverScreen() {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();

    glRasterPos2f(0, 0);
    glColor3f(1.0f, 0.0f, 0.0f);
    drawText(windowWidth / 2 - 100, windowHeight / 2 + 60, "GAME OVER");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(windowWidth / 2 - 100, windowHeight / 2 + 20, "Final Score: " + std::to_string(score));
    drawText(windowWidth / 2 - 100, windowHeight / 2 - 10, "Total Kills: " + std::to_string(totalKills));
    drawText(windowWidth / 2 - 100, windowHeight / 2 - 40, "Level Reached: " + std::to_string(currentLevel));

    glColor3f(0.0f, 1.0f, 0.0f);
    drawText(windowWidth / 2 - 100, windowHeight / 2 - 100, "Press 'R' to Play Again");
    drawText(windowWidth / 2 - 100, windowHeight / 2 - 130, "Press 'Q' or ESC to Quit");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void resetGame() {
    gameOver = false;
    lives = 3;
    score = 0;
    totalKills = 0;
    currentLevel = 1;
    killsToNextLevel = 10;
    enemySpeed = 0.02f;
    enemySpawnInterval = 2.0f;
    bullets.clear();
    enemies.clear();
    bulletsLeft = MAX_BULLETS;
    enemySpawnTime = globalTime;
    playedWadySound = false;
    playedAshSound = false;
    lastShootSoundTime = 0; // Reset sound timers
    lastDieSoundTime = 0;
}

void update(int value) {
    if (!gameOver) {
        globalTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

        if (totalKills >= killsToNextLevel && currentLevel < 5) {
            currentLevel++;
            killsToNextLevel += 10 + (currentLevel * 2);

            switch (currentLevel) {
            case 2:
                enemySpeed = 0.03f;
                enemySpawnInterval = 1.8f;
                break;
            case 3:
                enemySpeed = 0.04f;
                enemySpawnInterval = 1.4f;
                break;
            case 4:
                enemySpeed = 0.06f;
                enemySpawnInterval = 1.0f;
                break;
            case 5:
                enemySpeed = 0.08f;
                enemySpawnInterval = 0.6f;
                break;
            }
        }

        if (globalTime - enemySpawnTime >= enemySpawnInterval) {
            int enemiesToSpawn = 1;
            if (currentLevel >= 4) enemiesToSpawn = 2;
            if (currentLevel == 5) enemiesToSpawn = 3;

            for (int i = 0; i < enemiesToSpawn; i++) {
                EnemyType type = (rand() % 2 == 0) ? BAYMAX : CATERPILLAR;
                enemies.push_back({ (rand() % 14) - 7.0f, 0.0f, -20.0f - (rand() % 5), 0, false, false, type });
            }
            enemySpawnTime = globalTime;
        }

        for (auto& bullet : bullets) {
            bullet.x += bullet.dx * BULLET_SPEED;
            bullet.y += bullet.dy * BULLET_SPEED;
            bullet.z += bullet.dz * BULLET_SPEED;
        }

        for (auto& enemy : enemies) {
            if (!enemy.isDead) {
                enemy.z += enemySpeed;

                if (enemy.z >= PLAYER_Z_POSITION - PLAYER_HIT_RADIUS) {
                    lives--;
                    enemy.isCloseToPlayer = true;

                    if (lives <= 0) {
                        gameOver = true;
                    }
                }
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) {
            return e.isCloseToPlayer;
            }), enemies.end());

        for (auto& bullet : bullets) {
            for (auto& enemy : enemies) {
                if (enemy.isDead || enemy.isCloseToPlayer) continue;

                float headY, bodyY;
                if (enemy.type == BAYMAX) {
                    headY = enemy.y + 1.3f;
                    bodyY = enemy.y;
                }
                else {
                    headY = enemy.y + 1.4f;
                    bodyY = enemy.y + 0.2f;
                }

                // Head hit detection
                float dx = bullet.x - enemy.x;
                float dy = bullet.y - headY;
                float dz = bullet.z - enemy.z;
                float dist = sqrt(dx * dx + dy * dy + dz * dz);

                if (dist < 0.4f) {
                    enemy.isDead = true;
                    score += 300 * currentLevel;
                    totalKills++;
                    bullet.z = 1000.0f;
                    playDieSound();
                    if (totalKills >= 5 && !playedWadySound) {
                        playWadySound();
                        playedWadySound = true;
                    }
                    if (totalKills >= 10 && !playedAshSound) {
                        playAshSound();
                        playedAshSound = true;
                    }
                    break;
                }

                // Body hit detection
                dy = bullet.y - bodyY;
                dist = sqrt(dx * dx + dy * dy + dz * dz);
                if (dist < 0.6f) {
                    enemy.bodyHits++;
                    int hitsNeeded = 3;
                    if (currentLevel >= 4) hitsNeeded = 4;
                    if (enemy.bodyHits >= hitsNeeded) {
                        enemy.isDead = true;
                        score += 100 * currentLevel;
                        totalKills++;
                        playDieSound();
                        if (totalKills >= 5 && !playedWadySound) {
                            playWadySound();
                            playedWadySound = true;
                        }
                        if (totalKills >= 10 && !playedAshSound) {
                            playAshSound();
                            playedAshSound = true;
                        }
                    }
                    bullet.z = 1000.0f;
                    break;
                }
            }
        }

        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b) {
            return b.z < -50.0f || b.z > 50.0f || b.x < -50.0f || b.x > 50.0f || b.y < -50.0f || b.y > 50.0f;
            }), bullets.end());
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0,
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0);

    if (gameOver) {
        drawGameOverScreen();
    }
    else {
        drawScene();
        drawBullets();
        drawLives();
        drawCrosshair();
    }

    glutSwapBuffers();
}

void mouseClick(int button, int state, int x, int y) {
    if (!gameOver && state == GLUT_DOWN && button == GLUT_LEFT_BUTTON && bulletsLeft > 0) {
        int viewport[4];
        double modelview[16], projection[16];
        float winX = static_cast<float>(mouseX);
        float winY = static_cast<float>(windowHeight - mouseY);
        double nearX, nearY, nearZ;
        double farX, farY, farZ;

        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);

        gluUnProject(winX, winY, 0.0, modelview, projection, viewport, &nearX, &nearY, &nearZ);
        gluUnProject(winX, winY, 1.0, modelview, projection, viewport, &farX, &farY, &farZ);

        float dx = farX - nearX;
        float dy = farY - nearY;
        float dz = farZ - nearZ;
        normalize(dx, dy, dz);

        bullets.push_back({ static_cast<float>(nearX), static_cast<float>(nearY), static_cast<float>(nearZ), dx, dy, dz });
        bulletsLeft--;

        // Play shoot sound
        playShootSound();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        bulletsLeft = MAX_BULLETS;
    }
    glutPostRedisplay();
}

void handleKeyboard(unsigned char key, int x, int y) {
    if (gameOver) {
        if (key == 'r' || key == 'R') {
            resetGame();
            glutPostRedisplay();
        }
        else if (key == 27 || key == 'q' || key == 'Q') {
            exit(0);
        }
    }
    else {
        if (key == 27 || key == 'q' || key == 'Q') {
            exit(0);
        }
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspect, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glutSetCursor(GLUT_CURSOR_NONE);
    srand(static_cast<unsigned>(time(0)));
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Baymax and Caterpillar Rail Shooter");
    glutFullScreen();

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutKeyboardFunc(handleKeyboard);
    glutPassiveMotionFunc(updateMousePosition);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}