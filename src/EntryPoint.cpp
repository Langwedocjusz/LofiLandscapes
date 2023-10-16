#include "Application.h"

#include <iostream>

int main() {
    try {
        Application app("LofiLandscapes", 800, 600);

        app.StartMenu();
        app.Init();
        app.Run();
    }

    catch(const std::exception &e) {
        std::cerr << "Exception was thrown:\n";
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 0;
}
