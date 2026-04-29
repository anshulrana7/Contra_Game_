#include "Level.h"
#include <algorithm>
#include <string>
#include <vector>

// ── helpers ───────────────────────────────────────────────────────────────────
static sf::RectangleShape makePlatform(float x, float y, float w, float h,
                                       sf::Color col = sf::Color(90, 90, 100))
{
    sf::RectangleShape r({w, h});
    r.setPosition(x, y);
    r.setFillColor(col);
    r.setOutlineColor(sf::Color(
        (sf::Uint8)std::min(255, (int)col.r + 30),
        (sf::Uint8)std::min(255, (int)col.g + 30),
        (sf::Uint8)std::min(255, (int)col.b + 30), 180));
    r.setOutlineThickness(-1.f);
    return r;
}

static Checkpoint makeCheckpoint(float x, float groundY)
{
    Checkpoint cp;
    cp.activated = false;
    cp.position  = {x, groundY};

    cp.pole.setSize({4.f, 50.f});
    cp.pole.setPosition(x - 2.f, groundY - 50.f);
    cp.pole.setFillColor(sf::Color(200, 200, 200));

    cp.flag.setPointCount(3);
    cp.flag.setPoint(0, {x + 2.f,  groundY - 50.f});
    cp.flag.setPoint(1, {x + 22.f, groundY - 40.f});
    cp.flag.setPoint(2, {x + 2.f,  groundY - 30.f});
    cp.flag.setFillColor(sf::Color(220, 30, 30));
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
Level::Level(int index)
    : m_index(index), m_worldWidth(2000.f), m_completed(false),
    m_respawnPoint(80.f, 650.f),
    m_killCount(0), m_nextCheckpointKill(3), m_nextRewardHealth(true)
{
    std::vector<std::string> backgroundCandidates;
    if (m_index >= 1 && m_index <= 5) {
        backgroundCandidates.push_back("assests/level" + std::to_string(m_index) + ".png");
        backgroundCandidates.push_back("assets/level" + std::to_string(m_index) + ".png");
        backgroundCandidates.push_back("level" + std::to_string(m_index) + ".png");
        backgroundCandidates.push_back("level" + std::to_string(m_index) + ".jpg");
    }
    if (m_index == 5) {
        backgroundCandidates.push_back("bossLevel.png");
    }

    for (const auto& path : backgroundCandidates) {
        if (m_backgroundTexture.loadFromFile(path)) {
            m_background.setTexture(m_backgroundTexture);
            break;
        }
    }

    initLevel();

    if (m_backgroundTexture.getSize().x > 0)
        m_background.setScale(m_worldWidth / m_backgroundTexture.getSize().x,
                              1000.f       / m_backgroundTexture.getSize().y);
}

// ─────────────────────────────────────────────────────────────────────────────
void Level::initLevel()
{
    m_platforms.clear();
    m_enemies.clear();
    m_checkpoints.clear();
    m_completed    = false;
    m_respawnPoint = {80.f, 650.f};
    m_killCount = 0;
    m_nextCheckpointKill = 3;
    m_nextRewardHealth = true;

    const float G  = 860.f;
    const float GH = 80.f;
    const float yOffset = 340.f;

    static const sf::Color groundCols[] = {
        {}, {70,90,60}, {60,70,90}, {90,75,50}, {50,50,70}, {80,20,20}
    };
    sf::Color gc = groundCols[m_index];

    if (m_index == 1)
    {
        m_worldWidth = 2000.f;
        m_platforms.push_back(makePlatform(0.f, G, m_worldWidth, GH, gc));
        m_platforms.push_back(makePlatform( 300.f, 430.f + yOffset, 180.f, 18.f, {110,130,90}));
        m_platforms.push_back(makePlatform( 600.f, 370.f + yOffset, 160.f, 18.f, {110,130,90}));
        m_platforms.push_back(makePlatform( 950.f, 410.f + yOffset, 200.f, 18.f, {110,130,90}));
        m_platforms.push_back(makePlatform(1300.f, 350.f + yOffset, 160.f, 18.f, {110,130,90}));
        m_platforms.push_back(makePlatform(1600.f, 390.f + yOffset, 180.f, 18.f, {110,130,90}));
        m_checkpoints.push_back(makeCheckpoint(900.f, G));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f( 500.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f( 850.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1050.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1200.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1500.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1420.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1750.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1880.f, G-48.f));
    }
    else if (m_index == 2)
    {
        m_worldWidth = 2400.f;
        m_platforms.push_back(makePlatform(0.f, G, m_worldWidth, GH, gc));
        m_platforms.push_back(makePlatform( 350.f, 440.f + yOffset, 200.f, 18.f, {90,100,130}));
        m_platforms.push_back(makePlatform( 720.f, 360.f + yOffset, 180.f, 18.f, {90,100,130}));
        m_platforms.push_back(makePlatform(1100.f, 400.f + yOffset, 160.f, 18.f, {90,100,130}));
        m_platforms.push_back(makePlatform(1450.f, 330.f + yOffset, 200.f, 18.f, {90,100,130}));
        m_platforms.push_back(makePlatform(1800.f, 370.f + yOffset, 180.f, 18.f, {90,100,130}));
        m_checkpoints.push_back(makeCheckpoint(1050.f, G));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f( 550.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f( 700.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f( 900.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1200.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1450.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1550.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1850.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1980.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2100.f, G-48.f));
    }
    else if (m_index == 3)
    {
        m_worldWidth = 2600.f;
        m_platforms.push_back(makePlatform(0.f, G, m_worldWidth, GH, gc));
        m_platforms.push_back(makePlatform( 400.f, 440.f + yOffset, 220.f, 18.f, {110,95,65}));
        m_platforms.push_back(makePlatform( 800.f, 370.f + yOffset, 200.f, 18.f, {110,95,65}));
        m_platforms.push_back(makePlatform(1250.f, 410.f + yOffset, 180.f, 18.f, {110,95,65}));
        m_platforms.push_back(makePlatform(1600.f, 340.f + yOffset, 220.f, 18.f, {110,95,65}));
        m_platforms.push_back(makePlatform(2000.f, 380.f + yOffset, 200.f, 18.f, {110,95,65}));
        m_platforms.push_back(makePlatform(2300.f, 310.f + yOffset, 180.f, 18.f, {110,95,65}));
        m_checkpoints.push_back(makeCheckpoint(1200.f, G));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f( 600.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f( 950.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1120.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1350.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1700.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1880.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2100.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(2400.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2520.f, G-48.f));
    }
    else if (m_index == 4)
    {
        m_worldWidth = 2800.f;
        m_platforms.push_back(makePlatform(0.f, G, m_worldWidth, GH, gc));
        m_platforms.push_back(makePlatform( 400.f, 450.f + yOffset, 200.f, 18.f, {70,70,100}));
        m_platforms.push_back(makePlatform( 800.f, 370.f + yOffset, 190.f, 18.f, {70,70,100}));
        m_platforms.push_back(makePlatform(1200.f, 410.f + yOffset, 210.f, 18.f, {70,70,100}));
        m_platforms.push_back(makePlatform(1650.f, 340.f + yOffset, 200.f, 18.f, {70,70,100}));
        m_platforms.push_back(makePlatform(2050.f, 380.f + yOffset, 190.f, 18.f, {70,70,100}));
        m_platforms.push_back(makePlatform(2400.f, 310.f + yOffset, 220.f, 18.f, {70,70,100}));
        m_checkpoints.push_back(makeCheckpoint(1150.f, G));
        m_checkpoints.push_back(makeCheckpoint(2000.f, G));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f( 600.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1000.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1200.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1400.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1800.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1980.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(2200.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2500.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(2700.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2620.f, G-48.f));
    }
    else if (m_index == 5)
    {
        m_worldWidth = 3200.f;
        m_platforms.push_back(makePlatform(0.f, G, m_worldWidth, GH, gc));
        m_platforms.push_back(makePlatform( 500.f, 440.f + yOffset, 240.f, 18.f, {100,35,35}));
        m_platforms.push_back(makePlatform(1000.f, 370.f + yOffset, 220.f, 18.f, {100,35,35}));
        m_platforms.push_back(makePlatform(1500.f, 410.f + yOffset, 240.f, 18.f, {100,35,35}));
        m_platforms.push_back(makePlatform(2000.f, 340.f + yOffset, 220.f, 18.f, {100,35,35}));
        m_platforms.push_back(makePlatform(2500.f, 380.f + yOffset, 200.f, 18.f, {100,35,35}));
        m_checkpoints.push_back(makeCheckpoint(1450.f, G));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f( 700.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(1100.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1350.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(1600.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2000.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(2200.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Normal, sf::Vector2f(2400.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Boss,   sf::Vector2f(2800.f, G-130.f));
        m_enemies.emplace_back(EnemyType::Boss,   sf::Vector2f(3000.f, G-130.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(2660.f, G-48.f));
        m_enemies.emplace_back(EnemyType::Fast,   sf::Vector2f(3120.f, G-48.f));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Level::reset(Player& player)
{
    initLevel();
    m_respawnPoint = {80.f, 650.f};
    player.reset(m_respawnPoint);
    player.resetSpread();
}

void Level::update(float dt, Player& player)
{
    player.update(dt, m_platforms, m_worldWidth);

    for (auto& e : m_enemies)
        e.update(dt, player.getPosition(), m_platforms);

    if (player.getBounds().top > 1300.f)
        respawnPlayer(player);

    handleCheckpoints(player);
    handlePlayerCollisions(player);
    handlePlayerBullets(player);
    handleEnemyBullets(player);        // ← process enemy bullets hitting player

    m_completed = std::all_of(m_enemies.begin(), m_enemies.end(),
        [](const Enemy& e){ return !e.isAlive(); });
}

void Level::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    for (const auto& p : m_platforms)   window.draw(p);
    for (const auto& cp : m_checkpoints){ window.draw(cp.pole); window.draw(cp.flag); }
    for (const auto& e : m_enemies)      e.draw(window);
}

bool  Level::isCompleted()   const { return m_completed; }
int   Level::getIndex()      const { return m_index; }
float Level::getWorldWidth() const { return m_worldWidth; }

// ─────────────────────────────────────────────────────────────────────────────
void Level::handleCheckpoints(Player& player)
{
    sf::FloatRect pb = player.getBounds();
    for (auto& cp : m_checkpoints) {
        if (cp.activated) continue;
        sf::FloatRect zone{ cp.position.x - 15.f, cp.position.y - 60.f, 30.f, 60.f };
        if (pb.intersects(zone)) {
            cp.activated = true;
            cp.flag.setFillColor(sf::Color(50, 220, 50));
            m_respawnPoint = { cp.position.x, cp.position.y - 50.f };
            player.upgradeSpread();
        }
    }
}

void Level::grantKillCheckpointReward(Player& player)
{
    float respawnX = std::max(80.f, std::min(player.getPosition().x, m_worldWidth - 80.f));
    m_respawnPoint.x = respawnX;

    if (m_nextRewardHealth) {
        player.restoreFullHealth();
    } else {
        player.upgradeSpread();
    }

    m_nextRewardHealth = !m_nextRewardHealth;
    m_nextCheckpointKill += 3;
}

// ── Body collision: instant HP drain → respawn ────────────────────────────────
void Level::handlePlayerCollisions(Player& player)
{
    if (player.isInvincible()) return;
    sf::FloatRect pb = player.getBounds();
    for (auto& e : m_enemies) {
        if (!e.isAlive()) continue;
        if (pb.intersects(e.getBounds())) {
            player.takeBodyHit();
            respawnPlayer(player);
            return;
        }
    }
}

// ── Enemy bullets hit player: -1 HP each ─────────────────────────────────────
void Level::handleEnemyBullets(Player& player)
{
    sf::FloatRect pb = player.getBounds();

    for (auto& e : m_enemies) {
        auto& bullets = e.getBullets();
        for (auto& b : bullets) {
            if (b.shape.getGlobalBounds().intersects(pb)) {
                player.takeBulletHit();
                // Teleport bullet off-screen so it's cleaned up
                b.shape.setPosition(-2000.f, -2000.f);

                // If HP hit 0, lose a life and respawn
                if (player.isDead()) {
                    respawnPlayer(player);
                    return;  // pb is stale after reset; bail out
                }
            }
        }
        // Clean up spent bullets
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const EBullet& b){ return b.shape.getPosition().x < -1000.f; }),
            bullets.end());
    }
}

// ── Player bullets hitting enemies ───────────────────────────────────────────
void Level::handlePlayerBullets(Player& player)
{
    auto& bullets = player.getBullets();
    for (auto& e : m_enemies) {
        if (!e.isAlive()) continue;
        for (auto& b : bullets) {
            if (b.shape.getGlobalBounds().intersects(e.getBounds())) {
                bool wasAlive = e.isAlive();
                e.takeHit();
                int gain = (e.getType() == EnemyType::Boss) ? 500 :
                           (e.getType() == EnemyType::Fast) ? 150 : 100;
                if (!e.isAlive()) gain *= 2;
                player.addScore(gain);

                if (wasAlive && !e.isAlive()) {
                    ++m_killCount;
                    if (m_killCount >= m_nextCheckpointKill)
                        grantKillCheckpointReward(player);
                }

                b.shape.setPosition(-2000.f, -2000.f);
                break;
            }
        }
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b){ return b.shape.getPosition().x < -1000.f; }),
        bullets.end());
}

// ─────────────────────────────────────────────────────────────────────────────
void Level::respawnPlayer(Player& player)
{
    player.loseLife();
    player.reset(m_respawnPoint);   // reset() also restores HP to 10
    player.setInvincible(1.8f);
}