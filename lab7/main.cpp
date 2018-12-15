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
double getCPUTime() {
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
    if (getrusage(RUSAGE_SELF, &rusage) != -1)
      return (double) rusage.ru_utime.tv_sec +
          (double) rusage.ru_utime.tv_usec / 1000000.0;
  }
#endif

#if defined(_SC_CLK_TCK)
  {
    const double ticks = (double) sysconf(_SC_CLK_TCK);
    struct tms tms;
    if (times(&tms) != (clock_t) -1)
      return (double) tms.tms_utime / ticks;
  }
#endif

#if defined(CLOCKS_PER_SEC)
  {
    clock_t cl = clock();
    if (cl != (clock_t) -1)
      return (double) cl / (double) CLOCKS_PER_SEC;
  }
#endif

#endif

  return -1;      /* Failed. */
}

#include <iostream>
#include <vector>
#include <limits>
#include <cmath>
#include <fstream>
#include <random>

void СompSort(std::vector<int> &path, std::vector<double> &sort, size_t size) {
  double fakt = 1.2473309; // фактор уменьшения
  int step = size - 1;

  while (step >= 1) {
    for (int i = 0; i + step < size; ++i) {
      if (sort[i] > sort[i + step]) {
        std::swap(sort[i], sort[i + step]);
        std::swap(path[i], path[i + step]);
      }
    }
    step /= fakt;
  }
}

struct Ant {
  int num_path = 0;
  int start_town = 0;
  double distance = 0;
  std::vector<std::vector<double> > dpher;
  std::vector<int> path;
  std::vector<int> curPath;
};

void debug_output_ants(std::vector<Ant> ants) {
  for (int i = 0; i < ants.size(); ++i) {
    std::cout << "Ant: " << i << std::endl;
    std::cout << "Num Path: " << ants[i].num_path << std::endl;
    std::cout << "Start Town: " << ants[i].start_town << std::endl;
    std::cout << "Distance: " << ants[i].distance << std::endl;
    std::cout << "Cur paths: ";
    for (int j = 0; j < ants[i].curPath.size(); ++j) {
      std::cout << ants[i].curPath[j] << " ";
    }
    std::cout << std::endl;
    std::cout << "Path: ";
    for (int j = 0; j < ants[i].path.size(); ++j) {
      std::cout << ants[i].path[j] << " ";
    }
    std::cout << std::endl << std::endl;

  }
}

void debug_sum(std::vector<double> prob) {
  double sum = 0;
  for (int i = 0; i < prob.size(); ++i) {
    sum += prob[i];
  }

  std::cout << sum << std::endl;

}

void findBestTravel(std::vector<std::vector<double> > matrixGraph,
                    double alpha,
                    double beta,
                    double e,
                    double q,
                    int tmax) {

  // 3.Инициализация рёбер – присвоение видимости ηij и начальной концентрации феромона

  // Инициализировали область видимости
  std::vector<std::vector<double> > eta(matrixGraph.size());

  for (int i = 0; i < matrixGraph.size(); ++i) {
    eta[i].resize(matrixGraph.size());
    for (int j = 0; j < matrixGraph[i].size(); ++j) {
      (matrixGraph[i][j] != 0) ? (eta[i][j] = 1 / matrixGraph[i][j]) : eta[i][j] = 0;
    }
  }

  // Инициализировали феромоны единицами для "равновероятнотного" начала
  std::vector<std::vector<double> > pheromones(matrixGraph.size());
  for (int i = 0; i < matrixGraph.size(); ++i) {
    pheromones[i].resize(matrixGraph.size());
    for (int j = 0; j < matrixGraph[i].size(); ++j) {
      pheromones[i][j] = 1;
    }
  }

  // 4.Размещение муравьёв в случайно выбранные города без совпадений
  // Инициализируем массив муравьев
  std::vector<Ant> ants(matrixGraph.size());

  for (int i = 0; i < matrixGraph.size(); ++i) {
    Ant ant;
    ant.num_path = (int) matrixGraph.size();
    ant.start_town = (int) (i + 1);
    ant.curPath.push_back(ant.start_town);
    for (int j = 0; j < matrixGraph.size(); ++j) {
      if (j + 1 != i + 1) ant.path.push_back(j + 1);
    }
    ant.dpher.resize(matrixGraph.size());
    for (int j = 0; j < matrixGraph.size(); ++j) {
      ant.dpher[j].resize(matrixGraph.size());
      for (int k = 0; k < matrixGraph[i].size(); ++k) {
        ant.dpher[j][k] = 0;
      }
    }
    ants[i] = ant;
  }

  // 5.Выбор начального кратчайшего маршрута и определение L*
  double shortL = 1e5;
  std::vector<double> probabilities;

  // Основной цикл

  // 6.Цикл по времени жизни колонии t=1,tmax
  for (int t = 0; t < tmax; ++t) {
    // 7.Цикл по всем муравьям k=1,m
    for (int k = 0; k < ants.size(); ++k) {
      // 8.Построить маршрут Tk (t) по правилу (1) и рассчитать длину Lk (t)
      while (!ants[k].path.empty()) {
        int from = *(ants[k].curPath.end() - 1);

        // Посчитаем вероятности для каждого города
        for (int j = 0; j < ants[k].path.size(); ++j) {

          // Знаменатель вероятности
          double sum = 0;
          for (int i = 0; i < ants[k].path.size(); ++i) {
            sum += (std::pow(pheromones[from - 1][ants[k].path[i] - 1], alpha)
                * std::pow(eta[from - 1][ants[k].path[i] - 1], beta));
          }

          // вероятность
          int to = ants[k].path[j];
          double p = (std::pow(pheromones[from - 1][to - 1], alpha) * std::pow(eta[from - 1][to - 1], beta)) / sum;
          probabilities.push_back(p);
        }
        // Выюберем город в который пойдем
        double coin = ((double) rand() / (double) RAND_MAX);
        СompSort(ants[k].path, probabilities, probabilities.size());

        if (ants[k].path.size() != 1) {
          for (int i = 0; i < probabilities.size(); ++i) {
            if (probabilities[i] > coin) {
              ants[k].curPath.push_back(ants[k].path[i]); // поместил
              ants[k].path.erase(ants[k].path.begin() + i); // удалил
              ants[k].distance += matrixGraph[from - 1][*(ants[k].curPath.end() - 1) - 1];
              ants[k].dpher[from - 1][(ants[k].path[i]) - 1] = q / ants[k].distance;
              ants[k].dpher[(ants[k].path[i]) - 1][from - 1] = q / ants[k].distance;
              break;
            }
          }
        } else {
          ants[k].curPath.push_back(ants[k].path[0]); // поместил
          ants[k].path.erase(ants[k].path.begin()); // удалил
          ants[k].distance += matrixGraph[from - 1][(ants[k].path[0]) - 1];
          ants[k].dpher[from - 1][(ants[k].path[0]) - 1] = q / ants[k].distance;
          ants[k].dpher[(ants[k].path[0]) - 1][from - 1] = q / ants[k].distance;
        }

        //Очистка данных о вероятностях
        probabilities.clear();
      }

      // Замыкаем путь
      int from = *(ants[k].curPath.end() - 1);
      int to = *(ants[k].curPath.begin());
      ants[k].distance += matrixGraph[from - 1][to - 1];
      ants[k].dpher[from - 1][to - 1] = q / ants[k].distance;
      ants[k].dpher[to - 1][from - 1] = q / ants[k].distance;
    }

    debug_output_ants(ants);
    // Далее обновляем наш минимум
    for (int j = 0; j < ants.size(); ++j) {
      // Если минимальный путь нашелся - выводим муравья нашедшего этот путь, дистанцию, которую он наше,
      // а так же сам путь
      if (shortL > ants[j].distance) {
        std::cout << "New best way find by ant: " << j + 1 << " its distance: " << ants[j].distance << " ";
        std::cout << "way: ";
        for (int k = 0; k < ants[j].curPath.size(); ++k) {
          std::cout << ants[j].curPath[k] << " ";
        }
        std::cout << std::endl;
        shortL = ants[j].distance;
      }
    }

    //Обновляем феромоны
    for (int j = 0; j < pheromones.size(); ++j) {
      for (int k = 0; k < pheromones[j].size(); ++k) {
        double sum = 0;
        for (int z = 0; z < ants.size(); ++z) {
          sum += ants[z].dpher[j][k];
        }

        pheromones[j][k] = (1 - e) * pheromones[j][k] + sum;
      }
    }
    ants.clear();
    ants.resize(matrixGraph.size());

    for (int i = 0; i < matrixGraph.size(); ++i) {
      Ant ant;
      ant.num_path = (int) matrixGraph.size();
      ant.start_town = (int) (i + 1);
      ant.curPath.push_back(ant.start_town);
      for (int j = 0; j < matrixGraph.size(); ++j) {
        if (j + 1 != i + 1) ant.path.push_back(j + 1);
      }
      ant.dpher.resize(matrixGraph.size());
      for (int j = 0; j < matrixGraph.size(); ++j) {
        ant.dpher[j].resize(matrixGraph.size());
        for (int k = 0; k < matrixGraph[i].size(); ++k) {
          ant.dpher[j][k] = 0;
        }
      }
      ants[i] = ant;
    }
  }
}

int main(int argc, const char *argv[]) {
  int n = 0;

  std::ifstream fin("test1.txt");

  fin >> n;

  // 1.Ввод матрицы расстояний D
  std::vector<std::vector<double> > matrixGraph(n);
  matrixGraph.resize(n);
  for (size_t i = 0; i < n; ++i) {
    matrixGraph[i].resize(n);
    for (size_t j = 0; j < n; ++j) {
      fin >> matrixGraph[i][j];
    }
  }


  // 2.Инициализация параметров алгоритма – α,β,e,Q
  findBestTravel(matrixGraph, 1.9, 0, 1, 5, 100);

  return 0;
}