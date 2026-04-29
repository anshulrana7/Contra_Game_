#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class EnemyType { Normal, Fast, Boss };

struct EBullet {
    sf::RectangleShape shape;
    sf::Vector2f       velocity;
};

struct EParticle {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
};

class Enemy
{
public:
    Enemy(EnemyType type, const sf::Vector2f& position);

    void update(float dt, const sf::Vector2f& playerPos,
                const std::vector<sf::RectangleShape>& platforms);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getBounds() const;

    bool isAlive() const;
    void takeHit();

    EnemyType getType()      const;
    int       getMaxHealth() const;
    int       getHealth()    const;

    // Enemy bullets — Level reads these to check player collision
    const std::vector<EBullet>& getBullets() const;
    std::vector<EBullet>&       getBullets();

private:
    void buildHumanoid();
    void updateHumanoidPosition(const sf::Vector2f& c, float animAngle);
    void spawnDeathParticles();
    void shoot(const sf::Vector2f& playerPos);

    // ── Humanoid parts ────────────────────────────────────────────────────────
    sf::ConvexShape    m_helmet;
    sf::CircleShape    m_head;
    sf::RectangleShape m_neck;
    sf::RectangleShape m_torso;
    sf::RectangleShape m_belt;
    sf::RectangleShape m_leftUpperArm,  m_leftForearm;
    sf::RectangleShape m_rightUpperArm, m_rightForearm;
    sf::RectangleShape m_gun;
    sf::RectangleShape m_leftThigh,  m_leftShin,  m_leftBoot;
    sf::RectangleShape m_rightThigh, m_rightShin, m_rightBoot;

    sf::RectangleShape m_hpBarBg;
    sf::RectangleShape m_hpBar;

    mutable std::vector<EParticle> m_particles;
    std::vector<EBullet>           m_bullets;

    sf::Vector2f m_centerPosition;
    sf::Vector2f m_velocity;
    EnemyType    m_type;

    float m_speed;
    float m_gravity;
    int   m_health;
    int   m_maxHealth;
    bool  m_alive;

    float m_animTime;
    float m_hitFlashTime;
    float m_scale;

    // Firing
    float m_fireCooldown;
    float m_fireInterval;   // seconds between shots
    float m_bulletSpeed;
};