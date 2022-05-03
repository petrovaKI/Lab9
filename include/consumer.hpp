// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#ifndef INCLUDE_CONSUMER_HPP_
#define INCLUDE_CONSUMER_HPP_

#include "parametrs.hpp"
#include "thread_pool.hpp"
#include "queue.hpp"
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <gumbo.h>
#include <string>

class Consumer{
 public:
  explicit Consumer(unsigned poolsCount):pools(poolsCount){}
  void parsing();
  void writing();
  void search_for_links(GumboNode* node);
 private:
  ThreadPool pools;
  ThreadPool pools_outp{1};
};

void Consumer::writing(){
  std::ofstream fout;
  //открываем файл
  fout.open(p.output);
  while (true){
    //если нет очереди из картинок
    if (queues_pict.empty()){
      sleep(4); //засыпаем
      if (queues_pict.empty()) //проверяем очередь ещё раз
        break;
      else
        continue;
    }
    fout << queues_pict.front() << std::endl; //выводим в файл
    queues_pict.pop(); //снимаем задачу
  }
  fout.close();
}

void Consumer::parsing() {
  //ставим в очередь задачу
  pools_outp.enqueue(&Consumer::writing, this);
  while (true) {
    if (queues_.empty()) {
      sleep(4);
      if (queues_.empty())
        break;
      else
        continue;
    }
    pools.enqueue(&Consumer::search_for_links, this,
                  gumbo_parse(queues_.front().c_str())->root);
    queues_.pop();
  }
}

//из документации - Находит URL-адреса всех ссылок на странице
void Consumer::search_for_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* src = nullptr;
  //проверяем что это изображение
  if (node->v.element.tag == GUMBO_TAG_IMG &&
      (src = gumbo_get_attribute(&node->v.element.attributes, "src"))) {
    std::string str = src->value;
    queues_pict.push(str);
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*> (children->data[i]));
  }
}

#endif // INCLUDE_CONSUMER_HPP_
