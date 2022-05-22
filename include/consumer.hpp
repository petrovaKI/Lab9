// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef INCLUDE_CONSUMER_HPP_
#define INCLUDE_CONSUMER_HPP_

#include "queue.hpp"
#include "thread_pool.hpp"

#include <fstream>
#include <unistd.h>
#include <iostream>
#include <gumbo.h>
#include <string>


class Consumer{
 public:
  explicit Consumer(unsigned poolsCount):pars_pools_(poolsCount){}
  void parsing(std::string &outp, Queue &parser_queue);
  void writing(std::string &outp);
  void search_for_img_links(GumboNode* node);
 private:
  //пул забирает задачи из очереди ссылок на парсинг
  ThreadPool pars_pools_;
  //пул забирает задачи из очереди картинок на запись (один поток)
  ThreadPool outp_pools_{1};
  Queue writer_queue_;
};
#endif // INCLUDE_CONSUMER_HPP_
