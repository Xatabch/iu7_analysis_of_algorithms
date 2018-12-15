//
//  main.cpp
//  mult_matrix
//
//  Created by Иван Морозов on 31.10.2018.
//  Copyright © 2018 Ivan Morozov. All rights reserved.
//

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
#include <vector>
#include <pthread.h>

typedef struct {
    std::vector<std::vector<int> > matrix1;
    std::vector<std::vector<int> > matrix2;
    std::vector<std::vector<int> > result;
    int M;
    int N1;
    int N2;
    int Q;
    
} mult_matrix;

void *first_half(void *args)
{
    mult_matrix *mult = (mult_matrix*) args;
    int half_m = (mult->M)/2;
    
    for( int i = 0; i < half_m; ++i )
    {
        for( int j = 0; j < mult->Q; ++j )
        {
            for( int k = 0; k < mult->N1; ++k )
            {
                mult->result[i][j] += mult->matrix1[i][k] * mult->matrix2[k][j];
            }
        }
    }
    return args;
}

void *second_half(void *args)
{
    mult_matrix *mult = (mult_matrix*) args;
    int half_m = (mult->M)/2;
    int max_m = mult->M;
    for( int i = half_m; i < max_m/*mult->M*/; ++i )
    {
        for( int j = 0; j < mult->Q; ++j )
        {
            for( int k = 0; k < mult->N1; ++k )
            {
                mult->result[i][j] += mult->matrix1[i][k] * mult->matrix2[k][j];
            }
        }
    }
    return args;
}

int multiply_matrix(mult_matrix *mult)
{
    pthread_t thread[3];
    int errflag;
    
    errflag = pthread_create(&thread[0], NULL, first_half, mult);
    if(errflag != 0) std::cout << "First thread drop" << std::endl;
    errflag = pthread_create(&thread[1], NULL, second_half, mult);
    if(errflag != 0) std::cout << "Second thread drop" << std::endl;
    
    errflag = pthread_join(thread[0], NULL);
    if(errflag != 0) std::cout << "First thread can't stop" << std::endl;
    errflag = pthread_join(thread[1], NULL);
    if(errflag != 0) std::cout << "Second thread can't stop" << std::endl;
    
    return errflag;
}

int main(int argc, const char * argv[]) {
    
    int M = 3;
    int N1 = 3;
    int N2 = 3;
    int Q = 3;
    int n = 3;
    std::cin >> n;
    M = N1 = N2 = Q = n;
    
    mult_matrix mult;
    
    mult.M = M;
    mult.N1 = N1;
    mult.N2 = N2;
    mult.Q = Q;
    
    mult.matrix1 = std::vector<std::vector<int> > (mult.M);
    int k = 1;
    
    for( int i = 0; i < mult.M; ++i )
    {
        mult.matrix1[i].resize(mult.N1);
        for( int j = 0; j < mult.N1; ++j )
        {
            mult.matrix1[i][j] = k;
            k++;
        }
    }
    
    mult.matrix2 = std::vector<std::vector<int> > (mult.N2);
    k = 1;
    
    for( int i = 0; i < mult.N2; ++i )
    {
        mult.matrix2[i].resize(mult.Q);
        for( int j = 0; j < mult.Q; ++j )
        {
            mult.matrix2[i][j] = k;
            k++;
        }
    }
    
    mult.result = std::vector<std::vector<int> > (mult.M);
    
    for( int i = 0; i < mult.M; ++i )
    {
        mult.result[i].resize(mult.Q);
        for( int j = 0; j < mult.Q; ++j )
        {
            mult.result[i][j] = 0;
        }
    }
    
    double middle = 0;
    double startTime;
    double endTime;
    
    for(int i = 0; i < 100; ++i) {
        startTime = getCPUTime( );
        multiply_matrix(&mult);
        endTime = getCPUTime( );
        middle += (endTime - startTime);
    }

    std::cout << middle/100 << std::endl;
    
    
//    std::cout << "OK";
//    for( int i = 0; i < mult.M; ++i )
//    {
//        for( int j = 0; j < mult.Q; ++j )
//        {
//            std::cout << mult.result[i][j] << " ";
//        }
//        std::cout << std::endl;
//    }
 
    return EXIT_SUCCESS;
}
