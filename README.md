# ETW Patching For Dummies

## Inrtoduction to all things ETW

When initially getting into ETW and patching it, I was not prepared for what was coming. It was a deep rabbit hole of `.NET`, `CLR` and `Windows Internals`. Coming from a pure-`C` background(and a bit of `Rust`), this was a deep hole I was not prepared for at all. This blog is supposed to be more of a Journal than a tutorial as I try to figure things out! So, if you are someone who is as lost as I am, with Zero knowledge of the `.NET Framework` or any of the `C#` stuff and want to go around patching stuff, welcome to the journey.

*PS: There will be a scramble of topics in no particular order, mostly discussing topics relevant to the cause.*

## First-things-first: The .NET Framework
So, when I started writing this blog, I had no idea what `.NET` really was. All knew was that its _some-sorta_ framework by Microsoft for doing windows stuff. Turns out, it is a framework for `C#` and a version of `C++` called `C++/CLI`, which can be compiled with `Visual C++` on Windows. Here, my first question was, *"Hey, then how is it different from normal C++ code?"* 

And to solve that question, I decided to do what all good devs would have done: write a _"Hello World"_ program in both styles.

**(Normal)C++**
```cpp
#include <iostream>
using namespace std;

int main() 
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```
**C++/CLI**
```cpp
#using <mscorlib.dll>
using namespace System;

int main(array<System::String ^> ^args)
{
    Console::WriteLine("Hello, World!");
    return 0;
}
```
Both of these can be compiled with `cl.exe` aka the `Visual Studio Compiler` as such:

```powershell
> cl.exe helloworld.cpp
```

## Compilation
To compile the sources, you can run the `compile.bat` script:
```powershell
> .\compile.bat
```

Or, to compile them manually, run:
- **Consumer**
```powershell
cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /I .\includes /Tc.\src\consumer.c /link /OUT:consumer.exe /SUBSYSTEM:CONSOLE /MACHINE:x64
```
- **Patcher**
```powershell
cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /I .\includes /Tc.\src\patcher.c /link /OUT:patcher.exe /SUBSYSTEM:CONSOLE /MACHINE:x64
```

## References
- [Hiding Your .NET – ETW](https://www.mdsec.co.uk/2020/03/hiding-your-net-etw/)
- [Cobalt Strike 3.11 – The snake that eats its tail](https://www.cobaltstrike.com/blog/cobalt-strike-3-11-the-snake-that-eats-its-tail/)
- [Common Language Runtime #1: An Introduction](https://mez0.cc/posts/common-language-runtime-1/https://mez0.cc/posts/common-language-runtime-1/)
