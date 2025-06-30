#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <conio.h>
#include <mutex>
#include <windows.h>
#include <vector>
#include <cmath>

//global varaibles
std::atomic<bool> stopProgram(false);
std::mutex consoleMutex; //Mutex to protect console operations

//console helper functions

//hides blinking cursor
void hideCursor(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

//shows blinking cursor
void showCursor(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

//sets cursor's position in console window
void setCursorPosition(int x, int y){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(hConsole, pos);
}

//clears entire screen 
void clearScreen(){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD count;
    DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
    COORD homeCoords {0,0};
    FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
}

//thread functions

//thread function for console animation
void marqueeThreadFunc(const std::vector<std::string>& logo){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    const int screenWidth = csbi.dwSize.X;
    const int bannerHeight = 3; // 3 lines for banner
    const int screenHeight = csbi.dwSize.Y - 2; //avoid top and input line

    //calculate logo dimensions
    int logoHeight = static_cast<int>(logo.size());
    int logoWidth = 0;
    for(const auto& line : logo){
        if(static_cast<int>(line.length()) > logoWidth){
            logoWidth = static_cast<int>(line.length());
        }
    }

    double currentX = 0.0;
    double currentY = static_cast<double>(bannerHeight); //start below welcome message

    //speed and direction
    double dx = 0.2;
    double dy = 0.2;

    int lastDrawX = -1;
    int lastDrawY = -1;

    while(!stopProgram){
        {
            std::lock_guard<std::mutex> lock(consoleMutex);

            //erase previous text
            if(lastDrawX != -1 && lastDrawY != -1){
               for(int i=0; i < logoHeight; i++){
                    setCursorPosition(lastDrawX, lastDrawY + i);
                    std::cout << std::string(logoWidth, ' ');
               }
            }

            //move position
            currentX += dx;
            currentY += dy;

            //direction vector (bounce on edges while respecting banner)
            if(currentX <= 0 || currentX + logoWidth >= screenWidth){
                dx = -dx;
            }
            if(currentY <= bannerHeight || currentY + logoHeight >= screenHeight - 1){
                dy = -dy;
            }

            //clamp text position 
            if(currentX < 0){
                currentX = 0;
            }
            if(currentX + logoWidth >= screenWidth){
                currentX = screenWidth - logoWidth -1;
            }
            if(currentY < bannerHeight){
                currentY = bannerHeight;
            }
            if(currentY + logoHeight >= screenHeight - 1){
                currentY = screenHeight - logoHeight - 1;
            }

            int drawX = static_cast<int>(currentX);
            int drawY = static_cast<int>(currentY);

            //draw logo
            for(int i=0; i < logoHeight; i++){
                setCursorPosition(drawX, drawY + i);
                std::cout << logo[i] << std::flush;
            }

            lastDrawX = drawX;
            lastDrawY = drawY;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//thread function for user input
void inputThreadFunc(){
    std::string inputBuffer;
    std::string prompt = "Type 'exit' to exit: ";

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    const int inputLineY = csbi.dwSize.Y - 1;

    //display static welcome message once at top
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        setCursorPosition(0, 0);
        std::cout << "**************************************\n" << std::flush;
        //might need to remove the \033[3m ... \033[0m if terminal doesn't support
        std::cout << "* DVD Logo goes \033[3mboing boing\033[0m hehe :-) *\n" << std::flush;
        std::cout << "**************************************" << std::flush;
    }

    while(!stopProgram){
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            setCursorPosition(0, inputLineY);
            std::cout << prompt << inputBuffer << "      ";
            setCursorPosition(prompt.length() + inputBuffer.length(), inputLineY);
        }

        if(_kbhit()){
            char ch =_getch();
            if(ch == '\r'){//enter is pressed
                if(inputBuffer == "exit"){
                    std::cout << "\nSee you again! >:-)";
                    Sleep(3000);
                    stopProgram = true;
                }
            }else if(ch == '\b'){//if backspace is pressed
                if (!inputBuffer.empty()) {
                    inputBuffer.pop_back();
                }
            } else if (isprint(ch)) {
                inputBuffer += ch;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

//main function
int main(){
    SetConsoleOutputCP(CP_UTF8); // for ascii art
    srand(static_cast<unsigned int>(time(nullptr)));
    hideCursor();
    clearScreen();

    std::vector<std::string> logo = {
        " ⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣶⣦",
        "⢠⣿⣿⡿⠀⠀⠈⢹⣿⣿⡿⣿⣿⣇⠀⣠⣿⣿⠟⣽⣿⣿⠇⠀⠀⢹⣿⣿⣿",
        "⢸⣿⣿⡇⠀⢀⣠⣾⣿⡿⠃⢹⣿⣿⣶⣿⡿⠋⢰⣿⣿⡿⠀⠀⣠⣼⣿⣿⠏",
        "⣿⣿⣿⣿⣿⣿⠿⠟⠋⠁⠀⠀⢿⣿⣿⠏⠀⠀⢸⣿⣿⣿⣿⣿⡿⠟⠋⠁⠀",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣀⣸⣟⣁⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
        "⣠⣴⣶⣾⣿⣿⣻⡟⣻⣿⢻⣿⡟⣛⢻⣿⡟⣛⣿⡿⣛⣛⢻⣿⣿⣶⣦⣄⡀",
        "⠉⠛⠻⠿⠿⠿⠷⣼⣿⣿⣼⣿⣧⣭⣼⣿⣧⣭⣿⣿⣬⡭⠾⠿⠿⠿⠛⠉⠀"
    };

    std::thread marquee(marqueeThreadFunc, std::cref(logo));
    std::thread input(inputThreadFunc);

    marquee.join();
    input.join();

    clearScreen();
    setCursorPosition(0, 0);
    showCursor();
    std::cout << "Program stopped." << std::endl;

    return 0;
}


