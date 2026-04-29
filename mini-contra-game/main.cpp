#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

#include "Level.h"
#include "Player.h"

enum class GameState {
    MainMenu,
    MenuName,
    MenuAge,
    MenuMode,
    OptionsMenu,
    LeaderboardsMenu,
    Playing,
    PauseMenu,
    LevelTransition,
    GameOver,
    Victory
};

enum class GameMode { Story, QuickPlay };

// ── Celebration particle ──────────────────────────────────────────────────────
struct CelebParticle {
    sf::RectangleShape shape;
    sf::Vector2f vel;
    float life, maxLife, rotSpeed;
};

static void spawnCelebration(std::vector<CelebParticle>& parts, float cx, float cy)
{
    static const sf::Color cols[] = {
        {255,80,80},{255,200,50},{80,255,80},{80,180,255},{255,80,255},{255,255,255}
    };
    for (int i = 0; i < 80; ++i) {
        CelebParticle p;
        float angle = (float)i / 80.f * 6.2832f;
        float speed = 60.f + (std::rand() % 260);
        p.vel = { std::cos(angle)*speed, std::sin(angle)*speed - 150.f };
        float sz = 4.f + (std::rand() % 8);
        p.shape.setSize({sz, sz*0.5f});
        p.shape.setFillColor(cols[std::rand() % 6]);
        p.shape.setOrigin(sz*0.5f, sz*0.25f);
        p.shape.setPosition(cx, cy);
        p.maxLife = p.life = 1.2f + (std::rand() % 80) * 0.01f;
        p.rotSpeed = -360.f + (std::rand() % 720);
        parts.push_back(p);
    }
}

static void updateCelebration(std::vector<CelebParticle>& parts, float dt)
{
    for (auto& p : parts) {
        p.vel.y += 400.f * dt;
        p.shape.move(p.vel * dt);
        p.shape.rotate(p.rotSpeed * dt);
        p.life -= dt;
        sf::Color c = p.shape.getFillColor();
        c.a = (sf::Uint8)(std::max(0.f, p.life / p.maxLife) * 255);
        p.shape.setFillColor(c);
    }
    parts.erase(std::remove_if(parts.begin(), parts.end(),
        [](const CelebParticle& p){ return p.life <= 0.f; }), parts.end());
}

// ── HP bar ────────────────────────────────────────────────────────────────────
static void drawHPBar(sf::RenderWindow& w, int hp, int maxHp, float x, float y)
{
    const float barW = 160.f, barH = 14.f;

    // Background
    sf::RectangleShape bg({barW + 2.f, barH + 2.f});
    bg.setPosition(x - 1.f, y - 1.f);
    bg.setFillColor(sf::Color(0, 0, 0, 180));
    w.draw(bg);

    // Fill — colour shifts green→yellow→red with HP
    float ratio = (float)hp / (float)maxHp;
    sf::Uint8 r = (sf::Uint8)(255 * (1.f - ratio));
    sf::Uint8 g = (sf::Uint8)(255 * ratio);
    sf::RectangleShape fill({barW * ratio, barH});
    fill.setPosition(x, y);
    fill.setFillColor(sf::Color(r, g, 30));
    w.draw(fill);

    // Pip markers every 1 HP
    for (int i = 1; i < maxHp; ++i) {
        sf::RectangleShape pip({1.f, barH});
        pip.setFillColor(sf::Color(0, 0, 0, 120));
        pip.setPosition(x + barW * ((float)i / maxHp), y);
        w.draw(pip);
    }

    // Border
    sf::RectangleShape border({barW, barH});
    border.setPosition(x, y);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(200, 200, 200, 160));
    border.setOutlineThickness(1.f);
    w.draw(border);
}

// ── Lives hearts ──────────────────────────────────────────────────────────────
static void drawHeart(sf::RenderWindow& w, float x, float y, bool filled)
{
    sf::Color c = filled ? sf::Color(220, 50, 50) : sf::Color(80, 80, 80);
    sf::CircleShape cl(6.f), cr(6.f);
    cl.setFillColor(c); cr.setFillColor(c);
    cl.setPosition(x, y);  cr.setPosition(x+7.f, y);
    w.draw(cl); w.draw(cr);
    sf::ConvexShape tri; tri.setPointCount(3); tri.setFillColor(c);
    tri.setPoint(0, {x-1.f, y+7.f});
    tri.setPoint(1, {x+19.f, y+7.f});
    tri.setPoint(2, {x+9.f, y+19.f});
    w.draw(tri);
}

// ── Spread icons ──────────────────────────────────────────────────────────────
static void drawSpreadIcons(sf::RenderWindow& w, int level, float x, float y)
{
    for (int i = 0; i < 4; ++i) {
        sf::RectangleShape r({9.f, 6.f});
        r.setFillColor(i < level ? sf::Color(255,230,50) : sf::Color(60,60,60));
        r.setPosition(x + i * 12.f, y);
        w.draw(r);
    }
}

// ── Drop-shadow text draw ─────────────────────────────────────────────────────
static void drawShadowText(sf::Text& t, sf::RenderWindow& w)
{
    sf::Color orig = t.getFillColor();
    sf::Vector2f pos = t.getPosition();
    t.setFillColor(sf::Color(0,0,0,160));
    t.setPosition(pos.x+3.f, pos.y+3.f);
    w.draw(t);
    t.setFillColor(orig);
    t.setPosition(pos);
    w.draw(t);
}

static void centerText(sf::Text& text, float x, float y)
{
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    text.setPosition(x, y);
}

// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    unsigned W = 1920, H = 1000;

    sf::Texture mainMenuTexture;
    bool mainMenuBackgroundLoaded = mainMenuTexture.loadFromFile("mainWindow.png");

    sf::RenderWindow window(sf::VideoMode(W, H), "Contra - Enhanced Edition");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded =
        font.loadFromFile("VeniteAdoremus-rgRBA.ttf") ||
        font.loadFromFile("VeniteAdoremusStraight-Yzo6v.ttf") ||
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") ||
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf") ||
        font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");

    // ── Levels ────────────────────────────────────────────────────────────────
    Player player;
    Level level1(1), level2(2), level3(3), level4(4), level5(5);
    Level* levels[6] = { nullptr, &level1, &level2, &level3, &level4, &level5 };
    Level* currentLevel   = &level1;
    int    currentLevelIndex = 1;

    sf::Sprite mainMenuBackground;
    if (mainMenuBackgroundLoaded) {
        mainMenuBackground.setTexture(mainMenuTexture);
        mainMenuBackground.setScale(static_cast<float>(W) / mainMenuTexture.getSize().x,
                                    static_cast<float>(H) / mainMenuTexture.getSize().y);
    }

    std::srand((unsigned)std::time(nullptr));

    std::string playerName;
    std::string ageInput;
    int playerAge = 0;
    int modeCursor = 0;

    GameMode gameMode = GameMode::Story;
    GameState state = GameState::MainMenu;

    sf::View gameView(sf::FloatRect(0.f, 0.f, (float)W, (float)H));

    // ── Screen flash ──────────────────────────────────────────────────────────
    float      screenFlashTimer = 0.f;
    sf::Color  screenFlashCol   = sf::Color::White;

    // ── Transition ────────────────────────────────────────────────────────────
    float transTimer    = 0.f;
    const float TRANS_DUR = 3.2f;
    int   pendingLevel  = 0;

    // ── Victory ───────────────────────────────────────────────────────────────
    float victoryTimer = 0.f;
    std::vector<CelebParticle> celebParts;

    // ── UI texts ──────────────────────────────────────────────────────────────
    sf::Text scoreText, levelText, gameOverText, victoryText,
             levelUpText, subText, pressEnterText, hpLabel,
             menuTitleText, menuPromptText, menuInputText, menuHintText,
             mainMenuTitleText, mainMenuItemText, mainMenuHintText,
             optionsTitleText, optionsBodyText,
             leaderboardTitleText, leaderboardBodyText,
             pauseTitleText, pauseItemText, pauseHintText;

    auto mkText = [&](sf::Text& t, unsigned sz, sf::Color col) {
        if (fontLoaded) t.setFont(font);
        t.setCharacterSize(sz);
        t.setFillColor(col);
    };
    mkText(scoreText,      20, sf::Color::White);
    mkText(levelText,      20, sf::Color(255,220,80));
    mkText(gameOverText,   56, sf::Color(220,40,40));
    mkText(victoryText,    64, sf::Color(255,220,50));
    mkText(levelUpText,    72, sf::Color(80,255,100));
    mkText(subText,        22, sf::Color(200,200,200));
    mkText(pressEnterText, 18, sf::Color(180,180,180));
    mkText(hpLabel,        13, sf::Color(200,200,200));
    mkText(menuTitleText,  44, sf::Color(255,220,80));
    mkText(menuPromptText, 26, sf::Color::White);
    mkText(menuInputText,  30, sf::Color(80,255,120));
    mkText(menuHintText,   18, sf::Color(200,200,200));
    mkText(mainMenuTitleText, 54, sf::Color::White);
    mkText(mainMenuItemText,  30, sf::Color(255,220,80));
    mkText(mainMenuHintText,  18, sf::Color(230,230,230));
    mkText(optionsTitleText,   46, sf::Color::White);
    mkText(optionsBodyText,    24, sf::Color(230,230,230));
    mkText(leaderboardTitleText, 46, sf::Color::White);
    mkText(leaderboardBodyText,  24, sf::Color(230,230,230));
    mkText(pauseTitleText, 54, sf::Color::White);
    mkText(pauseItemText,  30, sf::Color(255,220,80));
    mkText(pauseHintText,  18, sf::Color(220,220,220));
    hpLabel.setString("HP");

    sf::RectangleShape hudBg({(float)W, 46.f});
    hudBg.setFillColor(sf::Color(0,0,0,150));

    sf::RectangleShape overlay({(float)W, (float)H});

    const std::vector<std::string> mainMenuItems = {
        "Start Game",
        "Options",
        "Leaderboards",
        "Quit"
    };
    int mainMenuCursor = 0;

    const std::vector<std::string> leaderboardLines = {
        "1. No saved scores yet",
        "2. Start a game to create a run",
        "3. Best score can be stored later"
    };

    const std::vector<std::string> pauseItems = {
        "Resume",
        "Restart",
        "Quit to Main Menu"
    };
    int pauseCursor = 0;

    sf::Clock clock;
    sf::Clock blinkClock;

    auto chooseDifferentQuickLevel = [&](int avoid) {
        int picked = 1 + (std::rand() % 5);
        if (picked == avoid)
            picked = (picked % 5) + 1;
        return picked;
    };

    auto startRunForMode = [&]() {
        player.restoreAllLives();
        player.resetScore();
        celebParts.clear();
        victoryTimer = 0.f;

        if (gameMode == GameMode::Story)
            currentLevelIndex = 1;
        else
            currentLevelIndex = chooseDifferentQuickLevel(0);

        currentLevel = levels[currentLevelIndex];
        currentLevel->reset(player);
        state = GameState::Playing;
    };

    while (window.isOpen())
    {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                window.close();
                continue;
            }

            if (ev.type == sf::Event::TextEntered) {
                if (state == GameState::MenuName) {
                    if (ev.text.unicode == 8) {
                        if (!playerName.empty()) playerName.pop_back();
                    } else if (ev.text.unicode >= 32 && ev.text.unicode < 127 && playerName.size() < 16) {
                        playerName.push_back((char)ev.text.unicode);
                    }
                } else if (state == GameState::MenuAge) {
                    if (ev.text.unicode == 8) {
                        if (!ageInput.empty()) ageInput.pop_back();
                    } else if (ev.text.unicode >= '0' && ev.text.unicode <= '9' && ageInput.size() < 3) {
                        ageInput.push_back((char)ev.text.unicode);
                    }
                }
            }

            if (ev.type == sf::Event::KeyPressed) {
                if (state == GameState::Playing) {
                    if (ev.key.code == sf::Keyboard::Escape) {
                        pauseCursor = 0;
                        state = GameState::PauseMenu;
                    }
                } else if (state == GameState::PauseMenu) {
                    if (ev.key.code == sf::Keyboard::Up || ev.key.code == sf::Keyboard::W)
                        pauseCursor = (pauseCursor + static_cast<int>(pauseItems.size()) - 1) % static_cast<int>(pauseItems.size());
                    else if (ev.key.code == sf::Keyboard::Down || ev.key.code == sf::Keyboard::S)
                        pauseCursor = (pauseCursor + 1) % static_cast<int>(pauseItems.size());
                    else if (ev.key.code == sf::Keyboard::Escape) {
                        state = GameState::Playing;
                    } else if (ev.key.code == sf::Keyboard::Enter) {
                        if (pauseCursor == 0) {
                            state = GameState::Playing;
                        } else if (pauseCursor == 1) {
                            startRunForMode();
                        } else if (pauseCursor == 2) {
                            state = GameState::MainMenu;
                        }
                    }
                } else if (state == GameState::MainMenu) {
                    if (ev.key.code == sf::Keyboard::Up || ev.key.code == sf::Keyboard::W)
                        mainMenuCursor = (mainMenuCursor + static_cast<int>(mainMenuItems.size()) - 1) % static_cast<int>(mainMenuItems.size());
                    else if (ev.key.code == sf::Keyboard::Down || ev.key.code == sf::Keyboard::S)
                        mainMenuCursor = (mainMenuCursor + 1) % static_cast<int>(mainMenuItems.size());
                    else if (ev.key.code == sf::Keyboard::Enter) {
                        if (mainMenuCursor == 0) {
                            playerName.clear();
                            ageInput.clear();
                            modeCursor = 0;
                            state = GameState::MenuName;
                        } else if (mainMenuCursor == 1) {
                            state = GameState::OptionsMenu;
                        } else if (mainMenuCursor == 2) {
                            state = GameState::LeaderboardsMenu;
                        } else if (mainMenuCursor == 3) {
                            window.close();
                        }
                    }
                } else if (state == GameState::OptionsMenu || state == GameState::LeaderboardsMenu) {
                    if (ev.key.code == sf::Keyboard::Escape || ev.key.code == sf::Keyboard::Enter)
                        state = GameState::MainMenu;
                } else if (state == GameState::MenuName) {
                    if (ev.key.code == sf::Keyboard::Enter && !playerName.empty())
                        state = GameState::MenuAge;
                } else if (state == GameState::MenuAge) {
                    if (ev.key.code == sf::Keyboard::Escape)
                        state = GameState::MenuName;

                    if (ev.key.code == sf::Keyboard::Enter && !ageInput.empty()) {
                        int age = std::stoi(ageInput);
                        if (age >= 1 && age <= 120) {
                            playerAge = age;
                            state = GameState::MenuMode;
                        }
                    }
                } else if (state == GameState::MenuMode) {
                    if (ev.key.code == sf::Keyboard::Up || ev.key.code == sf::Keyboard::W)
                        modeCursor = (modeCursor + 1) % 2;

                    if (ev.key.code == sf::Keyboard::Down || ev.key.code == sf::Keyboard::S)
                        modeCursor = (modeCursor + 1) % 2;

                    if (ev.key.code == sf::Keyboard::Escape)
                        state = GameState::MenuAge;

                    if (ev.key.code == sf::Keyboard::Enter) {
                        gameMode = (modeCursor == 0) ? GameMode::Story : GameMode::QuickPlay;
                        startRunForMode();
                    }
                }
            }
        }

        float dt = std::min(clock.restart().asSeconds(), 0.05f);
        if (screenFlashTimer > 0.f) screenFlashTimer -= dt;

        // ── State machine ─────────────────────────────────────────────────────
        if (state == GameState::Playing)
        {
            player.handleInput();
            currentLevel->update(dt, player);

            if (player.getLives() <= 0)
            {
                state = GameState::GameOver;
                screenFlashTimer = 0.5f;
                screenFlashCol   = sf::Color(200,0,0,180);
            }
            else if (currentLevel->isCompleted())
            {
                if (gameMode == GameMode::Story) {
                    if (currentLevelIndex < 5) {
                        pendingLevel     = currentLevelIndex + 1;
                        state            = GameState::LevelTransition;
                        transTimer       = TRANS_DUR;
                        screenFlashTimer = 0.3f;
                        screenFlashCol   = sf::Color(255,255,255,200);
                        spawnCelebration(celebParts, W*0.5f, H*0.4f);
                    } else {
                        state        = GameState::Victory;
                        victoryTimer = 0.f;
                        spawnCelebration(celebParts, W*0.5f, H*0.35f);
                        spawnCelebration(celebParts, W*0.25f, H*0.5f);
                        spawnCelebration(celebParts, W*0.75f, H*0.5f);
                    }
                } else {
                    pendingLevel     = chooseDifferentQuickLevel(currentLevelIndex);
                    state            = GameState::LevelTransition;
                    transTimer       = TRANS_DUR;
                    screenFlashTimer = 0.3f;
                    screenFlashCol   = sf::Color(255,255,255,200);
                    spawnCelebration(celebParts, W*0.5f, H*0.4f);
                }
            }
        }
        else if (state == GameState::LevelTransition)
        {
            transTimer -= dt;
            updateCelebration(celebParts, dt);
            if (transTimer <= 0.f) {
                currentLevelIndex = pendingLevel;
                currentLevel      = levels[currentLevelIndex];
                currentLevel->reset(player);
                celebParts.clear();
                state = GameState::Playing;
            }
        }
        else if (state == GameState::Victory)
        {
            victoryTimer += dt;
            updateCelebration(celebParts, dt);
            if (victoryTimer < 6.f && std::fmod(victoryTimer, 0.6f) < dt)
                spawnCelebration(celebParts, 100.f + (float)(std::rand()%600), 80.f + (float)(std::rand()%300));

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                modeCursor = 0;
                state = GameState::MainMenu;
            }
        }
        else if (state == GameState::GameOver)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                modeCursor = (gameMode == GameMode::Story) ? 0 : 1;
                state = GameState::MainMenu;
            }
        }

        // ── Camera ────────────────────────────────────────────────────────────
        float worldW = currentLevel->getWorldWidth();
        sf::Vector2f pPos = player.getPosition();
        float halfW  = gameView.getSize().x * 0.5f;
        float camX   = std::max(halfW, std::min(pPos.x + 16.f, worldW - halfW));
        gameView.setCenter(std::round(camX), std::round((float)H * 0.5f));

        // ── Render ────────────────────────────────────────────────────────────
        window.clear(sf::Color(20, 20, 30));
        window.setView(gameView);

        currentLevel->draw(window);
        if (state == GameState::Playing || state == GameState::LevelTransition || state == GameState::PauseMenu)
            player.draw(window);

        window.setView(window.getDefaultView());

        if (state == GameState::MainMenu)
        {
            if (mainMenuBackgroundLoaded) {
                window.draw(mainMenuBackground);
            }

            overlay.setFillColor(sf::Color(0, 0, 0, 120));
            window.draw(overlay);

            for (std::size_t i = 0; i < mainMenuItems.size(); ++i) {
                mainMenuItemText.setString((i == static_cast<std::size_t>(mainMenuCursor) ? "> " : "  ") + mainMenuItems[i]);
                centerText(mainMenuItemText, W * 0.5f, H - 320.f + static_cast<float>(i) * 56.f);
                mainMenuItemText.setFillColor(i == static_cast<std::size_t>(mainMenuCursor) ? sf::Color(255, 230, 100) : sf::Color::White);
                window.draw(mainMenuItemText);
            }

            mainMenuHintText.setString("Use W/S or Up/Down and press Enter");
            centerText(mainMenuHintText, W * 0.5f, H - 70.f);
            window.draw(mainMenuHintText);
        }
        else if (state == GameState::OptionsMenu)
        {
            overlay.setFillColor(sf::Color(0, 0, 0, 190));
            window.draw(overlay);

            optionsTitleText.setString("Options");
            centerText(optionsTitleText, W * 0.5f, 100.f);
            drawShadowText(optionsTitleText, window);

            optionsBodyText.setString(
                "Controls:\n"
                "A/D or Left/Right - Move\n"
                "W/Up/Space - Jump\n"
                "J - Shoot\n\n"
                "Press Esc or Enter to return");
            centerText(optionsBodyText, W * 0.5f, 275.f);
            window.draw(optionsBodyText);
        }
        else if (state == GameState::LeaderboardsMenu)
        {
            overlay.setFillColor(sf::Color(0, 0, 0, 190));
            window.draw(overlay);

            leaderboardTitleText.setString("Leaderboards");
            centerText(leaderboardTitleText, W * 0.5f, 100.f);
            drawShadowText(leaderboardTitleText, window);

            std::ostringstream lb;
            lb << leaderboardLines[0] << '\n'
               << leaderboardLines[1] << '\n'
               << leaderboardLines[2] << '\n' << '\n'
               << "Press Esc or Enter to return";
            leaderboardBodyText.setString(lb.str());
            centerText(leaderboardBodyText, W * 0.5f, 270.f);
            window.draw(leaderboardBodyText);
        }

        // Screen flash
        if (screenFlashTimer > 0.f) {
            sf::RectangleShape flash({(float)W, (float)H});
            sf::Color fc = screenFlashCol;
            fc.a = (sf::Uint8)(screenFlashTimer / 0.5f * 255.f);
            flash.setFillColor(fc);
            window.draw(flash);
        }

        // ── HUD ───────────────────────────────────────────────────────────────
        if (state == GameState::Playing || state == GameState::LevelTransition || state == GameState::PauseMenu)
        {
            window.draw(hudBg);

            // Lives (hearts) on left
            for (int i = 0; i < 3; ++i)
                drawHeart(window, 10.f + i*26.f, 10.f, i < player.getLives());

            // HP bar next to lives
            hpLabel.setPosition(90.f, 16.f);
            window.draw(hpLabel);
            drawHPBar(window, player.getHealth(), player.getMaxHealth(), 110.f, 16.f);

            // Spread icons below HP bar
            drawSpreadIcons(window, player.getSpreadLevel(), 110.f, 34.f);

            // Score centred
            std::ostringstream ss;
            ss << "SCORE  " << player.getScore();
            scoreText.setString(ss.str());
            scoreText.setPosition(W*0.5f - scoreText.getLocalBounds().width*0.5f, 14.f);
            window.draw(scoreText);

            // Stage top-right
            std::ostringstream ls;
            ls << "STAGE " << currentLevelIndex;
            levelText.setString(ls.str());
            levelText.setPosition(W - levelText.getLocalBounds().width - 12.f, 14.f);
            window.draw(levelText);

            std::ostringstream ps;
            ps << playerName << " (" << playerAge << ")";
            subText.setCharacterSize(18);
            subText.setString(ps.str());
            subText.setOrigin(0.f, 0.f);
            subText.setPosition(10.f, 52.f);
            window.draw(subText);

            subText.setString((gameMode == GameMode::Story) ? "MODE: STORY" : "MODE: QUICK PLAY");
            subText.setPosition(W - subText.getLocalBounds().width - 12.f, 52.f);
            window.draw(subText);
            subText.setCharacterSize(22);
        }

        // ── Menu flow (name -> age -> mode) ─────────────────────────────────
        if (state == GameState::MenuName || state == GameState::MenuAge || state == GameState::MenuMode)
        {
            if (mainMenuBackgroundLoaded) {
                window.draw(mainMenuBackground);
            }

            overlay.setFillColor(sf::Color(0,0,0,185));
            window.draw(overlay);

            if (state == GameState::MenuName) {
                menuPromptText.setString("Enter your name");
                menuInputText.setString(playerName + "_");
                menuHintText.setString("Type your name and press ENTER");
            } else if (state == GameState::MenuAge) {
                menuPromptText.setString("Enter your age");
                menuInputText.setString(ageInput + "_");
                menuHintText.setString("Numbers only (1-120). ENTER to continue, ESC to go back");
            } else {
                menuPromptText.setString("Select game mode");
                menuInputText.setString((modeCursor == 0 ? "> " : "  ") + std::string("Story Mode"));
                menuHintText.setString("W/S or Up/Down to switch. ENTER to start. ESC to go back");
            }

            sf::FloatRect mp = menuPromptText.getLocalBounds();
            menuPromptText.setOrigin(mp.width*0.5f, mp.height*0.5f);
            menuPromptText.setPosition(W*0.5f, H - 300.f);
            window.draw(menuPromptText);

            sf::FloatRect mi = menuInputText.getLocalBounds();
            menuInputText.setOrigin(mi.width*0.5f, mi.height*0.5f);
            menuInputText.setPosition(W*0.5f, H - 235.f);
            window.draw(menuInputText);

            if (state == GameState::MenuMode) {
                subText.setCharacterSize(30);
                subText.setFillColor(sf::Color(80,200,255));
                subText.setString((modeCursor == 1 ? "> " : "  ") + std::string("Quick Play"));
                sf::FloatRect qm = subText.getLocalBounds();
                subText.setOrigin(qm.width*0.5f, qm.height*0.5f);
                subText.setPosition(W*0.5f, H - 175.f);
                window.draw(subText);
                subText.setCharacterSize(22);
                subText.setFillColor(sf::Color(200,200,200));
            }

            sf::FloatRect mh = menuHintText.getLocalBounds();
            menuHintText.setOrigin(mh.width*0.5f, mh.height*0.5f);
            menuHintText.setPosition(W*0.5f, H - 95.f);
            window.draw(menuHintText);

            if (state == GameState::MenuAge && !ageInput.empty()) {
                int previewAge = std::stoi(ageInput);
                if (previewAge < 1 || previewAge > 120) {
                    pressEnterText.setString("Age must be between 1 and 120");
                    sf::FloatRect pe = pressEnterText.getLocalBounds();
                    pressEnterText.setOrigin(pe.width*0.5f, pe.height*0.5f);
                    pressEnterText.setPosition(W*0.5f, H - 50.f);
                    pressEnterText.setFillColor(sf::Color(255,120,120));
                    window.draw(pressEnterText);
                    pressEnterText.setFillColor(sf::Color(180,180,180));
                }
            }
        }

        if (state == GameState::PauseMenu)
        {
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(overlay);

            pauseTitleText.setString("Paused");
            centerText(pauseTitleText, W * 0.5f, 190.f);
            drawShadowText(pauseTitleText, window);

            for (std::size_t i = 0; i < pauseItems.size(); ++i) {
                pauseItemText.setString((i == static_cast<std::size_t>(pauseCursor) ? "> " : "  ") + pauseItems[i]);
                centerText(pauseItemText, W * 0.5f, 320.f + static_cast<float>(i) * 54.f);
                pauseItemText.setFillColor(i == static_cast<std::size_t>(pauseCursor) ? sf::Color(255, 230, 100) : sf::Color::White);
                window.draw(pauseItemText);
            }

            pauseHintText.setString("Use W/S or Up/Down, Enter to select, Esc to resume");
            centerText(pauseHintText, W * 0.5f, 560.f);
            window.draw(pauseHintText);
        }

        // ── Level-up transition ───────────────────────────────────────────────
        if (state == GameState::LevelTransition)
        {
            for (const auto& p : celebParts) window.draw(p.shape);

            float progress = 1.f - (transTimer / TRANS_DUR);
            float slideY   = (progress < 0.15f) ? H*0.3f*(1.f - progress/0.15f) : 0.f;

            float hue  = std::fmod(transTimer * 2.f, 1.f);
            sf::Uint8 r = (sf::Uint8)(std::sin(hue*6.28f)         * 100 + 155);
            sf::Uint8 g = (sf::Uint8)(std::sin(hue*6.28f + 2.09f) * 100 + 155);
            sf::Uint8 b = (sf::Uint8)(std::sin(hue*6.28f + 4.19f) * 100 + 155);

            levelUpText.setString("LEVEL UP!");
            sf::FloatRect lb = levelUpText.getLocalBounds();
            levelUpText.setOrigin(lb.width*0.5f, lb.height*0.5f);
            levelUpText.setScale(1.f + 0.08f*std::sin(transTimer*8.f),
                                 1.f + 0.08f*std::sin(transTimer*8.f));
            levelUpText.setFillColor(sf::Color(r,g,b));
            levelUpText.setPosition(W*0.5f, H*0.38f + slideY);
            drawShadowText(levelUpText, window);

            std::ostringstream sub;
            if (gameMode == GameMode::QuickPlay)
                sub << "NEXT QUICK MAP: STAGE " << pendingLevel;
            else
                sub << "ENTERING STAGE " << pendingLevel;
            subText.setString(sub.str());
            sf::FloatRect slb = subText.getLocalBounds();
            subText.setOrigin(slb.width*0.5f, slb.height*0.5f);
            subText.setPosition(W*0.5f, H*0.52f + slideY);
            window.draw(subText);
        }

        // ── Game Over ─────────────────────────────────────────────────────────
        if (state == GameState::GameOver)
        {
            overlay.setFillColor(sf::Color(0,0,0,170));
            window.draw(overlay);

            gameOverText.setString("GAME OVER");
            
            sf::FloatRect gb = gameOverText.getLocalBounds();
            gameOverText.setOrigin(gb.width*0.5f, gb.height*0.5f);
            gameOverText.setPosition(W*0.5f, H*0.38f);
            drawShadowText(gameOverText, window);

            std::ostringstream ss;
            ss << "FINAL SCORE:  " << player.getScore();
            subText.setString(ss.str());
            sf::FloatRect slb = subText.getLocalBounds();
            subText.setOrigin(slb.width*0.5f, slb.height*0.5f);
            subText.setPosition(W*0.5f, H*0.52f);
            window.draw(subText);

            pressEnterText.setString("Press ENTER for mode select");
            sf::FloatRect plb = pressEnterText.getLocalBounds();
            pressEnterText.setOrigin(plb.width*0.5f, plb.height*0.5f);
            float blink = std::sin(blinkClock.getElapsedTime().asSeconds() * 3.f);
            pressEnterText.setFillColor(sf::Color(255,255,255, blink > 0 ? 255 : 80));
            pressEnterText.setPosition(W*0.5f, H*0.65f);
            window.draw(pressEnterText);
            pressEnterText.setFillColor(sf::Color(180,180,180));
        }

        // ── Victory ───────────────────────────────────────────────────────────
        if (state == GameState::Victory)
        {
            for (const auto& p : celebParts) window.draw(p.shape);

            overlay.setFillColor(sf::Color(0,0,0,120));
            window.draw(overlay);

            float pulse = 1.f + 0.06f*std::sin(victoryTimer*5.f);
            float hue   = std::fmod(victoryTimer*0.5f, 1.f);
            sf::Uint8 vr = (sf::Uint8)(std::sin(hue*6.28f)         * 80 + 175);
            sf::Uint8 vg = (sf::Uint8)(std::sin(hue*6.28f + 2.09f) * 80 + 175);
            sf::Uint8 vb = (sf::Uint8)(std::sin(hue*6.28f + 4.19f) * 80 + 175);
            victoryText.setFillColor(sf::Color(vr,vg,vb));
            victoryText.setString("YOU WIN!");
            sf::FloatRect vb2 = victoryText.getLocalBounds();
            victoryText.setOrigin(vb2.width*0.5f, vb2.height*0.5f);
            victoryText.setScale(pulse, pulse);
            victoryText.setPosition(W*0.5f, H*0.36f);
            drawShadowText(victoryText, window);

            std::ostringstream ss;
            ss << "FINAL SCORE:  " << player.getScore();
            subText.setString(ss.str());
            sf::FloatRect slb = subText.getLocalBounds();
            subText.setOrigin(slb.width*0.5f, slb.height*0.5f);
            subText.setPosition(W*0.5f, H*0.52f);
            window.draw(subText);

            pressEnterText.setString("Press ENTER for mode select");
            sf::FloatRect plb = pressEnterText.getLocalBounds();
            pressEnterText.setOrigin(plb.width*0.5f, plb.height*0.5f);
            float blink = std::sin(victoryTimer*2.5f);
            pressEnterText.setFillColor(sf::Color(255,255,255, blink > 0 ? 255 : 80));
            pressEnterText.setPosition(W*0.5f, H*0.65f);
            window.draw(pressEnterText);
            pressEnterText.setFillColor(sf::Color(180,180,180));
        }

        window.display();
    }
    return 0;
}