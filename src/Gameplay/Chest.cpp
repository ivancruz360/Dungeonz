#include "Chest.hpp"
#include "../Collision/CollisionHandler.hpp"
#include "../Render/IndicationHandler.hpp"

Chest::Chest()
{
    m_type = Entity::Type::Chest;
    m_sprite = Sprite::Ptr(new Sprite());
    m_sprite->loadFromFile("chest.png");
    m_sprite->setOrigin({16,32});

    m_box = Box::Ptr(new Box());
    m_box->rect = Rectf(0,0,26,16);
    m_box->type = Box::Type::Static;

    CollisionHandler::Get().addBody(m_box);
}

void Chest::update(float deltaTime)
{
    m_sprite->setPosition({m_box->rect.x + m_box->rect.w/2, m_box->rect.y + m_box->rect.h});
    m_sprite->draw();
}

void Chest::setRequiredItem(const std::string& item)
{
    m_requiredKey = item;
}

void Chest::tryOpening(Inventory* inv)
{
    if (inv->hasItem(m_requiredKey) and !m_isOpen)
    {
        m_isOpen = true;
        inv->removeItem(m_requiredKey);
        IndicationHandler::Get().addIndication("Unlocked", sf::Color(0,255,0), getPosition() + vec2f(0,-50));
    }
    else
        IndicationHandler::Get().addIndication("Locked", sf::Color(255,255,0), getPosition() + vec2f(0,-50));
}

void Chest::open()
{
    m_isOpen = true;
}

bool Chest::isOpen()
{
    return m_isOpen;
}

Inventory& Chest::accessInv()
{
    return m_inv;
}