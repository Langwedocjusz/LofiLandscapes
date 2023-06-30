#include "Application.h"

#include <iostream>

int main() {
    try {
        Application app("LofiLandscapes", 800, 600);

        app.StartMenu();
        app.Init();
        app.Run();
    }

    catch(std::string error_message) {
        std::cerr << error_message << '\n';
        return -1;
    }

    return 0;
}
