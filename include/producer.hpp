// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef INCLUDE_PRODUCER_HPP_
#define INCLUDE_PRODUCER_HPP_

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <utility>
#include <queue>
#include <mutex>
#include <gumbo.h>
#include <future>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>


#include "queue.hpp"
#include "root_certificates.hpp"
#include "thread_pool.hpp"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class Producer{
 public:
  //В конструкторе задаём количество потоков для пула потоков
  explicit Producer(unsigned poolsCount):pools(poolsCount){}
  std::string download_url(std::string host, std::string target);
  //парсим хост url-адреса
  std::string parse_url_to_host(std::string url);
  //парсим остальную часть адреса
  std::string parse_url_to_target(std::string url);
  //ищем ссылки
  void search_for_links(GumboNode* node);
  void download_next();


  //future- это объект, который может извлекать значение
  // из некоторого объекта или функции поставщика,
  // правильно синхронизируя этот доступ, если он находится в разных потоках.
  std::vector<std::future<std::string>> urls;
  //Пул потоков
  ThreadPool pools;
  Queue parser_queue_;
};
#endif  // INCLUDE_PRODUCER_HPP_
