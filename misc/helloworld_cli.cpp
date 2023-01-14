// helloworld_cli.cpp
#using <mscorlib.dll>
using namespace System;
using namespace System::Threading;

int main()
{
    Console::WriteLine("Hello, World!");
    Thread::Sleep(100000);
    return 0;
}
