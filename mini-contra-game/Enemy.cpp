#include "Enemy.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ── helper ────────────────────────────────────────────────────────────────────
static void setRC(sf::RectangleShape& r, float cx, float top)
{
    r.setPosition(std::round(cx - r.getSize().x * 0.5f), std::round(top));
}

static float snap(float v)
{
    return std::round(v);
}

// ── colour palettes ───────────────────────────────────────────────────────────
struct Palette {
    sf::Color skin, helm, torso, belt, pants, boot, gun;
};

static const Palette PAL_NORMAL = {
    {255,180,130}, {130,30,30}, {160,40,40}, {100,60,20}, {100,30,30}, {60,35,20}, {30,30,30}
};
static const Palette PAL_FAST = {
    {200,220,255}, {60,60,180}, {70,70,200}, {40,40,130}, {50,50,160}, {20,20,80}, {50,50,50}
};
static const Palette PAL_BOSS = {
    {180,100,80}, {80,10,10}, {100,10,10}, {60,30,10}, {70,10,10}, {40,10,10}, {20,20,20}
};

static void applyPalette(const Palette& p,
    sf::ConvexShape& helm, sf::CircleShape& head, sf::RectangleShape& neck,
    sf::RectangleShape& torso, sf::RectangleShape& belt,
    sf::RectangleShape& lua, sf::RectangleShape& lfa,
    sf::RectangleShape& rua, sf::RectangleShape& rfa,
    sf::RectangleShape& gun,
    sf::RectangleShape& lt, sf::RectangleShape& ls, sf::RectangleShape& lb,
    sf::RectangleShape& rt, sf::RectangleShape& rs, sf::RectangleShape& rb)
{
    helm.setFillColor(p.helm);   head.setFillColor(p.skin);  neck.setFillColor(p.skin);
    torso.setFillColor(p.torso); belt.setFillColor(p.belt);
    lua.setFillColor(p.torso);   lfa.setFillColor(p.skin);
    rua.setFillColor(p.torso);   rfa.setFillColor(p.skin);
    gun.setFillColor(p.gun);
    lt.setFillColor(p.pants); ls.setFillColor(p.pants); lb.setFillColor(p.boot);
    rt.setFillColor(p.pants); rs.setFillColor(p.pants); rb.setFillColor(p.boot);
}

// ─────────────────────────────────────────────────────────────────────────────
Enemy::Enemy(EnemyType type, const sf::Vector2f& position)
    : m_type(type), m_alive(true),
      m_velocity(0.f, 0.f), m_gravity(1200.f),
      m_animTime(0.f), m_hitFlashTime(0.f),
      m_fireCooldown(0.f)
{
    if (type == EnemyType::Normal) {
        m_speed = 70.f;  m_maxHealth = m_health = 2;
        m_scale = 1.45f;
        m_fireInterval  = 2.5f;   // fires every 2.5s
        m_bulletSpeed   = 280.f;
    } else if (type == EnemyType::Fast) {
        m_speed = 150.f; m_maxHealth = m_health = 1;
        m_scale = 1.35f;
        m_fireInterval  = 1.8f;   // faster fire rate
        m_bulletSpeed   = 320.f;
    } else { // Boss
        m_speed = 80.f;  m_maxHealth = m_health = 20;
        m_scale = 2.8f;
        m_fireInterval  = 1.0f;   // fires every second
        m_bulletSpeed   = 350.f;
    }

    // Stagger initial cooldown so enemies don't all fire at once
    m_fireCooldown = (float)(std::rand() % 100) / 100.f * m_fireInterval;

    buildHumanoid();
    updateHumanoidPosition(position, 0.f);

    float barW = (m_type == EnemyType::Boss) ? 70.f : 24.f;
    m_hpBarBg.setSize({barW, 5.f});
    m_hpBarBg.setFillColor(sf::Color(60, 0, 0, 200));
    m_hpBar.setSize({barW, 5.f});
    m_hpBar.setFillColor(sf::Color(220, 50, 50));
}

// ─────────────────────────────────────────────────────────────────────────────
void Enemy::buildHumanoid()
{
    float s = m_scale;
    m_helmet.setPointCount(5);
    m_head.setRadius(8.f * s);
    m_neck.setSize({5.f*s, 5.f*s});
    m_torso.setSize({14.f*s, 20.f*s});
    m_belt.setSize({14.f*s, 4.f*s});
    m_leftUpperArm.setSize({5.f*s,10.f*s});  m_leftForearm.setSize({5.f*s, 9.f*s});
    m_rightUpperArm.setSize({5.f*s,10.f*s}); m_rightForearm.setSize({5.f*s, 9.f*s});
    m_gun.setSize({14.f*s, 4.f*s});
    m_leftThigh.setSize({6.f*s,11.f*s});  m_leftShin.setSize({5.f*s,10.f*s});  m_leftBoot.setSize({7.f*s,5.f*s});
    m_rightThigh.setSize({6.f*s,11.f*s}); m_rightShin.setSize({5.f*s,10.f*s}); m_rightBoot.setSize({7.f*s,5.f*s});

    const Palette& pal = (m_type == EnemyType::Normal) ? PAL_NORMAL :
                         (m_type == EnemyType::Fast)   ? PAL_FAST   : PAL_BOSS;

    applyPalette(pal,
        m_helmet, m_head, m_neck, m_torso, m_belt,
        m_leftUpperArm, m_leftForearm, m_rightUpperArm, m_rightForearm, m_gun,
        m_leftThigh, m_leftShin, m_leftBoot,
        m_rightThigh, m_rightShin, m_rightBoot);
}

// ─────────────────────────────────────────────────────────────────────────────
void Enemy::updateHumanoidPosition(const sf::Vector2f& c, float anim)
{
    m_centerPosition = c;
    float s = m_scale;

    float headTop = c.y - 42.f * s;
    m_head.setPosition(snap(c.x - 8.f * s), snap(headTop));

    float hx = c.x, hy = headTop - 4.f * s;
    m_helmet.setPoint(0, {hx - 9.f*s, hy + 8.f*s});
    m_helmet.setPoint(1, {hx - 7.f*s, hy});
    m_helmet.setPoint(2, {hx,          hy - 5.f*s});
    m_helmet.setPoint(3, {hx + 7.f*s, hy});
    m_helmet.setPoint(4, {hx + 9.f*s, hy + 8.f*s});

    setRC(m_neck,  c.x, c.y - 26.f*s);
    setRC(m_torso, c.x, c.y - 22.f*s);
    setRC(m_belt,  c.x, c.y -  2.f*s);

    // stride: X-only, no Y bobbing
    float stride   = std::sin(anim) * 5.f * s;
    float armSwing = std::sin(anim) * 2.f * s;
    float armY     = c.y - 20.f * s;

    setRC(m_leftUpperArm,  c.x + 10.f*s + armSwing, armY);
    setRC(m_leftForearm,   c.x + 11.f*s + armSwing, armY + 10.f*s);
    setRC(m_rightUpperArm, c.x -  7.f*s - armSwing, armY);
    setRC(m_rightForearm,  c.x -  8.f*s - armSwing, armY + 10.f*s);
    m_gun.setPosition(snap(c.x - 21.f*s), snap(armY + 14.f*s));

    float lgt = c.y + 2.f*s;
    m_leftThigh.setPosition( snap(c.x - 7.f*s + stride),  snap(lgt));
    m_leftShin.setPosition(  snap(c.x - 6.f*s + stride),  snap(lgt + 11.f*s));
    m_leftBoot.setPosition(  snap(c.x - 7.f*s + stride),  snap(lgt + 21.f*s));
    m_rightThigh.setPosition(snap(c.x + 1.f*s - stride),  snap(lgt));
    m_rightShin.setPosition( snap(c.x + 1.f*s - stride),  snap(lgt + 11.f*s));
    m_rightBoot.setPosition( snap(c.x          - stride),  snap(lgt + 21.f*s));

    float barW = m_hpBarBg.getSize().x;
    m_hpBarBg.setPosition(snap(c.x - barW * 0.5f), snap(headTop - 10.f));
    float ratio = (float)m_health / (float)m_maxHealth;
    m_hpBar.setSize({barW * ratio, 5.f});
    m_hpBar.setPosition(snap(c.x - barW * 0.5f), snap(headTop - 10.f));
}

// ─────────────────────────────────────────────────────────────────────────────
void Enemy::shoot(const sf::Vector2f& playerPos)
{
    // Direction vector from enemy gun tip toward player
    sf::Vector2f gunTip = {
        m_gun.getPosition().x,
        m_gun.getPosition().y + m_gun.getSize().y * 0.5f
    };

    sf::Vector2f dir = playerPos - gunTip;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.f) return;
    dir /= len;   // normalise

    EBullet b;
    float bw = 10.f * m_scale, bh = 3.f * m_scale;
    b.shape.setSize({bw, bh});
    b.shape.setOrigin(0.f, bh * 0.5f);
    // Enemy bullet colour: red-orange for normals, electric blue for fast, dark red for boss
    if (m_type == EnemyType::Fast)
        b.shape.setFillColor(sf::Color(100, 180, 255));
    else if (m_type == EnemyType::Boss)
        b.shape.setFillColor(sf::Color(255, 50, 50));
    else
        b.shape.setFillColor(sf::Color(255, 120, 40));

    b.shape.setPosition(gunTip);
    b.velocity = dir * m_bulletSpeed;
    m_bullets.push_back(b);
}

// ─────────────────────────────────────────────────────────────────────────────
void Enemy::update(float dt, const sf::Vector2f& playerPos,
                   const std::vector<sf::RectangleShape>& platforms)
{
    if (!m_alive) {
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
            [](const EParticle& p){ return p.lifetime <= 0.f; }), m_particles.end());
        // Also update any in-flight bullets even after death
        for (auto& b : m_bullets) b.shape.move(b.velocity * dt);
        return;
    }

    if (m_hitFlashTime > 0.f) m_hitFlashTime -= dt;

    // ── Fire cooldown ─────────────────────────────────────────────────────────
    if (m_fireCooldown > 0.f) {
        m_fireCooldown -= dt;
    } else {
        // Only fire if player is within range (800px)
        float dx = playerPos.x - m_centerPosition.x;
        float dy = playerPos.y - m_centerPosition.y;
        float dist2 = dx * dx + dy * dy;
        if (dist2 < 800.f * 800.f) {
            shoot(playerPos);
            m_fireCooldown = m_fireInterval;
        }
    }

    // ── Update in-flight bullets ──────────────────────────────────────────────
    for (auto& b : m_bullets)
        b.shape.move(b.velocity * dt);

    // Remove bullets that fly far off screen
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [](const EBullet& b){
            sf::Vector2f p = b.shape.getPosition();
            return p.x < -200.f || p.x > 5000.f || p.y < -200.f || p.y > 1300.f;
        }), m_bullets.end());

    // ── Movement ──────────────────────────────────────────────────────────────
    float s = m_scale;
    const float bW = 12.f * s, bH = 20.f * s;

    m_velocity.y += m_gravity * dt;
    m_animTime   += dt * 7.f;

    sf::Vector2f pos = m_centerPosition;
    float dir = (playerPos.x > pos.x) ? 1.f : -1.f;
    pos.x += dir * m_speed * dt;

    sf::FloatRect bb{ pos.x - bW * 0.5f, pos.y - bH * 0.5f, bW, bH };
    for (const auto& p : platforms) {
        if (bb.intersects(p.getGlobalBounds())) {
            if (dir > 0.f) pos.x = p.getGlobalBounds().left - bW;
            else           pos.x = p.getGlobalBounds().left + p.getGlobalBounds().width + bW;
            bb.left = pos.x - bW * 0.5f;
        }
    }

    pos.y += m_velocity.y * dt;
    bb.top = pos.y - bH * 0.5f;
    for (const auto& p : platforms) {
        sf::FloatRect pb = p.getGlobalBounds();
        if (bb.intersects(pb)) {
            if (m_velocity.y > 0.f) { pos.y = pb.top - 22.f * s; m_velocity.y = 0.f; }
            else                    { pos.y = pb.top + pb.height + bH * 0.5f; m_velocity.y = 0.f; }
            bb.top = pos.y - bH * 0.5f;
        }
    }

    updateHumanoidPosition(pos, m_animTime);
}

// ─────────────────────────────────────────────────────────────────────────────
void Enemy::draw(sf::RenderWindow& window) const
{
    for (const auto& p : m_particles) window.draw(p.shape);
    for (const auto& b : m_bullets)  window.draw(b.shape);

    if (!m_alive) return;

    window.draw(m_rightThigh); window.draw(m_rightShin); window.draw(m_rightBoot);
    window.draw(m_leftUpperArm); window.draw(m_leftForearm);
    window.draw(m_torso); window.draw(m_belt); window.draw(m_neck);
    window.draw(m_head);  window.draw(m_helmet);
    window.draw(m_leftThigh); window.draw(m_leftShin); window.draw(m_leftBoot);
    window.draw(m_rightUpperArm); window.draw(m_rightForearm);
    window.draw(m_gun);

    if (m_type != EnemyType::Fast) {
        window.draw(m_hpBarBg);
        window.draw(m_hpBar);
    }

    if (m_hitFlashTime > 0.f) {
        float s = m_scale;
        sf::RectangleShape ov({30.f * s, 60.f * s});
        ov.setFillColor(sf::Color(255, 255, 255, 150));
        ov.setPosition(m_centerPosition.x - 15.f * s, m_centerPosition.y - 48.f * s);
        window.draw(ov);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Enemy::spawnDeathParticles()
{
    static const sf::Color boss_cols[] = {{255,50,50},{255,180,50},{200,200,200},{255,255,255}};
    static const sf::Color norm_cols[] = {{255,100,50},{200,50,50},{255,220,80},{180,180,180}};
    const sf::Color* cols = (m_type == EnemyType::Boss) ? boss_cols : norm_cols;
    int count = (m_type == EnemyType::Boss) ? 24 : 10;
    for (int i = 0; i < count; ++i) {
        EParticle p;
        float angle = (i / (float)count) * 6.2832f;
        float speed = 90.f + (std::rand() % 150);
        p.velocity  = { std::cos(angle) * speed, std::sin(angle) * speed - 80.f };
        float sz    = 3.f + (std::rand() % 5);
        p.shape.setSize({sz, sz});
        p.shape.setFillColor(cols[std::rand() % 4]);
        p.shape.setPosition(m_centerPosition);
        p.maxLifetime = p.lifetime = 0.5f + (std::rand() % 60) * 0.01f;
        m_particles.push_back(p);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
sf::FloatRect Enemy::getBounds()     const { return m_torso.getGlobalBounds(); }
bool          Enemy::isAlive()       const { return m_alive; }
EnemyType     Enemy::getType()       const { return m_type; }
int           Enemy::getMaxHealth()  const { return m_maxHealth; }
int           Enemy::getHealth()     const { return m_health; }

const std::vector<EBullet>& Enemy::getBullets() const { return m_bullets; }
std::vector<EBullet>&       Enemy::getBullets()       { return m_bullets; }

void Enemy::takeHit()
{
    if (!m_alive) return;
    m_hitFlashTime = 0.15f;
    if (--m_health <= 0) {
        m_alive = false;
        spawnDeathParticles();
    }
}