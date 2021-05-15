#pragma once

#include "pch.h"
#include "resourcepool.h"
#include "event.h"

#define MAX_CONC_SOUNDS 8

class SoundSystem
{
public:
    static SoundSystem* get()
    {
        if (!instance) instance = new SoundSystem;
        return instance;
    }

    void play(const std::string& file)
    {
        for (int i = 0; i < MAX_CONC_SOUNDS; i++)
        {
            if (sound[i].getStatus() == sf::Sound::Playing) continue;
            sound[i].setBuffer(m_sfx.get(file));
            sound[i].play();
            return;
        }
    }

    void processEvent(const Event& evn)
    {
        switch (evn.type)
        {
            case EventType::EVENT_PLAYER_COIN_PICKUP:
                play("coin.wav");
                break;
            case EventType::EVENT_PLAYER_DIED:
                play("die.wav");
                break;
            case EventType::EVENT_PLAYER_RESPAWN:
                play("respawn.wav");
                break;
        }
    }

private:
    static SoundSystem* instance;
    SoundSystem() : m_sfx(".wav")
    {
        EventSystem::subscribe(std::bind(&SoundSystem::processEvent, this,
                                         std::placeholders::_1));
        for (int i = 0; i < MAX_CONC_SOUNDS; i++)
            sound[i].setVolume(80.f);
    }

    ResourcePool<sf::SoundBuffer> m_sfx;
    sf::Sound sound[MAX_CONC_SOUNDS]; // up to 8 sounds can play at same time;
};
