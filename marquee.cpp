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
void marqueeThreadFunc(std::string text){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    const int screenWidth = csbi.dwSize.X;
    const int bannerHeight = 3; // 3 lines for banner
    const int screenHeight = csbi.dwSize.Y - 2; //avoid top and input line

    double currentX = 0.0;
    double currentY = static_cast<double>(bannerHeight); //start below welcome message

    int targetX = rand() % (screenWidth - text.length());
    int targetY = 1 + rand() % (screenHeight - 1);

    const double speed = 1.0;
    std::string eraser(text.length(), ' ');

    int lastDrawX = - 1;
    int lastDrawY = -1;

    while(!stopProgram){
        {
            std::lock_guard<std::mutex> lock(consoleMutex);

            //erase previous text
            if(lastDrawX != -1){
                setCursorPosition(lastDrawX, lastDrawY);
                std::cout << eraser;
            }

            //randomize new target occasionally
            if(std::abs(currentX - targetX) < 1.0 && std::abs(currentY - targetY) < 1.0
               || rand() % 10 == 0){
                    targetX = rand() % (screenWidth - text.length());
                    targetY = 1 + rand() % (screenHeight - 1);
            }

            //direction vector
            double dirX = targetX - currentX;
            double dirY = targetY - currentY;
            double distance = std::sqrt(dirX * dirX + dirY * dirY);

            if(distance > 0){
                currentX += (dirX/distance) * speed;
                currentY += (dirY/distance) * speed;
            }

            //clamp and draw
            int drawX = static_cast<int>(currentX);
            int drawY = static_cast<int>(currentY);
            if(drawY < 1){
                drawY = 1;
            }
            if(drawX + text.length() >= screenWidth){
                drawX = screenWidth - text.length() - 1;
            }
            if(drawY >= screenHeight -1){
                drawY = screenHeight - 2;
            }

            setCursorPosition(drawX, drawY);
            std::cout << text << std::flush;

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
        std::cout << "******************************\n" << std::flush;
        std::cout << "* Displaying a marquee console! * \n" << std::flush;
        std::cout << "******************************\n" << std::flush;
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
                    std::cout << "\nSee you again! :-)";
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

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//main function
int main(){
    srand(static_cast<unsigned int>(time(nullptr)));
    hideCursor();
    clearScreen();

    std::string message = "Hello WWOrld!";

    std::thread marquee(marqueeThreadFunc, message);
    std::thread input(inputThreadFunc);

    marquee.join();
    input.join();

    clearScreen();
    setCursorPosition(0, 0);
    showCursor();
    std::cout << "Program stopped." << std::endl;

    return 0;
}


