#include "BigParticle.hpp"

BigParticle::BigParticle()
{
}

void BigParticle::init(const std::string& path, const vec2i& offset, float lifeLength)
{
    m_sprite.loadFromFile(path);
    vec2i orig = m_sprite.getOrigin();
    m_sprite.setOrigin(orig - offset);
    
    m_lifeLenght = lifeLength;
    m_timer.restart();
}

void BigParticle::update(float deltaTime)
{
    if (m_timer.getElapsedTime().asSeconds() >= m_lifeLenght)
        m_dead = true;

    m_sprite.update(deltaTime);
}

void BigParticle::setPosition(const vec2i& pos)
{
    m_sprite.setPosition(pos);
}

bool BigParticle::isDead()
{
    return m_dead;
}
