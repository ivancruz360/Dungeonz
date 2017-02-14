#ifndef ai_player_hpp
#define ai_player_hpp
#include "ai.hpp"
#include <SFML/System/Clock.hpp>

enum class PlayerState
{
	MOVING = 0,
	IDLE,
	ATTACK,
	PICKING
};

class Entity;

class AIPlayer : public AI
{
	public:
		virtual void setup() override final;
		virtual void update(float deltaTime) override final;
		void movingState(float deltaTime);
		void idleState(float deltaTime);
		void attackState(float deltaTime);
		void pickingState(float deltaTime);

		void focus();
	private:
		float     m_speed = 100;
		Entity*   m_focus;
		sf::Clock m_timer;
		PlayerState m_state;
};

#endif
