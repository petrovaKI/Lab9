// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#ifndef INCLUDE_QUEUE_HPP_
#define INCLUDE_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <iostream>
#include <string>

class Queue{
 public:
  //Добавляет элемент в конец queue.
  void push(std::string& str){
    std::lock_guard<std::mutex> lk{mtx};
    queue_.push(str);
  }
  //Возвращает ссылку на первый элемент в начале queue.
  std::string front(){
    std::lock_guard<std::mutex> lk{mtx};
    return queue_.front();
  }
  //Удаляет элемент из начала queue.
  void pop(){
    std::lock_guard<std::mutex> lk{mtx};
    queue_.pop();
  }
  //Проверяет, является ли queue пустым.
  bool empty(){
    std::lock_guard<std::mutex> lk{mtx};
    return queue_.empty() ? 1 : 0;
  }

 private:
  std::mutex mtx;
  std::queue<std::string> queue_;
};

Queue queues_;
Queue queues_pict;

#endif  // INCLUDE_QUEUE_HPP_
