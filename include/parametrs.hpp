// Copyright 2022 Petrova Kseniya <ksyushki5@yandex.ru>
#ifndef INCLUDE_PARAMETRS_HPP_
#define INCLUDE_PARAMETRS_HPP_
#include <iostream>
#include <string>

struct Parametrs {
  std::string url;
  std::string output;
  unsigned depth;
  unsigned network_threads;
  unsigned parser_threads;
};
Parametrs p;
#endif  // INCLUDE_PARAMETRS_HPP_
