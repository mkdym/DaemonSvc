//reference: http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus
#pragma once


//need function "MiniDumpWriteDump" in Dbghelp.dll which is always existing on Windows XP and later version
//if you want to use it on Windows 2000, you can put Windows XP's Dbghelp.dll in your program dir
//dmp file name will end with ".dmp" in exe dir, see source file for details


namespace exception_catcher
{
    //some exception handlers work on per process, others work on per thread
    //per process: SEH exception, pure virtual function call, C++ new exception, runtime invalid parameter error, signal: SIGABRT, SIGINT, SIGTERM
    //per thread: terminate() call, unexpected() call, signal: SIGFPE, SIGILL, SIGSEGV
    void set_process_exception_handlers();
    void set_thread_exception_handlers();
}



/*

//
//
//代码修改自http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus
//有两处大修改：
//      1. 原代码中GetExceptionPointers的实现是复制自vc8.0，我这里是复制自vc9.0
//      2. 原代码中是动态获取MiniDumpWriteDump函数的地址，我这里是静态链接了Dbghelp.dll
//  因为经我测试，虽然Windows2000的Dbghelp.dll里没有MiniDumpWriteDump函数，但是是可以用WindowsXPSP3的Dbghelp.dll而正常生成dump文件的
//  所以，你最好将WindowsXPSP3的Dbghelp.dll和你的应用程序一同发布。我没有测试其它版本的Dbghelp.dll是否能在Windows2000上用
//
//

//对于示例中的SEH异常，一次会有两个dmp文件，第一个是SIGSEGV的，第二个是SEH的。两个都指示到了正确的异常语句
//对于示例中的terminate钩子，一次会产生两个dmp文件，第一个是terminate的，第二个是SIGABRT的。但是两个都指示到了unexpected()的调用位置？？？
//对于示例中的unexpected钩子，一次会产生三个dmp文件，第一个是unexpected的，第二个是terminate的，第三个是SIGABRT的。都指示到了unexpected()的调用位置
//对于示例中的纯虚函数调用异常，一次会产生两个dmp文件，第一个是纯虚函数调用的，第二个是SIGABRT的。都指示到了纯虚函数调用的位置
//对于示例中的非法参数调用，一次会产生一个dmp文件，指示位置正确
//对于示例中的new异常，一次会产生两个dmp文件，第一个是new的，指示位置正确，第二个是SEH的，调用栈：AVbadcast->_onexit->_unlockexit->_unlock->LeaveCriticalSection
//对于示例中的SIGABRT信号钩子，一次会产生一个dmp文件，指示位置正确
//对于示例中的SIGFPE信号钩子，我测试的时候除零语句并没有发生除零异常（Release版），可能还要设置什么东西，所以无法验证是否能够捕获到除零异常
//对于示例中的SIGILL信号钩子，一次会产生一个dmp文件，指示位置正确
//对于示例中的SIGINT信号钩子，一次会产生一个dmp文件，指示位置正确
//对于示例中的SIGSEGV信号钩子，一次会产生一个dmp文件，指示位置正确
//对于示例中的SIGTERM信号钩子，一次会产生一个dmp文件，指示位置正确
//对于示例中的RaiseException异常，在不同的code下可能产生不同个数的dmp文件，不过调用栈都是：_onexit->printf
//对于示例中的C++类型异常，一次会产生一个dmp文件，是SEH的，根据类型的不同调用栈不同，但是都没指向正确的代码上？？？


//
//下面是原代码中的示例程序（稍加修改）
//


#include <float.h>
#include <stdio.h>
#include <signal.h>
#include <conio.h>
#include <stdlib.h>
#include <exception>
#include <Windows.h>
#include "logger.h"
#include "exception_catcher.h"



void sigfpe_test()
{ 
    // Code taken from http://www.devx.com/cplus/Article/34993/1954

    //Set the x86 floating-point control word according to what
    //exceptions you want to trap. 
    _clearfp(); //Always call _clearfp before setting the control
    //word
    //Because the second parameter in the following call is 0, it
    //only returns the floating-point control word
    unsigned int cw; 
    _controlfp_s(&cw, 0, 0); //Get the default control
    //word
    //Set the exception masks off for exceptions that you want to
    //trap.  When a mask bit is set, the corresponding floating-point
    //exception is //blocked from being generating.
    cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_ZERODIVIDE|
        EM_DENORMAL|EM_INVALID);
    //For any bit in the second parameter (mask) that is 1, the 
    //corresponding bit in the first parameter is used to update
    //the control word.  
    unsigned int cwOriginal;
    _controlfp_s(&cwOriginal, cw, MCW_EM); //Set it.
    //MCW_EM is defined in float.h.
    //Restore the original value when done:
    //_controlfp(cwOriginal, MCW_EM);

    // Divide by zero

    float a = 1;
    float b = 0;
    float c = a/b;
    c; 
}

#define BIG_NUMBER 0x1fffffff
#pragma warning(disable: 4717) // avoid C4717 warning
int RecurseAlloc() 
{
    int *pi = new int[BIG_NUMBER];
    pi;
    RecurseAlloc();
    return 0;
}

class CDerived;
class CBase
{
public:
    CBase(CDerived *derived): m_pDerived(derived) {};
    ~CBase();
    virtual void function(void) = 0;

    CDerived * m_pDerived;
};

#pragma warning(disable:4355)
class CDerived : public CBase
{
public:
    CDerived() : CBase(this) {};   // C4355
    virtual void function(void) {};
};

CBase::~CBase()
{
    m_pDerived -> function();
}

int main()
{
    InitLog("", 0, LOG_DEBUG);

    exception_catcher::SetProcessExceptionHandlers();
    exception_catcher::SetThreadExceptionHandlers();

    printf("Choose an exception type:\n");
    printf("0 - SEH exception\n");
    printf("1 - terminate\n");
    printf("2 - unexpected\n");
    printf("3 - pure virtual method call\n");
    printf("4 - invalid parameter\n");
    printf("5 - new operator fault\n");	
    printf("6 - SIGABRT\n");
    printf("7 - SIGFPE\n");
    printf("8 - SIGILL\n");
    printf("9 - SIGINT\n");
    printf("10 - SIGSEGV\n");
    printf("11 - SIGTERM\n");
    printf("12 - RaiseException\n");
    printf("13 - throw C++ typed exception\n");
    printf("Your choice >  ");

    int ExceptionType = 0;
    scanf_s("%d", &ExceptionType);

    switch(ExceptionType)
    {
    case 0: // SEH
        {
            // Access violation
            int *p = 0;
#pragma warning(disable : 6011)   // warning C6011: Dereferencing NULL pointer 'p'
            *p = 0;
#pragma warning(default : 6011)   
        }
        break;
    case 1: // terminate
        {
            // Call terminate
            terminate();
        }
        break;
    case 2: // unexpected
        {
            // Call unexpected
            unexpected();
        }
        break;
    case 3: // pure virtual method call
        {
            // pure virtual method call
            CDerived derived;
        }
        break;
    case 4: // invalid parameter
        {      
            char* formatString;
            // Call printf_s with invalid parameters.
            formatString = NULL;
#pragma warning(disable : 6387)   // warning C6387: 'argument 1' might be '0': this does not adhere to the specification for the function 'printf'
            printf(formatString);
#pragma warning(default : 6387)   

        }
        break;
    case 5: // new operator fault
        {
            // Cause memory allocation error
            RecurseAlloc();
        }
        break;
    case 6: // SIGABRT 
        {
            // Call abort
            abort();
        }
        break;
    case 7: // SIGFPE
        {
            // floating point exception ( /fp:except compiler option)
            sigfpe_test();            
        }    
        break;
    case 8: // SIGILL 
        {
            raise(SIGILL);              
        }    
        break;
    case 9: // SIGINT 
        {
            raise(SIGINT);              
        }    
        break;
    case 10: // SIGSEGV 
        {
            raise(SIGSEGV);              
        }    
        break;
    case 11: // SIGTERM
        {
            raise(SIGTERM);            
        }
        break;
    case 12: // RaiseException 
        {
            // Raise noncontinuable software exception
            RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);        
        }
        break;
    case 13: // throw 
        {
            // Throw typed C++ exception.
            throw std::runtime_error("123");
        }
        break;
    default:
        {
            printf("Unknown exception type specified.");       
            _getch();
        }
        break;
    }

    system("pause");
    return 0;
}

*/

