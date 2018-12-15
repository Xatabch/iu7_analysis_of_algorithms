/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */
#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <time.h>

#else
#error "Unable to define getCPUTime( ) for an unknown OS."
#endif

/**
 * Returns the amount of CPU time used by the current process,
 * in seconds, or -1.0 if an error occurred.
 */
double getCPUTime( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    FILETIME createTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if ( GetProcessTimes( GetCurrentProcess( ),
                         &createTime, &exitTime, &kernelTime, &userTime ) != -1 )
    {
        SYSTEMTIME userSystemTime;
        if ( FileTimeToSystemTime( &userTime, &userSystemTime ) != -1 )
            return (double)userSystemTime.wHour * 3600.0 +
            (double)userSystemTime.wMinute * 60.0 +
            (double)userSystemTime.wSecond +
            (double)userSystemTime.wMilliseconds / 1000.0;
    }
    
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* AIX, BSD, Cygwin, HP-UX, Linux, OSX, and Solaris --------- */
    
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
    /* Prefer high-res POSIX timers, when available. */
    {
        clockid_t id;
        struct timespec ts;
#if _POSIX_CPUTIME > 0
        /* Clock ids vary by OS.  Query the id, if possible. */
        if ( clock_getcpuclockid( 0, &id ) == -1 )
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
        /* Use known clock id for AIX, Linux, or Solaris. */
            id = CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
        /* Use known clock id for BSD or HP-UX. */
        id = CLOCK_VIRTUAL;
#else
        id = (clockid_t)-1;
#endif
        if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
            return (double)ts.tv_sec +
            (double)ts.tv_nsec / 1000000000.0;
    }
#endif
    
#if defined(RUSAGE_SELF)
    {
        struct rusage rusage;
        if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
            return (double)rusage.ru_utime.tv_sec +
            (double)rusage.ru_utime.tv_usec / 1000000.0;
    }
#endif
    
#if defined(_SC_CLK_TCK)
    {
        const double ticks = (double)sysconf( _SC_CLK_TCK );
        struct tms tms;
        if ( times( &tms ) != (clock_t)-1 )
            return (double)tms.tms_utime / ticks;
    }
#endif
    
#if defined(CLOCKS_PER_SEC)
    {
        clock_t cl = clock( );
        if ( cl != (clock_t)-1 )
            return (double)cl / (double)CLOCKS_PER_SEC;
    }
#endif
    
#endif
    
    return -1;      /* Failed. */
}

#include <iostream>

double* getArray(int size, int type)
{
    double* array = new double[size];
    
    if(type == 1)
    {
        for(int i = 0; i < size; ++i)
        {
            array[i] = rand() % 100 + 1;
        }
    }
    else if(type == 2)
    {
        for(int i = 0; i < size; ++i)
        {
            array[i] = i;
        }
    }
    else if(type == 3)
    {
        for(int i = 0; i < size; ++i)
        {
            array[i] = size - i;
        }
    }
    else if(type == 4)
    {
        for(int i = 0; i < size; ++i)
        {
            array[i] = i;
        }
        
        for(int i = 0; i < size/2; ++i)
        {
            array[rand() % size + 1] = rand() % 100 + 1;
        }
    }
    
    return array;
}

void InsertionSort(double* a, int n) {
    for(int i = 1; i < n; ++i) {
        int temp = a[i];
        int j = i - 1;
        for(; j >= 0 && temp < a[j]; --j) {
            a[j+1] = a[j];
        }

        a[j+1] = temp;
    }
}

void СompSort(double* sort, int size)
{
    double fakt = 1.2473309; // фактор уменьшения
    int step = size - 1;
    
    while (step >= 1)
    {
        for (int i = 0; i + step < size; ++i)
        {
            if (sort[i] > sort[i + step])
            {
                std::swap(sort[i], sort[i + step]);
            }
        }
        step /= fakt;
    }
}

void heapify(double* arr, int n, int i)
{
    int largest = i;
    int l = 2*i + 1;
    int r = 2*i + 2;
    
    if (l < n && arr[l] > arr[largest])
        largest = l;
    
    if (r < n && arr[r] > arr[largest])
        largest = r;
    
    if (largest != i)
    {
        std::swap(arr[i], arr[largest]);
        
        heapify(arr, n, largest);
    }
}

void heapSort(double* arr, int n)
{
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);
    
    for (int i=n-1; i>=0; i--)
    {
        std::swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}

int main(int argc, const char * argv[]) {
    
    double startTime;
    double endTime;
    double middle = 0;
    
    // Рандомный массив вставки
    for(int j = 1000; j <= 10000; j+=1000)
    {
        double* a = getArray(j, 1);
        for(int i = 0; i < 100; ++i)
        {
            startTime = getCPUTime( );
            InsertionSort(a, j);
            endTime = getCPUTime( );
            middle += (endTime - startTime);
        }

        std::cout << j << " " << middle/100 << std::endl;
        middle = 0;
        delete[] a;
    }
//
//    // Упорядочен по возрастанию вставки
//    for(int j = 100; j <= 1000; j+=100)
//    {
//        double* a = getArray(j, 2);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            InsertionSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    // Упорядочен по убыванию вставки
//    for(int j = 100; j <= 1000; j+=100)
//    {
//        double* a = getArray(j, 3);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            InsertionSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    // Частично упорядочен вставки
//    for(int j = 100; j <= 1000; j+=100)
//    {
//        double* a = getArray(j, 4);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            InsertionSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    std::cout << std::endl;
//
    // Рандомный массив расческа
    for(int j = 1000; j <= 10000; j+=1000)
    {
        double* a = getArray(j, 1);
        for(int i = 0; i < 100; ++i)
        {
            startTime = getCPUTime( );
            СompSort(a, j);
            endTime = getCPUTime( );
            middle += (endTime - startTime);
        }

        std::cout << j << " " << middle/100 << std::endl;
        middle = 0;
        delete[] a;
    }
//
//    // Упорядочен по возрастанию расческа
//    for(int j = 100; j <= 1000; j+=100)
//    {
//        double* a = getArray(j, 2);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            СompSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    // Упорядочен по убыванию расческа
//    for(int j = 100; j <= 1000; j+=100)
//    {
//        double* a = getArray(j, 3);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            СompSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    // Частично упорядочен расческа
//    for(int j = 100; j <= 1000; j+=100)
//    {
//        double* a = getArray(j, 4);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            СompSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    std::cout << std::endl;
//
    // Рандомный куча
    for(int j = 1000; j <= 10000; j+=1000)
    {
        double* a = getArray(j, 1);
        for(int i = 0; i < 100; ++i)
        {
            startTime = getCPUTime( );
            heapSort(a, j);
            endTime = getCPUTime( );
            middle += (endTime - startTime);
        }

        std::cout << j << " " << middle/100 << std::endl;
        middle = 0;
        delete[] a;
    }

//    // Упорядочен по возрастанию куча
//    for(int j = 1000; j <= 10000; j+=1000)
//    {
//        double* a = getArray(j, 2);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            heapSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }
//
//    // Упорядочен по убыванию куча
//    for(int j = 1000; j <= 10000; j+=1000)
//    {
//        double* a = getArray(j, 3);
//        for(int i = 0; i < 100; ++i)
//        {
//            startTime = getCPUTime( );
//            heapSort(a, j);
//            endTime = getCPUTime( );
//            middle += (endTime - startTime);
//        }
//
//        std::cout << j << " " << middle/100 << std::endl;
//        middle = 0;
//        delete[] a;
//    }

    return 0;
}
