#pragma once


enum class EventType {
    None = 0,
    WindowResize,
    KeyPressed, KeyReleased,
    MouseMoved
};

class Event {
public:
    virtual EventType getEventType() = 0;
};


class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(unsigned int width, unsigned int height)
        : m_Width(width), m_Height(height) {}

    virtual EventType getEventType() override {return EventType::WindowResize;}

    unsigned int getWidth() {return m_Width;}
    unsigned int getHeight() {return m_Height;}
private:
    unsigned int m_Width, m_Height;
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

