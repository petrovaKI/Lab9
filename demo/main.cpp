// Copyright 2022 Petrova Kseniya <petrovaKI>

#include "producer.hpp"
#include "consumer.hpp"
#include "crawler.hpp"
//Суть шаблона Producer-Consumer заключается в том,
// что каждый поставщик и потребитель работает в отдельном потоке.
// Поставщики помещают задачи в очередь,
// а не занятые потребители вытаскивают их и выполняют

using boost::asio::ip::tcp;
using namespace boost::asio::ssl;
using namespace boost::beast::http;
using namespace std;

struct Options {
  std::string url;
  std::string output;
  unsigned depth;
  unsigned network_threads;
  unsigned parser_threads;
};
Options opt;

void parse_cmdl(int argc,char *argv[]){
  //options_description - класс, объявляющий разрешённые опции
  // add_options - метод этого класса, который возвращает
  // специальный прокси-объект, определяющий operator().
  // Вызовы этого оператора фактически объявляют параметры.
  // Параметрами являются имя опции, информация о значении и описание опции.

  po::options_description desc{"Options"};
  desc.add_options()
      ("url", po::value<std::string>())
          ("output", po::value<std::string>())
              ("depth",po::value<unsigned>())
                  ("network_threads",po::value<unsigned>())
                      ("parser_threads",po::value<unsigned>());

  //Класс variables_map можно использовать так же, как std::map,
  //за исключением того, что значения, хранящиеся там,
  //должны быть извлечены с помощью метода as.
  po::variables_map vm;
  //Затем вызовы для store, parse_command_line заставляют vm содержать
  // все параметры, найденные в командной строке.
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);


  if(vm.count("url"))
    opt.url = vm["url"].as<std::string>();
  if(vm.count("output"))
    opt.output = vm["output"].as<std::string>();
  if(vm.count("depth"))
    opt.depth = vm["depth"].as<unsigned>();
  if(vm.count("network_threads"))
    opt.network_threads = vm["network_threads"].as<unsigned>();
  if(vm.count("parser_threads"))
    opt.parser_threads = vm["parser_threads"].as<unsigned>();
  std::cout << opt.url << std::endl;
}

int main(int argc, char* argv[]){
  //Парсим командную строку
  parse_cmdl(argc,argv);

  Producer p(opt.network_threads);
  Consumer k(opt.parser_threads);

  crawler(opt.url,opt.depth,k, p, opt.output, p.parser_queue_);
  return 0;
}
//./demo
// --url https://yandex.ru/images/
// --output /home/kseniya/lab-09-producer-consumer/urls.txt
// --depth 1 --network_threads 1 --parser_threads 2
//std::queue<std::string> q;
