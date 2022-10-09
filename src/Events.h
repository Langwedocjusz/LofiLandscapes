#pragma once

enum class EventType {
    None = 0,
    KeyPressed
};

class Event {
public:
    virtual EventType getEventType() = 0;
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
