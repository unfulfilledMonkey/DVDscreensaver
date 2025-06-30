# DVDscreensaver
Run: g++ marquee.cpp -o <marqueeConsole> (or any name you want) to compile the code <br />
Then you can run marquee.exe (for windows) in the terminal of your IDE or run the .exe file generated <br />
Change std::this_thread::sleep_for(std::chrono::milliseconds(...)); found in the bottom of the thread functions depending on your machine's refresh rate; this helps with the flickering of the animation
