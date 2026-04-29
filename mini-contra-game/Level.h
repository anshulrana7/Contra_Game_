#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Enemy.h"
#include "Player.h"

struct Checkpoint {
    sf::Vector2f position;
    bool activated;
    sf::RectangleShape pole;
    sf::ConvexShape    flag;
};

class Level
{
public:
    explicit Level(int index);

    void reset(Player& player);
    void update(float dt, Player& player);
    void draw(sf::RenderWindow& window);

    bool  isCompleted()   const;
    int   getIndex()      const;
    float getWorldWidth() const;

private:
    void initLevel();
    void grantKillCheckpointReward(Player& player);
    void handleCheckpoints(Player& player);
    void handlePlayerCollisions(Player& player);
    void handlePlayerBullets(Player& player);
    void handleEnemyBullets(Player& player);   // ← NEW
    void respawnPlayer(Player& player);

    int   m_index;
    float m_worldWidth;
    bool  m_completed;

    std::vector<sf::RectangleShape> m_platforms;
    std::vector<Enemy>              m_enemies;
    std::vector<Checkpoint>         m_checkpoints;

    sf::Vector2f m_respawnPoint;
    int          m_killCount;
    int          m_nextCheckpointKill;
    bool         m_nextRewardHealth;

    sf::Sprite  m_background;
    sf::Texture m_backgroundTexture;
};