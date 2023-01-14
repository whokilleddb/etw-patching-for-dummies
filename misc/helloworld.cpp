// helloworld.cpp
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

int main() 
{
    std::cout << "Hello, World!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(100));
    return 0;
}