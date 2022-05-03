// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>

#ifndef INCLUDE_PRODUCER_HPP_
#define INCLUDE_PRODUCER_HPP_

#include "thread_pool.hpp"
#include "queue.hpp"
#include "consumer.hpp"
#include "root_certificates.hpp"

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

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class Producer{
 public:
  //В конструкторе задаём количество для пула потоков
  explicit Producer(unsigned poolsCount):pools(poolsCount){}
  void producing(std::string url, unsigned depth, Consumer& k);
  std::string download_url(std::string host, std::string target);
  std::string parse_url_to_host(std::string url);
  std::string parse_url_to_target(std::string url);
  void search_for_links(GumboNode* node);
  void download_next();
 private:
  //Класс std::future представляет собой обертку,
  // над каким-либо значением или объектом(далее значением),
  // вычисление или получение которого происходит отложено.
  // Точнее, future предоставляет доступ к некоторому разделяемому
  // состоянию, которое состоит из 2-х частей: данные(здесь лежит
  // значение) и флаг готовности. future является получателем значения и
  // не может самостоятельно выставлять его.

  std::map <std::string, std::string> urls;
  std::vector<std::future<std::string>> fut;
  //Пул потоков
  ThreadPool pools;
};
void Producer::download_next() {
  unsigned counts = fut.size();
  for (unsigned i = 0; i < counts; ++i){
    GumboOutput* out = gumbo_parse(fut[i].get().c_str());
    search_for_links(out->root);
    gumbo_destroy_output(&kGumboDefaultOptions, out);
  }
  std::cout << "DONE" << std::endl;
}


void Producer::producing(std::string url_, unsigned depth_, Consumer& k){
  GumboOutput* out = gumbo_parse(download_url
                                 (parse_url_to_host(url_),
                                  parse_url_to_target(url_)).c_str());
  search_for_links(out->root);
  gumbo_destroy_output(&kGumboDefaultOptions, out);
  k.parsing();
  while (depth_ > 0){
    depth_--;
    download_next();
  }
}
std::string Producer::download_url(std::string host_, std::string target_){
  try {
    auto const host = host_.c_str();
    auto const port = "443";
    auto const target = target_.c_str();
    int version = 11;

    boost::asio::io_context ioc;

    ssl::context ctx{ssl::context::sslv23_client};

    load_root_certificates(ctx);

    tcp::resolver resolver{ioc};
    ssl::stream<tcp::socket> stream{ioc, ctx};

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
      boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                   boost::asio::error::get_ssl_category()};
      throw boost::system::system_error{ec};
    }

    auto const results = resolver.resolve(host, port);

    boost::asio::connect(stream.next_layer(), results.begin(), results.end());

    stream.handshake(ssl::stream_base::client);

    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    boost::beast::flat_buffer buffer;

    http::response<http::string_body> res;

    http::read(stream, buffer, res);

    boost::system::error_code ec;

    if (ec == boost::asio::error::eof) {
      ec.assign(0, ec.category());
    }
    if (ec)
      throw boost::system::system_error{ec};

    queues_.push(res.body());

    return res.body();
  }catch (std::exception const& e){
    std::cerr << e.what() << std::endl;
  }
  return "";
}
std::string Producer::parse_url_to_host(std::string url_){
  if (url_.find("https://") == 0)
    url_ = url_.substr(8);
  std::string result_host;
  for (unsigned i = 0; i < url_.size(); ++i) {
    if ((url_[i] == '/') || (url_[i] == '?')) break;
    result_host += url_[i];
  }
  return result_host;
}
std::string Producer::parse_url_to_target(std::string url_){
  if (url_.find("https:") == 0)
    url_ = url_.substr(8);
  std::string result_target;
  unsigned pos = 0;
  for (; pos < url_.size(); ++pos) {
    if ((url_[pos] == '/') || (url_[pos] == '?')) break;
  }
  for (unsigned i = pos; i < url_.size(); ++i) {
    result_target += url_[i];
  }

  return result_target;
}
//рекурсивный обход дерева
void Producer::search_for_links(GumboNode* node){
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href = nullptr;
  if (node->v.element.tag == GUMBO_TAG_A &&(href =
                                                 gumbo_get_attribute
                                             (&node->v.element.attributes,
                                              "href"))) {
    std::string curr_str = href->value;
    if (curr_str.find("https:") == 0) {
      unsigned count = urls.size();
      urls.insert(std::pair<std::string, std::string>(curr_str, "res"));
      if (urls.size() > count)
        fut.push_back(pools.enqueue(&Producer::download_url,
                                    this, parse_url_to_host(curr_str),
                                    parse_url_to_target(curr_str)));
    }
  }
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}
#endif  // INCLUDE_PRODUCER_HPP_
