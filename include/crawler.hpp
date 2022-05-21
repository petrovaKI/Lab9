// Copyright 2022 Petrova Kseniya <petrovaKI>

#ifndef TEMPLATE_CRAWLER_HPP
#define TEMPLATE_CRAWLER_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "consumer.hpp"
#include "producer.hpp"

namespace po = boost::program_options;

void crawler(std::string url, unsigned depth, Consumer& k, Producer &p,
             std::string outp, Queue &parser_queue);

#endif  // TEMPLATE_CRAWLER_HPP
