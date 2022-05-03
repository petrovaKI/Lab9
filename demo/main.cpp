#include "producer.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <queue>

using boost::asio::ip::tcp;
using namespace boost::asio::ssl;
using namespace boost::beast::http;
namespace po = boost::program_options;
using namespace std;

void parse_cmdl(int argc,char* argv[]){
  //Мы начинаем с объявления всех разрешенных опций с помощью класса
  // options_description. add_options Метод этого класса возвращает
  // специальный прокси-объект, который определяет operator().
  // Вызовы этого оператора фактически объявляют параметры.
  // Параметрами являются имя опции, информация о значении и описание опции.
  // В этом примере первый параметр не имеет значения, а второй имеет значение типа int.

  po::options_description desc{"Options"};
  desc.add_options()
      ("url", po::value<std::string>())
          ("output", po::value<std::string>())
              ("depth",po::value<unsigned>())
                  ("network_threads",po::value<unsigned>())
                      ("parser_threads",po::value<unsigned>());

  //Класс variables_map можно использовать так же, как std::map,
  // за исключением того, что значения, хранящиеся там,
  // должны быть извлечены с помощью метода as.
  po::variables_map vm;
  //Затем вызовы для store, parse_command_line заставляют vm содержать
  // все параметры, найденные в командной строке.
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);


  if(vm.count("url"))
    p.url = vm["url"].as<std::string>();
  if(vm.count("output"))
    p.output = vm["output"].as<std::string>();
  if(vm.count("depth"))
    p.depth = vm["depth"].as<unsigned>();
  if(vm.count("network_threads"))
    p.network_threads = vm["network_threads"].as<unsigned>();
  if(vm.count("parser_threads"))
    p.parser_threads = vm["parser_threads"].as<unsigned>();
  std::cout << p.url << std::endl;
}

int main(int argc, char* argv[]){
  //./demo
  // --url https://yandex.ru/images/
  // --output /home/kseniya/lab-09-producer-consumer/Log.txt
  // --depth 1 --network_threads 1 --parser_threads 2
  std::queue<std::string> q;
  //Парсим командную строку
  parse_cmdl(argc,argv);

  Producer pr(p.network_threads);
  Consumer k(p.parser_threads);

  pr.producing(p.url,p.depth,k);
  return 0;
}