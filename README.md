# ETW Patching For Dummies

## Inrtoduction to all things ETW

When initially getting into ETW and patching it, I was not prepared for what was coming. It was a deep rabbit hole of `.NET`, `CLR` and `Windows Internals`. Coming from a pure-`C` background(and a bit of `Rust`), this was a deep hole I was not prepared for at all. This blog is supposed to be more of a Journal than a tutorial as I try to figure things out! So, if you are someone who is as lost as I am, with Zero knowledge of the `.NET Framework` or any of the `C#` stuff and want to go around patching stuff, welcome to the journey.

*PS: There will be a scramble of topics in no particular order, mostly discussing topics relevant to the cause.*

## First-things-first: The .NET Framework
So, when I started writing this blog, I had no idea what `.NET` really was. All knew was that its _some-sorta_ framework by Microsoft for doing windows stuff. Turns out, it is a framework for `C#` and a version of `C++` called `C++/CLI`, which can be compiled with `Visual C++` on Windows. Here, my first question was, *"Hey, then how is it different from normal C++ code?"* 

And to solve that question, I decided to do what all good devs would have done: write a _"Hello World"_ program in both styles.

**Native C++**
```cpp
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
```
**C++/CLI**
```cpp
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
```
Both of these can be compiled with `cl.exe` aka the `Visual Studio Compiler` as such:

```powershell
> cl.exe helloworld.cpp /EHsc
> cl.exe helloworld_cli.cpp /clr
```

This should give us two binaries: `helloworld.exe` and `helloworld_cli.exe`, both of which print the same `"Hello, World!"` statement and then go to sleep. The sleep part is there to give us enough time to examine the process. 

The first thing which I noticed was that the `C++/CLI` excutable was much smaller than the native `C++` executable.

```bash
10-01-2023  15:19           242,688 helloworld.exe
10-01-2023  04:49            32,256 helloworld_cli.exe
```
This is becase when you compile a C++/CLI program, the resulting executable is actually a combination of native machine code and `managed code`.

`Managed code` is code that runs under the control of the `.NET` runtime, which provides a number of services such as memory management, exception handling, and type safety (we would talk more about this later). Because the runtime provides these services, managed code does not need to include the same kind of infrastructure that is required for native code.

As a result, `C++/CLI` executables are typically smaller in size than native `C++` executables. This is because they don't need to include memory management, exception handling and other related implementations themselves, which are provided by the runtime.

Additionally, C++/CLI codes makes use of the `CLR` (another thing we would get to later) which is a part of the `.NET` Framework, this framework ships with the operating system, as such `C++/CLI` application does not require any additional runtime or library dependencies.

Looking at each process in [Process Explorer](https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer), we see the following:

![](./img/helloworld_process_explorer.png?raw=True)

![](./img/helloworld_cli_process_explorer.png)

The first thing I noticed was that the `C++/CLI` executable has two extra tabs: 
- `.NET Assemblies` - This feature allows you to view the `.NET` assemblies that are loaded in a specific process. The tab displays information such as the assembly name, version, and location, as well as the classes and methods that are present in each assembly. This information can be helpful for troubleshooting issues with `.NET` applications, as well as for understanding the internal workings of a process.
  
- `.NET Performance` - This feature allows you to view performance statistics for the `.NET Framework` in a specific process. The tab displays information such as the number of `.NET` objects and exceptions, the number of GC collections, the amount of memory allocated, and the CPU time consumed by the `.NET` runtime. This tab also allows you to view performance counter metrics for the `CLR` such as the number of bytes in all heaps and the current number of Gen 0, 1 and 2 collections. This information can be helpful for diagnosing performance issues in `.NET` applications, such as high memory usage or poor garbage collection behavior.


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
- [Common Language Runtime #1: An Introduction](https://mez0.cc/posts/common-language-runtime-1)
