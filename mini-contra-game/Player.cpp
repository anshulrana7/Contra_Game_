#include "Player.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

// ── Colour palette ────────────────────────────────────────────────────────────
static const sf::Color COL_SKIN   (255, 200, 150);
static const sf::Color COL_HELMET ( 50, 100,  50);
static const sf::Color COL_TORSO  ( 60, 110,  60);
static const sf::Color COL_BELT   ( 80,  50,  20);
static const sf::Color COL_ARMS   ( 60, 110,  60);
static const sf::Color COL_PANTS  ( 40,  60,  40);
static const sf::Color COL_BOOT   ( 50,  35,  20);
static const sf::Color COL_GUN    ( 30,  30,  30);
static const sf::Color COL_BULLET (255, 230,  80);
static constexpr float PLAYER_SCALE = 1.6f;

// ─────────────────────────────────────────────────────────────────────────────
Player::Player()
    : m_onGround(false), m_facingRight(true),
      m_fireCooldown(0.f),
      m_health(10), m_maxHealth(10), m_lives(3),
      m_score(0), m_spreadLevel(1),
      m_moveSpeed(290.f), m_jumpSpeed(580.f), m_gravity(1200.f),
      m_bulletSpeed(660.f),
      m_invincibleTime(0.f), m_invincibleDuration(1.5f),
      m_hitFlashTime(0.f),
      m_animTime(0.f), m_animState(PlayerAnim::Idle),
      m_centerPosition(80.f, 300.f)
{
    buildHumanoid();
    updateHumanoidPosition(m_centerPosition, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
void Player::buildHumanoid()
{
    const float s = PLAYER_SCALE;

    m_helmet.setPointCount(5);
    m_helmet.setFillColor(COL_HELMET);

    m_head.setRadius(8.f * s);
    m_head.setFillColor(COL_SKIN);

    m_neck.setSize({5.f * s, 5.f * s});
    m_neck.setFillColor(COL_SKIN);

    m_torso.setSize({14.f * s, 20.f * s});
    m_torso.setFillColor(COL_TORSO);

    m_belt.setSize({14.f * s, 4.f * s});
    m_belt.setFillColor(COL_BELT);

    m_leftUpperArm.setSize({5.f * s, 10.f * s});  m_leftUpperArm.setFillColor(COL_ARMS);
    m_leftForearm.setSize({5.f * s, 9.f * s});    m_leftForearm.setFillColor(COL_SKIN);
    m_rightUpperArm.setSize({5.f * s, 10.f * s}); m_rightUpperArm.setFillColor(COL_ARMS);
    m_rightForearm.setSize({5.f * s, 9.f * s});   m_rightForearm.setFillColor(COL_SKIN);

    m_gun.setSize({14.f * s, 4.f * s});
    m_gun.setFillColor(COL_GUN);

    m_leftThigh.setSize({6.f * s, 11.f * s});  m_leftThigh.setFillColor(COL_PANTS);
    m_leftShin.setSize({5.f * s, 10.f * s});   m_leftShin.setFillColor(COL_PANTS);
    m_leftBoot.setSize({7.f * s, 5.f * s});    m_leftBoot.setFillColor(COL_BOOT);
    m_rightThigh.setSize({6.f * s, 11.f * s}); m_rightThigh.setFillColor(COL_PANTS);
    m_rightShin.setSize({5.f * s, 10.f * s});  m_rightShin.setFillColor(COL_PANTS);
    m_rightBoot.setSize({7.f * s, 5.f * s});   m_rightBoot.setFillColor(COL_BOOT);
}

// ── helpers ───────────────────────────────────────────────────────────────────
static void setRC(sf::RectangleShape& r, float cx, float top)
{
    r.setPosition(std::round(cx - r.getSize().x * 0.5f), std::round(top));
}

static float snap(float v)
{
    return std::round(v);
}

void Player::updateHumanoidPosition(const sf::Vector2f& c, float anim)
{
    m_centerPosition = c;
    const float s = PLAYER_SCALE;
    float dir = m_facingRight ? 1.f : -1.f;

    float headTop = c.y - 42.f * s;
    m_head.setPosition(snap(c.x - 8.f * s), snap(headTop));

    float hx = c.x, hy = headTop - 4.f * s;
    m_helmet.setPoint(0, {hx - 9.f * s, hy + 8.f * s});
    m_helmet.setPoint(1, {hx - 7.f * s, hy});
    m_helmet.setPoint(2, {hx,            hy - 5.f * s});
    m_helmet.setPoint(3, {hx + 7.f * s, hy});
    m_helmet.setPoint(4, {hx + 9.f * s, hy + 8.f * s});

    setRC(m_neck,  c.x, c.y - 26.f * s);
    setRC(m_torso, c.x, c.y - 22.f * s);
    setRC(m_belt,  c.x, c.y -  2.f * s);

    // stride: horizontal-only offset so legs alternate left/right with no Y bobbing
    float stride = (m_animState == PlayerAnim::Run) ? std::sin(anim) * 5.f * s : 0.f;

    // Arms – fixed Y, only X varies slightly with arm swing
    float armSwing = (m_animState == PlayerAnim::Run) ? std::sin(anim) * 2.f * s : 0.f;
    float armY = c.y - 20.f * s;
    setRC(m_leftUpperArm,  c.x - 10.f * s * dir + armSwing, armY);
    setRC(m_leftForearm,   c.x - 11.f * s * dir + armSwing, armY + 10.f * s);
    setRC(m_rightUpperArm, c.x +  7.f * s * dir - armSwing, armY);
    setRC(m_rightForearm,  c.x +  8.f * s * dir - armSwing, armY + 10.f * s);
    m_gun.setPosition(snap(m_facingRight ? c.x + 7.f * s : c.x - 21.f * s), snap(armY + 14.f * s));

    // Legs – stride shifts X only, Y is always fixed
    float lt = c.y + 2.f * s;
    m_leftThigh.setPosition( snap(c.x - 7.f * s + stride),  snap(lt));
    m_leftShin.setPosition(  snap(c.x - 6.f * s + stride),  snap(lt + 11.f * s));
    m_leftBoot.setPosition(  snap(c.x - 7.f * s + stride),  snap(lt + 21.f * s));
    m_rightThigh.setPosition(snap(c.x + 1.f * s - stride),  snap(lt));
    m_rightShin.setPosition( snap(c.x + 1.f * s - stride),  snap(lt + 11.f * s));
    m_rightBoot.setPosition( snap(c.x           - stride),  snap(lt + 21.f * s));
}

void Player::setPosition(const sf::Vector2f& pos) { updateHumanoidPosition(pos, m_animTime); }
sf::Vector2f Player::getPosition() const { return m_centerPosition; }

// ─────────────────────────────────────────────────────────────────────────────
void Player::handleInput()
{
    m_velocity.x = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    { m_velocity.x = -m_moveSpeed; m_facingRight = false; }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    { m_velocity.x =  m_moveSpeed; m_facingRight = true;  }

    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W)   ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::Up)  ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) && m_onGround)
    { m_velocity.y = -m_jumpSpeed; m_onGround = false; }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
        shoot();
}

// ─────────────────────────────────────────────────────────────────────────────
void Player::update(float dt, const std::vector<sf::RectangleShape>& platforms, float worldWidth)
{
    if (m_invincibleTime > 0.f) m_invincibleTime -= dt;
    if (m_fireCooldown   > 0.f) m_fireCooldown   -= dt;
    if (m_hitFlashTime   > 0.f) m_hitFlashTime   -= dt;

    if (!m_onGround)             m_animState = PlayerAnim::Jump;
    else if (m_velocity.x != 0.f) m_animState = PlayerAnim::Run;
    else                          m_animState = PlayerAnim::Idle;

    if (m_animState == PlayerAnim::Run) m_animTime += dt * 8.f;
    else                                m_animTime  = 0.f;

    m_velocity.y += m_gravity * dt;

    sf::Vector2f pos = m_centerPosition;
    const float bW = 12.f * PLAYER_SCALE, bH = 20.f * PLAYER_SCALE;

    // ── Horizontal ────────────────────────────────────────────────────────────
    pos.x += m_velocity.x * dt;
    sf::FloatRect bb{ pos.x - bW * 0.5f, pos.y - bH * 0.5f, bW, bH };
    for (const auto& p : platforms) {
        if (bb.intersects(p.getGlobalBounds())) {
            if (m_velocity.x > 0.f) pos.x = p.getGlobalBounds().left - bW;
            else                    pos.x = p.getGlobalBounds().left + p.getGlobalBounds().width + bW;
            bb.left = pos.x - bW * 0.5f;
        }
    }
    pos.x = std::max(bW, std::min(pos.x, worldWidth - bW));

    // ── Vertical ──────────────────────────────────────────────────────────────
    pos.y += m_velocity.y * dt;
    bb.top = pos.y - bH * 0.5f;
    m_onGround = false;
    for (const auto& p : platforms) {
        sf::FloatRect pb = p.getGlobalBounds();
        if (bb.intersects(pb)) {
            if (m_velocity.y > 0.f) { pos.y = pb.top - 22.f * PLAYER_SCALE; m_velocity.y = 0.f; m_onGround = true; }
            else                    { pos.y = pb.top + pb.height + bH * 0.5f; m_velocity.y = 0.f; }
            bb.top = pos.y - bH * 0.5f;
        }
    }

    updateHumanoidPosition(pos, m_animTime);

    // ── Player bullets ────────────────────────────────────────────────────────
    for (auto& b : m_bullets)
        b.shape.move(b.velocity * dt);

    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [worldWidth](const Bullet& b){
            float x = b.shape.getPosition().x;
            return x < -100.f || x > worldWidth + 100.f;
        }), m_bullets.end());

    // ── Particles ─────────────────────────────────────────────────────────────
    for (auto& p : m_particles) {
        p.velocity.y += 800.f * dt;
        p.shape.move(p.velocity * dt);
        p.lifetime -= dt;
        float a = std::max(0.f, p.lifetime / p.maxLifetime);
        sf::Color c = p.shape.getFillColor();
        c.a = (sf::Uint8)(a * 255);
        p.shape.setFillColor(c);
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
        [](const Particle& p){ return p.lifetime <= 0.f; }), m_particles.end());
}

// ─────────────────────────────────────────────────────────────────────────────
void Player::draw(sf::RenderWindow& window) const
{
    // Blink when invincible: skip odd frames
    if (m_invincibleTime > 0.f) {
        int frame = static_cast<int>(m_invincibleTime * 14.f);
        if (frame % 2 != 0) {
            for (const auto& p : m_particles) window.draw(p.shape);
            return;
        }
    }

    window.draw(m_rightThigh); window.draw(m_rightShin); window.draw(m_rightBoot);
    window.draw(m_leftUpperArm); window.draw(m_leftForearm);
    window.draw(m_torso); window.draw(m_belt); window.draw(m_neck);
    window.draw(m_head);  window.draw(m_helmet);
    window.draw(m_leftThigh); window.draw(m_leftShin); window.draw(m_leftBoot);
    window.draw(m_rightUpperArm); window.draw(m_rightForearm);
    window.draw(m_gun);

    // Hit flash overlay
    if (m_hitFlashTime > 0.f) {
        sf::RectangleShape flash({30.f * PLAYER_SCALE, 60.f * PLAYER_SCALE});
        flash.setFillColor(sf::Color(255, 80, 80, 180));
        flash.setPosition(m_centerPosition.x - 15.f * PLAYER_SCALE, m_centerPosition.y - 48.f * PLAYER_SCALE);
        window.draw(flash);
    }

    for (const auto& b : m_bullets)  window.draw(b.shape);
    for (const auto& p : m_particles) window.draw(p.shape);
}

// ─────────────────────────────────────────────────────────────────────────────
void Player::shoot()
{
    if (m_fireCooldown > 0.f) return;
    m_fireCooldown = 0.15f;

    float gunTipX = m_centerPosition.x + (m_facingRight ? 22.f * PLAYER_SCALE : -22.f * PLAYER_SCALE);
    float gunTipY = m_centerPosition.y - 8.f * PLAYER_SCALE;

    std::vector<float> angles;
    if      (m_spreadLevel == 1) angles = {0.f};
    else if (m_spreadLevel == 2) angles = {-8.f, 8.f};
    else if (m_spreadLevel == 3) angles = {-14.f, 0.f, 14.f};
    else                         angles = {-18.f, -6.f, 6.f, 18.f};

    for (float deg : angles) {
        float rad = deg * 3.14159f / 180.f;
        float vx  = m_facingRight ? m_bulletSpeed : -m_bulletSpeed;
        float vy  = vx * std::tan(rad);

        Bullet b;
        b.shape.setSize({14.f * PLAYER_SCALE, 4.f * PLAYER_SCALE});
        b.shape.setFillColor(COL_BULLET);
        b.shape.setOrigin(0.f, 2.f * PLAYER_SCALE);
        b.shape.setPosition(gunTipX, gunTipY);
        b.velocity = {vx, vy};
        m_bullets.push_back(b);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Player::spawnDeathParticles()
{
    static const sf::Color cols[] = {
        {255,100,50},{255,220,80},{200,50,50},{255,255,255}
    };
    for (int i = 0; i < 16; ++i) {
        Particle p;
        float angle = (i / 16.f) * 6.2832f;
        float speed = 80.f + (std::rand() % 120);
        p.velocity  = { std::cos(angle) * speed, std::sin(angle) * speed - 60.f };
        p.shape.setSize({4.f + (float)(std::rand() % 4), 4.f + (float)(std::rand() % 4)});
        p.shape.setFillColor(cols[std::rand() % 4]);
        p.shape.setPosition(m_centerPosition);
        p.maxLifetime = p.lifetime = 0.5f + (std::rand() % 50) * 0.01f;
        m_particles.push_back(p);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Player::reset(const sf::Vector2f& pos)
{
    spawnDeathParticles();
    updateHumanoidPosition(pos, 0.f);
    m_velocity    = {0.f, 0.f};
    m_onGround    = false;
    m_bullets.clear();
    m_invincibleTime = m_invincibleDuration;
    m_animTime    = 0.f;
    m_health      = m_maxHealth;   // refill HP on respawn
    m_spreadLevel = 1;
}

// ── Health system ─────────────────────────────────────────────────────────────
int  Player::getHealth()    const { return m_health; }
int  Player::getMaxHealth() const { return m_maxHealth; }
bool Player::isDead()       const { return m_health <= 0; }

void Player::takeBulletHit()
{
    if (m_invincibleTime > 0.f) return;
    m_hitFlashTime = 0.14f;
    m_invincibleTime = 0.5f;   // brief i-frames after bullet hit
    if (--m_health <= 0) m_health = 0;
}

void Player::takeBodyHit()
{
    if (m_invincibleTime > 0.f) return;
    m_health = 0;
}

void Player::restoreFullHealth() { m_health = m_maxHealth; }

// ── Lives ─────────────────────────────────────────────────────────────────────
int  Player::getLives()         const { return m_lives; }
void Player::loseLife()               { if (m_lives > 0) --m_lives; }
void Player::restoreAllLives()        { m_lives = 3; restoreFullHealth(); }

// ── Misc ──────────────────────────────────────────────────────────────────────
sf::FloatRect Player::getBounds() const { return m_torso.getGlobalBounds(); }

const std::vector<Bullet>& Player::getBullets() const { return m_bullets; }
std::vector<Bullet>&       Player::getBullets()       { return m_bullets; }

void Player::addScore(int v) { m_score += v; }
int  Player::getScore()  const { return m_score; }
void Player::resetScore()      { m_score = 0; }

void Player::setInvincible(float d) { m_invincibleTime = d; }
bool Player::isInvincible()   const { return m_invincibleTime > 0.f; }

int  Player::getSpreadLevel() const { return m_spreadLevel; }
void Player::upgradeSpread()  { if (m_spreadLevel < 4) ++m_spreadLevel; }
void Player::resetSpread()    { m_spreadLevel = 1; }

void Player::triggerHitFlash() { m_hitFlashTime = 0.14f; }