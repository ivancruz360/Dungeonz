#include "StateMenu.hpp"
#include "../Core/Screen.hpp"
#include "../Resource/FontCache.hpp"
#include "../Render/Renderer.hpp"
#include "../Input/InputHandler.hpp"

constexpr int g_menuOptionsOffset = 48;

StateMenu::StateMenu()
{
}

void StateMenu::init()
{
    m_tmpLogo.setFont(*FontCache::Get().getFont("BLKCHCRY.ttf"));
    m_tmpLogo.setCharacterSize(128);
    m_tmpLogo.setString("Dungeonz");
    m_tmpLogo.setOrigin(static_cast<int>(m_tmpLogo.getLocalBounds().width /2),
                        static_cast<int>(m_tmpLogo.getLocalBounds().height /2));
    m_tmpLogo.setPosition({Screen::Get().halfWidth, Screen::Get().height /4});

    for (int i = 0; i < MenuOptions::NumMenuOptions; i++)
    {
        m_options[i].setFont(*FontCache::Get().getFont("BLKCHCRY.ttf"));
        m_options[i].setCharacterSize(32);
    }

    m_options[MenuOptions::NewGame].setString("New Game");
    m_options[MenuOptions::HowToPlay].setString("How To Play");
    m_options[MenuOptions::Exit].setString("Exit");

    for (int i = 0; i < MenuOptions::NumMenuOptions; i++)
    {
        m_options[i].setOrigin(static_cast<int>(m_options[i].getLocalBounds().width /2), 0);
        m_options[i].setPosition(Screen::Get().halfWidth,
                                 Screen::Get().halfHeight + i * g_menuOptionsOffset);
    }

    m_helpText.setFont(*FontCache::Get().getFont("BLKCHCRY.ttf"));
    m_helpText.setCharacterSize(30);
    m_helpText.setString("There you go! Your beloved manual.\n\nUp, Down, Left, Right = Movement\nF = Attack\nD = Dodge/Roll\nE = Use, Interact, Take, Loot\nR = Cast a selected spell\nS = Spellbook\n");
    m_helpText.setOrigin(static_cast<int>(m_helpText.getLocalBounds().width/2),
                         static_cast<int>(m_helpText.getLocalBounds().height/2));
    m_helpText.setPosition({Screen::Get().halfWidth, Screen::Get().halfHeight});
}

void StateMenu::update(float deltaTime)
{
    switch (m_state)
    {
        case MenuState::MainScreen:
            menuState();
        break;
        case MenuState::HelpScreen:
            helpState();
        break;
    }
}

void StateMenu::menuState()
{
    if (InputHandler::Get().isUp() and m_timer.getElapsedTime().asMilliseconds() > 150)
    {
        if (m_chosen > 0)
            m_chosen--;
        m_timer.restart();
    }
    else if (InputHandler::Get().isDown() and m_timer.getElapsedTime().asMilliseconds() > 150)
    {
        if (m_chosen < MenuOptions::NumMenuOptions -1)
            m_chosen++;
        m_timer.restart();
    }
    else if (InputHandler::Get().isKeyPressed(sf::Keyboard::Return))
    {
        switch (m_chosen)
        {
            case MenuOptions::NewGame:
                m_newGameFunc();
                break;
            case MenuOptions::HowToPlay:
                m_state = MenuState::HelpScreen;
                break;
            case MenuOptions::Exit:
                m_exitFunction();
                break;
        }
    }

    for (auto& i : m_options)
        i.setFillColor({128,128,128});

    m_options[m_chosen].setFillColor(sf::Color::White);

    Renderer::Get().submitOverlay(&m_tmpLogo);

    for (auto& i : m_options)
        Renderer::Get().submitOverlay(&i);
}

void StateMenu::helpState()
{
    if ((InputHandler::Get().isEscape() or InputHandler::Get().isInv()) and
        m_timer.getElapsedTime().asMilliseconds() > 150)
    {
        m_state = MenuState::MainScreen;
    }

    Renderer::Get().submitOverlay(&m_helpText);
}

void StateMenu::leave()
{
}

void StateMenu::setNewGameFunc(std::function<void ()> func)
{
    m_newGameFunc = func;
}