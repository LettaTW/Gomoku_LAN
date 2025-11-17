#include <iostream>
#include "NetworkServer.h"

int main() {
    try {
        // TODO (網路) 檢查並完成 NetworkServer 的實作
        NetworkServer server(8686);
        server.start();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}