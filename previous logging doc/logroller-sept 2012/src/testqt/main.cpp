#include <QStringList>
#include <QVector>
#include <QThread>
#include <QtConcurrentRun>
#include <unistd.h>

#include <QDebug>

#include <iostream>
using namespace std;
#include <sys/time.h>

QString getNumberName(unsigned char n)
{
    QString result;

    switch (n / 100)
    {
    case 1: result += "one hundred"; break;
    case 2: result += "two hundred"; break;
    }

    if (n > 100 && n % 100 != 0) result += ' ';
    n = n % 100;

    switch (n / 10)
    {
    case 0: break;
    case 1:
    {
        switch (n % 10)
        {
        case 0: result += "ten"; break;
        case 1: result += "eleven"; break;
        case 2: result += "twelve"; break;
        case 3: result += "thirteen"; break;
        case 4: result += "fourteen"; break;
        case 5: result += "fithteen"; break;
        case 6: result += "sixteen"; break;
        case 7: result += "seventeen"; break;
        case 8: result += "heighteen"; break;
        case 9: result += "nineteen"; break;
        }
        break;
    }
    case 2: result += "twenty"; break;
    case 3: result += "thirty"; break;
    case 4: result += "fourty"; break;
    case 5: result += "fifty"; break;
    case 6: result += "sixty"; break;
    case 7: result += "seventy"; break;
    case 8: result += "heighty"; break;
    case 9: result += "ninety"; break;
    }

    if (n < 10 || n > 19)
    {
        if (!result.isEmpty()) result += ' ';
        switch (n % 10)
        {
            case 0: if (result.isEmpty()) result = "zero"; break;
            case 1: result += "one"; break;
            case 2: result += "two"; break;
            case 3: result += "three"; break;
            case 4: result += "four"; break;
            case 5: result += "five"; break;
            case 6: result += "six"; break;
            case 7: result += "seven"; break;
            case 8: result += "height"; break;
            case 9: result += "nine"; break;
        }

    }
    return result;
}

static volatile size_t current_nb = 0;
static bool quit = false;

static const int nb = 1000;
static QVector<QString> * strings = 0;

static size_t sum = 0;
void Log(QString s)
{
    if (strings == 0)
    {
        strings = new QVector<QString>;
        strings->reserve(nb * 250);
    }

    sum += s.size();

#if 0
    static char buffer[1000];
    std::string s2 = s.toStdString();
    memcpy(buffer, s2.c_str(), s2.size());
    s.clear();
#endif

    *strings += s;
    current_nb = strings->size();
}

void gc_thread()
{
    cout << "gc_thread() started\n";

    size_t head = 0;

    while (head != nb * 250 && !quit)
    {
        while (head < current_nb)
        {
            (*strings)[head].clear();
            ++head;
            //QThread::yieldCurrentThread();
        }
        usleep(1000 * 100);
    }

    cout << "gc_thread() finished\n";
}

void f()
{
    for (unsigned char n = 0; n < 250; ++n)
        Log(getNumberName(n));
        //qDebug() << getNumberName(n);
}

void myMessageOutput(QtMsgType, const char *)
{
}

inline bool log_if_test(const QString & s) { return !s.isEmpty(); }
inline bool log_if_test(bool expr) { return expr; }

#define log_if(exp, level)\
    if (log_if_test(expr)) { log(level, #expr ": %1")

inline bool test_is_empty (const QString &);
inline bool test_is_empty (const std::string &);

#define check_empty(msg) \
    if (test_is_empty(msg)) log_error("Not empty: %1") << msg

inline bool log_if_not(bool cond)

#define verify(expr)\
    log_if_not(expr, &current_scope_details)

#define log_fail(statement)\
    if (0) else { log_error(); statement; }

std::string getText(HWND hWnd)
{
    char buffer[100];
    int err = GetWindowText(hWnd, buffer, 100);
    check_win32(err, return "");

    return buffer;
}

void f()
{
    int errCode = 0;
    QString errMsg;
    check_empty(errMsg);

    verify(errCode == 0) << errCode << errMsg;

    log_scope();

    char *buffer = malloc(100);
    log_if(!buffer) << "echec malloc";

    if (!buffer) { log_error("echec malloc"); return; }
    check_malloc(buffer, return);

    if (!buffer) log_fail(return);

    if (!buffer) log_return("malloc failed", return);

    bool ret = false;

    log_if(!ret) << ret;

    return ret;

    log_if(!errMsg.isEmpty()) << errMsg;

}

int main()
{
    qInstallMsgHandler(&myMessageOutput);

    QtConcurrent::run(&gc_thread);
    //sleep(1);

    struct timeval tv1, tv2;
    struct timezone tzp;
    gettimeofday(&tv1, &tzp);

    for (int i = 0; i < nb; ++i)
        f();

    gettimeofday(&tv2, &tzp);

    int sec = tv2.tv_sec - tv1.tv_sec;
    int usec = tv2.tv_usec - tv1.tv_usec;

    cout << (sec * 1000) + (usec / 1000) << " msec\n";

    sleep(1);
    quit = true;
    return sum;
}
