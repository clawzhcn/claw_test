#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

struct FallingChar {
    char ch;
    int y;
    int x;
};

std::vector<FallingChar> active_chars;
std::mutex mtx;
bool running = true;

// 非阻塞读取键盘输入
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void render() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(mtx);
        
        std::cout << "\033[2J\033[1;1H"; // 清屏
        for (const auto& item : active_chars) {
            std::cout << "\033[" << item.y << ";" << item.x << "H" << item.ch << std::flush;
        }
    }
}

void game_logic() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> char_dist('a', 'z');
    std::uniform_int_distribution<> x_dist(1, 40);

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        std::lock_guard<std::mutex> lock(mtx);
        
        // 随机产生字母
        active_chars.push_back({(char)char_dist(gen), 1, x_dist(gen)});
        
        // 移动
        for (auto& item : active_chars) {
            item.y++;
        }
        
        // 移除到底部的
        active_chars.erase(std::remove_if(active_chars.begin(), active_chars.end(), 
            [](const FallingChar& c){ return c.y > 20; }), active_chars.end());
    }
}

int main() {
    std::thread t1(render);
    std::thread t2(game_logic);

    std::cout << "Press 'q' to quit. Type falling characters to remove them!" << std::endl;

    while (running) {
        if (kbhit()) {
            char ch = getchar();
            if (ch == 'q') running = false;
            
            std::lock_guard<std::mutex> lock(mtx);
            active_chars.erase(std::remove_if(active_chars.begin(), active_chars.end(), 
                [ch](const FallingChar& c){ return c.ch == ch; }), active_chars.end());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    t1.join();
    t2.join();
    return 0;
}
