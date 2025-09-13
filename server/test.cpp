#include "db.h"
#include <iostream>
#include <vector>

int main() {
    Database db("data/memory.db");

    // insert with vec
    std::vector<float> v1 = {1.0f, 2.0f, 3.0f};
    db.savePrompt("hello?", "world!", "recall", v1, "en");

    // insert without vec
    db.savePrompt("ping", "pong", "chat", {}, "en");

    // history
    auto rows = db.chatHistory(10);
    std::cout << "got " << rows.size() << " rows\n";
    for (auto& row : rows) {
        std::cout << "Q: " << row.first << "\nA: " << row.second << "\n---\n";
    }

    return 0;
}

