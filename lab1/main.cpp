//
//  main.cpp
//  lab1
//
//  Created by Иван Морозов on 23.09.2018.
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
#include <string>
#include <vector>
#include <regex>

using namespace std;

size_t min_three(const size_t d1, const size_t d2, const size_t d3)
{
    return (d1 < d2 ? (d1 < d3 ? d1 : d3) : (d2 < d3 ? d2 : d3));
}

size_t min_two(const size_t d1, const size_t d2)
{
    return (d1 < d2 ? d1 : d2);
}

size_t cyrillic(const size_t &i, bool is_cyrillic)
{
    return is_cyrillic ? (i*2)+1 : i;
}

size_t LevenshteinDistance(const std::string &s1, const std::string &s2, double *startTime, double *endTime)
{
    size_t m = s1.size();
    size_t n = s2.size();
    
    // check cyrillic
    bool is_cyrillic = 0;
    std::regex txt_regex("[a-zA-Z]");
    if(!regex_search(s1, txt_regex)) { m = m / 2; is_cyrillic = 1; }
    if(!regex_search(s2, txt_regex)) { n = n / 2; is_cyrillic = 1; }
    
    if( m==0 ) return n;
    if( n==0 ) return m;
    
    vector<vector<size_t> > matrix(n+1);
    
    *startTime = getCPUTime( );
    for(size_t i = 0; i < (n+1); i++)
    {
        matrix[i].resize(m+1);
        for(size_t j = 0; j < (m+1); j++)
        {
            matrix[i][j] = 0;
        }
    }
    
    for(size_t i = 0; i < (m+1); i++) matrix[0][i] = i;
    for(size_t i = 0; i < (n+1); i++) matrix[i][0] = i;
    
    for(size_t i = 1; i < (n+1); i++)
    {
        for(size_t j = 1; j < (m+1); j++)
        {
            matrix[i][j] = min_three(matrix[i][j-1] + 1, matrix[i-1][j] + 1, matrix[i-1][j-1] + (s1[cyrillic(j - 1, is_cyrillic)] == s2[cyrillic(i - 1, is_cyrillic)] ? 0 : 1));
        }
    }
    *endTime = getCPUTime( );
    
    cout << "Матрица: " << endl;
    for(size_t i = 0; i < (n+1); i++)
    {
        for(size_t j = 0; j < (m+1); j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << endl;
    }
    
    return matrix[n][m];
}

size_t D(const string &s1, const string &s2, size_t n,size_t m, bool is_cyrillic)
{
    if(n==0)
        return m;
    else
        if(m==0)
            return n;
        else
            return min_three(D(s1, s2, n-1, m, is_cyrillic) + 1, D(s1, s2, n, m-1, is_cyrillic) + 1, D(s1, s2, n-1, m-1, is_cyrillic)+(s1[cyrillic(m - 1, is_cyrillic)] == s2[cyrillic(n - 1, is_cyrillic)] ? 0 : 1));
}

size_t recursiveLivenshtein(const std::string &s1, const std::string &s2, double *startTime, double *endTime) //e
{
    size_t m = s1.size();
    size_t n = s2.size();
    
    // check cyrillic
    bool is_cyrillic = 0;
    std::regex txt_regex("[a-zA-Z]");
    if(!regex_search(s1, txt_regex)) { m = m / 2; is_cyrillic = 1; }
    if(!regex_search(s2, txt_regex)) { n = n / 2; is_cyrillic = 1; }
    
    if( m==0 ) return n;
    if( n==0 ) return m;
    
    vector<vector<size_t> > matrix(n+1);
    
    for(size_t i = 0; i < (n+1); i++)
    {
        matrix[i].resize(m+1);
        for(size_t j = 0; j < (m+1); j++)
        {
            matrix[i][j] = 0;
        }
    }
    
    for(size_t i = 0; i < (m+1); i++) matrix[0][i] = i;
    for(size_t i = 0; i < (n+1); i++) matrix[i][0] = i;
    
    size_t result = 0;
    
    *startTime = getCPUTime( );
    result = D(s1,s2,n,m,is_cyrillic);
    *endTime = getCPUTime( );
    
    return result;
}

void gen_random(char *s, const int len) {
    static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    s[len] = 0;
}

size_t DomerauLevenshteinDistance(const std::string &s1, const std::string &s2, double *startTime, double *endTime)
{
    size_t m = s1.size();
    size_t n = s2.size();
    size_t cost = 0;
    
    // check cyrillic
    bool is_cyrillic = 0;
    std::regex txt_regex("[a-zA-Z]");
    if(!regex_search(s1, txt_regex)) { m = m / 2; is_cyrillic = 1; }
    if(!regex_search(s2, txt_regex)) { n = n / 2; is_cyrillic = 1; }
    
    if( m==0 ) return n;
    if( n==0 ) return m;
    
    *startTime = getCPUTime( );
    std::vector<std::vector<size_t >> d(m+1);
    
    for(size_t i = 0; i <= m; i++)
    {
        d[i].resize(n+1);
        for(size_t j = 0; j <= n; j++)
        {
            d[i][j] = 0;
        }
    }
    
    for(int i = 0; i <= m; i++) d[i][0] = i;
    for(int j = 0; j <= n; j++) d[0][j] = j;
    
    
    for(int i = 1; i <= m; ++i)
    {
        for(int j = 1; j <= n; ++j)
        {
            if(s1[cyrillic(i, is_cyrillic)] == s2[cyrillic(j, is_cyrillic)])
                cost = 0;
            else
                cost = 1;
            
            d[i][j] = min_three(d[i-1][j] + 1, d[i][j-1] + 1, d[i-1][j-1] + cost);
            if(i > 1 && j > 1 && (s1[cyrillic(i, is_cyrillic)] == s2[cyrillic(j-1, is_cyrillic)]) && (s1[cyrillic(i-1, is_cyrillic)] == s2[cyrillic(j, is_cyrillic)]))
                d[i][j] = min_two(d[i][j], d[i-2][j-2] + cost);
        }
    }
    *endTime = getCPUTime( );
    
    cout << "Матрица:" << endl;
    for(size_t i = 0; i <= m; i++)
    {
        for(size_t j = 0; j <= m; j++)
        {
            std::cout << d[i][j] << " ";
        }
        std::cout << endl;
    }
    
    
    
    return d[m][n];
}

int main(int argc, const char * argv[]) {
    
//    char *s3 = new char [1001];
//    char *s4 = new char [1001];
    double startTime, endTime;
//    long double sum = 0.0;
//
//    for(int j = 100; j <= 1000; j += 100)
//    {
//        gen_random(s3, j);
//        gen_random(s4, j);
//        for(int i = 0; i < 100; i++)
//        {
//            LevenshteinDistance(s3, s4, &startTime, &endTime);
//            sum += (endTime - startTime);
//        }
//        cout << sum/100.0 << endl;
//        sum = 0;
//    }
//
//    std::cout << endl;
//
//    for(int j = 100; j <= 1000; j += 100)
//    {
//        gen_random(s3, j);
//        gen_random(s4, j);
//
//        for(int i = 0; i < 100; i++)
//        {
//            myDomerauLevenshteinDistance(s3, s4, &startTime, &endTime);
//            sum += (endTime - startTime);
//        }
//        cout << sum/100.0 << endl;
//        sum = 0;
//    }
    
    
    string s0;
    string s1;
    std::cout << "Введите перую строку: ";
    std::cin >> s0;
    std::cout << "Введите вторую строку: ";
    std::cin >> s1;
    std::cout << endl;

    size_t result1 = LevenshteinDistance(s0, s1, &startTime, &endTime);
    std::cout << "Левенштейн: " << result1 << endl << endl;
    size_t result2 = DomerauLevenshteinDistance(s0, s1, &startTime, &endTime);
    std::cout << "Домерау-Левенштейн: " << result2 << endl;
    size_t result3 = recursiveLivenshtein(s0, s1, &startTime, &endTime);
    std::cout << endl << endl << "Левенштейн(рекурсия): " << result3 << endl;
    
    
    
    return 0;
}
