// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef INCLUDE_THREAD_POOL_HPP_
#define INCLUDE_THREAD_POOL_HPP_

//Пул потоков — это коллекция рабочих потоков,
//которые эффективно выполняют асинхронные обратные вызовы
// от имени приложения.
// Пул потоков в основном используется
// для сокращения числа потоков приложения
// и обеспечения управления рабочими потоками
//Пул потоков принимает задачи из очереди

//Thread Pool имеет очередь задач, из которой
//каждый поток достаёт новую задачу при условии,
//что очередь не пуста и поток свободен
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <utility>

class ThreadPool {
 public:
  explicit ThreadPool(size_t);
  template<class F, class... Args>
  //постановка в очередь
  auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type>;
  ~ThreadPool();
 private:
  //нужно отслеживать потоки, чтобы мы могли присоединиться к ним
  std::vector< std::thread > workers;
  // очередь задач
  std::queue< std::function<void()> > tasks;

  // синхронизация
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

// конструктор просто запускает некоторое количество рабочих потоков
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
  for (size_t i = 0; i < threads; ++i)
    workers.emplace_back(
        [this]
        {
          for (;;)
          {
            std::function<void()> task;

            {
              std::unique_lock<std::mutex> lock(this->queue_mutex);
              this->condition.wait(lock,
                                   [this]{ return this->stop ||
                                                   !this->tasks.empty(); });
              if (this->stop && this->tasks.empty())
                return;
              task = std::move(this->tasks.front());
              this->tasks.pop();
            }

            task();
          }
        });
}

// добавление нового рабочего элемента в пул
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared< std::packaged_task<return_type()> >(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex);

    // запрет на постановку в очередь после остановки пула
    if (stop)
      throw std::runtime_error("enqueue on stopped ThreadPool");

    tasks.emplace([task](){ (*task)(); });
  }
  condition.notify_one();
  return res;
}

// деструктор объединяет потоки
inline ThreadPool::~ThreadPool()
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for (std::thread &worker : workers)
    worker.join();
}

#endif  // INCLUDE_THREAD_POOL_HPP_
