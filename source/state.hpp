#pragma once

class State
{
public:
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update() = 0;

    virtual ~State() = default;
};