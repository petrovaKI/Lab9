// Copyright 2022 Petrova Kseniya <petrovaKI>
#include "consumer.hpp"

//Находит URL-адреса всех ссылок-картинок на странице
void Consumer::search_for_img_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  //GumboAttribute - структура, представляющая один атрибут в теге HTML.
  // Это пара имя-значение, но также включает в себя информацию
  // о местоположении источника и исходном исходном тексте
  GumboAttribute* src = nullptr;
  //проверяем что это изображение
  if (node->v.element.tag == GUMBO_TAG_IMG &&
      (src = gumbo_get_attribute(&node->v.element.attributes, "src"))) {
    std::string str = src->value;
    //------СТАВИМ ССЫЛКУ НА КАРТИНКУ В ОЧЕРЕДЬ НА ЗАПИСЬ В ФАЙЛ-----
    writer_queue_.push(str);
    //---------------------------------------------------------------
  }
  //продолжаем поиск по дочерним узлам, пока не найдём все нужные ссылки
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_img_links(static_cast<GumboNode*> (children->data[i]));
  }
}
void Consumer::writing(std::string &outp){
  std::ofstream fout;
  //открываем файл
  fout.open(outp);
  while (true){
    //если нет очереди из картинок
    if (writer_queue_.empty()){
      sleep(4); //засыпаем
      if (writer_queue_.empty()) //проверяем очередь ещё раз
        break;
      else
        continue;
    }
    fout << writer_queue_.front() << std::endl; //выводим в файл
    writer_queue_.pop(); //снимаем задачу
  }
  fout.close();
}

void Consumer::parsing(std::string &outp, Queue &parser_queue) {
  //в пул потоков парсера забираем задачи из parser_queue
  while (true) {
    if (parser_queue.empty()) {
      sleep(4);
      if (parser_queue.empty())
        break;
      else
        continue;
    }
    //помещаем в пул все картинки
    pars_pools_.enqueue(&Consumer::search_for_img_links, this,
                       gumbo_parse(parser_queue.front().c_str())->root);
    //снимаем задачу
    parser_queue.pop();
  }
  outp_pools_.enqueue(&Consumer::writing, this, outp);
}
