// Copyright 2022 Petrova Kseniya <petrovaKI>

#include "crawler.hpp"
#include "producer.hpp"
#include "consumer.hpp"

//основная функция
void crawler(std::string url_, unsigned depth_, Consumer &k, Producer &p,
             std::string outp, Queue &parser_queue){
  GumboOutput* out = gumbo_parse(p.download_url
                                 (p.parse_url_to_host(url_),
                                  p.parse_url_to_target(url_)).c_str());
  //подгружаем ссылки с начальной страницы
  p.search_for_links(out->root);
  gumbo_destroy_output(&kGumboDefaultOptions, out);
  //парсим их
  k.parsing(outp, parser_queue);
  depth_--;
  //переходим на следующую станицу, пока не достигнут предел глубины
  while (depth_ > 0){
    depth_--;
    p.download_next();
    k.parsing(outp, parser_queue);
  }
  std::cout << "DONE" << std::endl;
}
