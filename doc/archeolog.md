# Features of archeolog

The main features of archeolog are the folowing:

* Logging character strings

That's all.

The goal of archeolog is to be as simple of use as possible, in order to solve real life problems. The goal of archeolog is to help you to simplify information logging, not to make it more complex. It is really simple to learn. Here is an example of a usage of the default API which comes with archelog:

```cpp
#include <archeolog.h>

void Test()
{
    log_trace() << "This is a test of TRACE";
    log_warning() << "This is a test of WARNING";
    log_error() << "This is a test of ERROR";
}
```

In fact, the `log_xxx` functions are just some stupids macros you can very easily replace with your own macro style:

```cpp
#include <archeolog.h>

#define LOG_TYPE_CUSTOM 3

#define TRACE() archeolog(LOG_TYPE_TRACE)
#define WARNING() archeolog(LOG_TYPE_WARNING)
#define ERROR() archeolog(LOG_TYPE_ERROR)
#define CUSTOM() archeolog(LOG_TYPE_CUSTOM)

void Test()
{
    TRACE() << "This is a test of TRACE";
    WARNING() << "This is a test of WARNING";
    ERROR() << "This is a test of ERROR";
    CUSTOM() << "This is a test of CUSTOM";
}
```

Congratulations, you now know how to use archeolog, and even how to customize it!

*acheolog* client library is nothing more than a stupid logging framework which does almost nothing more than logging text strings. It symply try to store it in memory in an efficient maner, and that's all. If you want to store something else, it hase to be converted in a string. That's what happen to numbers for example:  

```cpp
void Test()
{
    log_trace() << "Number ten: " << 10;
}
```

Archeolog will convert the interger value `10` to the string `"10"` and store that string. So you can use the stream manipulators od iomanip to format the resulting string:

```cpp
void Test()
{
    log_trace() << "Number ten in hexa: " << std::hex << 10;
}
```

This code will log the string `Number ten in hexa: F`.

As you can see, we try not to "reinvent the wheel", and to simplify your learning of acheolog by reusing exisiting standard C++ features.

## What about the performance?

Archeolog is fast. In fact, archeolog is <i>really</i> fast. If you use it properly (which is easy to do), it can be amazing to see how fast it is, and how few memory it needs. It could be very easy to give the example of a source code that can log 1000 wide strings of 100 characters, including the source file name and the function name where each logging operation is done, all of that whithout filling a buffer of only 20Ko.

If you are good in maths, you may have compute that each string needs at least 200 bytes to be stored, so we need at least 1000*200 = 200Ko of memory (400Ko on some systems). And this is whithout taking in account the memory needed to store the source file names and so on.

However, it is possible, and even more, this example exists. It is used as an example for performance comparison against other logging libraries, as a typical unreal and unprobable tuned case of study used to fake/twick the benchmarks results in the favor of archeocode. As a result, the statictics are in favor of archeocode in a very impressive way, as acheocode appears to be 100x times faster than many of the other librairies.

It is possible to make archeocode even faster. But that's not the purpose. One of the main goal of archeocode is to help to solve real life problems. We found a way to implement it in a manner that allow very good performance,
always better than a basic use of C or C++ streams (printf, cout, ...). Unless you develop software running with very low resource in a very restricted environment, the main thing you should remember is that archeocode is much
enought fast for your need, and that you wont notice its overhead.
