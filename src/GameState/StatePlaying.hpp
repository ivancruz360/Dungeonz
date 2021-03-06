#ifndef STATE_PLAYING_HPP
#define STATE_PLAYING_HPP
#include "GameState.hpp"
#include "../Gameplay/Level.hpp"

enum class TheLevel
{
    First,
    Second
};

class Level;

class StatePlaying : public GameState
{
    public:
        StatePlaying();
        StatePlaying(const std::string& loadFirst);

        void init() override final;
        void update(float deltaTime) override final;
        void leave() override final;

        void setLevel(const std::string& level, Level::InitMode mode);
        void begForLevel(const std::string& level, Level::InitMode mode);

    private:
        Level m_level;
        std::string m_consider;
        bool m_considered = true;
        Level::InitMode m_conMode;

        std::string m_loadFirst;
};

#endif