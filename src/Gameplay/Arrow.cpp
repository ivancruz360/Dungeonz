#include "Arrow.hpp"
#include "Level.hpp"
#include "Living.hpp"
#include "../Collision/CollisionHandler.hpp"

Arrow::Arrow()
{
    m_type = EntityType::Arrow;

    m_sprite = SpritePtr_t(new Sprite());
    m_sprite->loadFromFile("arrow.png");
    m_sprite->setOrigin({12,12});

    m_box = BoxPtr_t(new Box());
    m_box->type = CollisionType::TriggerVolume;
    m_box->reactMaterial = CollMaterial::Regular;
    m_box->callback = [this]()
    {
        blow(nullptr);
    };
    m_box->enabled = false;

    CollisionHandler::Get().addBody(m_box);
}

void Arrow::init(const vec2f& pos, Direction_t dir)
{
    if (dir == Direction::Up or dir == Direction::Down)
        m_box->rect = Rectf(0,0,4,24);
    else
        m_box->rect = Rectf(0,0,24,4);

    switch (dir)
    {
        case Direction::Up:
            m_velocity = vec2f(0,-1);
            m_sprite->setRect({0,0,24,24});
            break;
        case Direction::Down:
            m_velocity = vec2f(0,1);
            m_sprite->setRect({0,24,24,24});
            break;
        case Direction::Left:
            m_velocity = vec2f(-1,0);
            m_sprite->setRect({0,48,24,24});
            break;
        case Direction::Right:
            m_velocity = vec2f(1,0);
            m_sprite->setRect({0,72,24,24});
            break;
    }

    setPosition(pos);
}

void Arrow::update(float deltaTime)
{
    if (m_warmup.getElapsedTime().asMilliseconds() > 150)
        m_box->enabled = true;

    if (m_box->enabled == true)
    {
        std::vector<Entity*> foundEnts = m_level->getEntitiesInRange(vec2f(m_box->rect.x + m_box->rect.w/2,
                                         m_box->rect.y + m_box->rect.h/2), 24);
        
        for (auto& i : foundEnts)
            blow(i);
    }

    vec2f translation(m_velocity * m_speed * deltaTime);
    move(translation);

    m_sprite->setPosition(vec2i(m_box->rect.x + m_box->rect.w/2, m_box->rect.y - 12));
    m_sprite->draw();
}

void Arrow::blow(Entity* ent)
{
    if (ent)
    {
        switch (ent->getType())
        {
            case EntityType::Living:
            {
                auto living = static_cast<Living*>(ent);

                if (!living->isDead() and living != m_owner and m_speed != 0)
                {
                    auto beholder = static_cast<Living*>(m_owner);

                    living->damage(100);
                    // living->push(m_direction, 8, 0.1);

                    if (living->isDead())
                    {
                        beholder->addXp(living->getXp());
                    }

                    m_speed = 0;
                    destroy();
                }
                else if (living->isDead())
                    return;
            }
            break;
        }
    }
    else
    {
        m_speed = 0;
        destroy();
    }
}

void Arrow::setOwner(Entity* owner)
{
    m_owner = owner;
}
