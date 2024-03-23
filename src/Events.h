#pragma once

#include <cstdint>

enum class EventType {
    None = 0,
    WindowResize,
    KeyPressed, KeyReleased,
    MouseMoved, MousePressed, MouseReleased
};

class Event {
public:
    virtual EventType getEventType() = 0;
};


class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {}

    virtual EventType getEventType() override {return EventType::WindowResize;}

    uint32_t getWidth() {return m_Width;}
    uint32_t getHeight() {return m_Height;}
private:
    uint32_t m_Width, m_Height;
};

class KeyPressedEvent : public Event {
public:
    KeyPressedEvent(int keycode, bool repeat)
        : m_Keycode(keycode), m_Repeat(repeat) {}

    virtual EventType getEventType() override {return EventType::KeyPressed;}
    
    int getKeycode() {return m_Keycode;}
    bool getRepeat() {return m_Repeat;}
private:
    int m_Keycode;
    bool m_Repeat;
};

class KeyReleasedEvent : public Event {
public:
    KeyReleasedEvent(int keycode) : m_Keycode(keycode) {}

    virtual EventType getEventType() override {return EventType::KeyReleased;}

    int getKeycode() {return m_Keycode;}
private:
    int m_Keycode;
};

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : m_X(x), m_Y(y) {}

    virtual EventType getEventType() override {return EventType::MouseMoved;}

    float getX() {return m_X;}
    float getY() {return m_Y;}
private:
    float m_X, m_Y;
};

class MousePressedEvent : public Event {
public:
    MousePressedEvent(int button, int mods)
        : m_Button(button), m_Mods(mods) {}

    virtual EventType getEventType() override { return EventType::MousePressed; }

    int getButton() { return m_Button; }
    int getMods() { return m_Mods; }

private:
    int m_Button, m_Mods;
};

class MouseReleasedEvent : public Event {
public:
    MouseReleasedEvent(int button, int mods)
        : m_Button(button), m_Mods(mods) {}

    virtual EventType getEventType() override { return EventType::MouseReleased; }

    int getButton() { return m_Button; }
    int getMods() { return m_Mods; }

private:
    int m_Button, m_Mods;
};
