#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct Bullet
{
    sf::RectangleShape shape;
    sf::Vector2f velocity;
};

struct Particle
{
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
};

enum class PlayerAnim { Idle, Run, Jump };

class Player
{
public:
    Player();

    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

    void handleInput();
    void update(float dt, const std::vector<sf::RectangleShape>& platforms, float worldWidth);
    void draw(sf::RenderWindow& window) const;

    void reset(const sf::Vector2f& position);

    sf::FloatRect getBounds() const;

    const std::vector<Bullet>& getBullets() const;
    std::vector<Bullet>&       getBullets();

    void addScore(int amount);
    int  getScore()  const;
    void resetScore();

    // ── Health system (10 HP per life, 3 lives) ──────────────────────────────
    int  getHealth()    const;   // current HP 0-10
    int  getMaxHealth() const;   // always 10
    void takeBulletHit();        // enemy bullet  -> -1 HP
    void takeBodyHit();          // body contact  -> -10 HP (instant)
    bool isDead()       const;   // true when HP <= 0
    void restoreFullHealth();    // refill to 10

    // Lives (3 total; lose one when HP hits 0)
    int  getLives()         const;
    void loseLife();
    void restoreAllLives();

    void setInvincible(float seconds);
    bool isInvincible()     const;

    // Spread shot: 1-way / 2-way / 3-way / 4-way
    int  getSpreadLevel()   const;
    void upgradeSpread();
    void resetSpread();

    void triggerHitFlash();

private:
    void shoot();
    void buildHumanoid();
    void updateHumanoidPosition(const sf::Vector2f& c, float anim);
    void spawnDeathParticles();

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

    mutable std::vector<Particle> m_particles;

    sf::Vector2f m_centerPosition;
    sf::Vector2f m_velocity;
    bool         m_onGround;
    bool         m_facingRight;
    float        m_animTime;
    PlayerAnim   m_animState;

    std::vector<Bullet> m_bullets;
    float m_fireCooldown;

    int   m_health;
    int   m_maxHealth;
    int   m_lives;
    int   m_score;
    int   m_spreadLevel;

    float m_moveSpeed;
    float m_jumpSpeed;
    float m_gravity;
    float m_bulletSpeed;

    float m_invincibleTime;
    float m_invincibleDuration;
    float m_hitFlashTime;
};