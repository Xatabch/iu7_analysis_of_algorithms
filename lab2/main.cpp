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

#define INCORRECT_SIZES -1
#define OK 0


int multiplyMatrix(const std::vector<std::vector<int> > matrix1, const int M, const int N1,
                   const std::vector<std::vector<int> > matrix2, const int N2, const int Q, std::vector<std::vector<int> > *matrix) // сложность 13MNQ + 4MQ + 4M + 2
{
    if( N1 != N2 ) return INCORRECT_SIZES;
    
    for( int i = 0; i < M; ++i )
    {
        for( int j = 0; j < Q; ++j )
        {
            for( int k = 0; k < N1; ++k )
            {
                (*matrix)[i][j] = (*matrix)[i][j] + matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    
    return OK;
}

int vinogradMultiplyMartix(const std::vector<std::vector<int> > matrix1, const int M, const int N1,
                           const std::vector<std::vector<int> > matrix2, const int N2, const int Q, std::vector<std::vector<int> > *matrix)
{
    if( N1 != N2 ) return INCORRECT_SIZES;
    
    // 1. Вычисляем для каждой строки матрицы A суммы произведений пар соседних эл-тов MulH
    // Сложность: 15MN + 5M + 2
    std::vector<int> MulH(M, 0);
    
    for( int i = 0; i < M; ++i )
    {
        for( int k = 0; k < N1/2; ++k )
        {
            MulH[i] = MulH[i] + matrix1[i][2*k] * matrix1[i][2*k+1];
        }
    }
    
    // 2. Вычисляем для каждого столбца матрицы B суммы произведений пар соседних эл-тов MulV
    // Сложность: 15MN + 5M + 2
    std::vector<int> MulV(Q, 0);
    
    for( int i = 0; i < Q; ++i )
    {
        for( int k = 0; k < N1/2; ++k )
        {
            MulV[i] = MulV[i] + matrix2[2*k][i] * matrix2[2*k+1][i];
        }
    }
    
    // 3. Результат
    // Сложность: 25/2MNQ + 12MQ + 4M + 2
    for( int i = 0; i < M; ++i )
    {
        for( int j = 0; j < Q; ++j )
        {
            (*matrix)[i][j] = -MulH[i] - MulV[j];
            for( int k = 0; k < N1/2; ++k )
            {
                (*matrix)[i][j] = (*matrix)[i][j] + ( matrix1[i][2*k] + matrix2[2*k+1][j] ) *
                ( matrix1[i][2*k+1] + matrix2[2*k][j] );
            }
        }
    }
    
    if( N1 % 2 == 1 )
    {
        for( int i = 0; i < M; ++i )
        {
            for( int j = 0; j < Q; ++j )
            {
                (*matrix)[i][j] = (*matrix)[i][j] + matrix1[i][N1-1] * matrix2[N1-1][j];
            }
        }
    }
    
    return OK;
}

int optimizeVinogradMultiplyMartix(const std::vector<std::vector<int> > matrix1, const int M, const int N1,
                                   const std::vector<std::vector<int> > matrix2, const int N2, const int Q, std::vector<std::vector<int> > *matrix)
{
    if( N1 != N2 ) return INCORRECT_SIZES;
    
    // 1-й тип оптимизации: хранение N1/2
    // 2-й тип оптимизации: вместо 2*k использую битовый сдвиг <<1
    // 3-й тип оптимизации:  MulH[i] +=
    
    int half_n = N1/2;
    
    // 1. Вычисляем для каждой строки матрицы A суммы произведений пар соседних эл-тов MulH
    std::vector<int> MulH(M, 0);
    
    for( int i = 0; i < M; ++i )
    {
        for( int k = 0; k < half_n; ++k )
        {
            MulH[i] += matrix1[i][(k<<1)] * matrix1[i][(k<<1)+1];
        }
    }

    for(int i = 0; i < M; ++i) std::cout << MulH[i] << " ";
    std::cout << std::endl;
    
    // 2. Вычисляем для каждого столбца матрицы B суммы произведений пар соседних эл-тов MulV
    std::vector<int> MulV(Q, 0);
    
    for( int i = 0; i < Q; ++i )
    {
        for( int k = 0; k < half_n; ++k )
        {
            MulV[i] += matrix2[k<<1][i] * matrix2[(k<<1)+1][i];
        }
    }

    for(int i = 0; i < Q; ++i) std::cout << MulV[i] << " ";
    std::cout << std::endl;
    
    // 3. Результат
    for( int i = 0; i < M; ++i )
    {
        for( int j = 0; j < Q; ++j )
        {
            (*matrix)[i][j] = -MulH[i] - MulV[j];
            for( int k = 0; k < half_n; ++k )
            {
                (*matrix)[i][j] += ( matrix1[i][k<<1] + matrix2[(k<<1)+1][j] ) *
                ( matrix1[i][(k<<1)+1] + matrix2[k<<1][j] );
            }
        }
    }
    
    if( N1 % 2 == 1 )
    {
        for( int i = 0; i < M; ++i )
        {
            for( int j = 0; j < Q; ++j )
            {
                (*matrix)[i][j] += matrix1[i][N1-1] * matrix2[N1-1][j];
            }
        }
    }
    
    return OK;
}

int main(void)
{
    int M = 3;
    int N1 = 3;
    int N2 = 3;
    int Q = 3;
    
    std::cin >> M;
    
    N1 = N2 = Q = M;
    
    int error = OK;
    double startTime;
    double endTime;
    
    std::vector<std::vector<int> > matrix1(M);
    int k = 1;
    
    for( int i = 0; i < M; ++i )
    {
        matrix1[i].resize(N1);
        for( int j = 0; j < N1; ++j )
        {
            matrix1[i][j] = k;
            k++;
        }
    }
    
    std::vector<std::vector<int> > matrix2(N2);
    k = 1;
    
    for( int i = 0; i < N2; ++i )
    {
        matrix2[i].resize(Q);
        for( int j = 0; j < Q; ++j )
        {
            matrix2[i][j] = k;
            k++;
        }
    }
    
    std::vector<std::vector<int> > matrix(M);
    
    for( int i = 0; i < M; ++i )
    {
        matrix[i].resize(Q);
        for( int j = 0; j < Q; ++j )
        {
            matrix[i][j] = 0;
        }
    }
    
    error = optimizeVinogradMultiplyMartix(matrix1, M, N1, matrix2, N2, Q, &matrix);

    // for( int i = 0; i < M; ++i )
    // {
    //     for( int j = 0; j < Q; ++j )
    //     {
    //         std::cout << matrix[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // double middle = 0;
    // for(int i = 0; i < 100; ++i)
    // {
    //     startTime = getCPUTime( );
    //     error = multiplyMatrix(matrix1, M, N1, matrix2, N2, Q, &matrix);
    //     endTime = getCPUTime( );
    //     middle += (endTime - startTime);
    // }
    
    // std::cout << middle/100 << std::endl;

    // middle = 0;
    // for(int i = 0; i < 100; ++i)
    // {
    //     startTime = getCPUTime( );
    //     error = vinogradMultiplyMartix(matrix1, M, N1, matrix2, N2, Q, &matrix);
    //     endTime = getCPUTime( );
    //     middle += (endTime - startTime);
    // }
    
    // std::cout << middle/100 << std::endl;
    // middle = 0;
    // for(int i = 0; i < 100; ++i)
    // {
    //     startTime = getCPUTime( );
    // 	error = optimizeVinogradMultiplyMartix(matrix1, M, N1, matrix2, N2, Q, &matrix);
    //     endTime = getCPUTime( );
    //     middle += (endTime - startTime);
    // }
    
    // std::cout << middle/100 << std::endl;
    
    // if(!error)
    // {
    //     for(int i = 0; i < M; ++i)
    //     {
    //         for(int j = 0; j < Q; ++j)
    //         {
    //             std::cout << matrix[i][j] << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }
    // else
    // {
    //     std::cout << "Mult error" << std::endl;
    // }
    
    return 0;
}
