// Copyright 2022 Petrova Kseniya <petrovaKI>
#include "producer.hpp"

//скачиваем все ссылки
std::string Producer::download_url(std::string host_, std::string target_){
  try {
    auto const host = host_.c_str();
    auto const port = "443";
    auto const target = target_.c_str();
    int version = 11;

    boost::asio::io_context ioc;
    //Перед созданием зашифрованного потока приложение должно
    //создать объект контекста SSL. Этот объект используется
    // для установки параметров SSL, таких как режим проверки,
    // файлы сертификатов и так далее
    ssl::context ctx{ssl::context::sslv23_client};
    load_root_certificates(ctx);

    // Эти объекты выполняют наш ввод-вывод
    tcp::resolver resolver{ioc};
    ssl::stream<tcp::socket> stream{ioc, ctx};
    // Установить имя хоста SNI (многим хостам это
    // необходимо для успешного соединения)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
      boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                   boost::asio::error::get_ssl_category()};
      throw boost::system::system_error{ec};
    }
    // Ищем доменное имя
    auto const results = resolver.resolve(host, port);
    // Установливаем соединение по IP-адресу, который мы получаем из поиска
    boost::asio::connect(stream.next_layer(), results.begin(), results.end());
    //Устанавливаем SSL соединенение
    stream.handshake(ssl::stream_base::client);

    // Настройка HTTP-запроса на получение сообщения
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // Отправить HTTP-запрос на удаленный хост
    http::write(stream, req);
    // Этот буфер используется для чтения и должен быть сохранен
    boost::beast::flat_buffer buffer;
    // Объявляем контейнер для хранения ответа
    http::response<http::string_body> res;
    // Получаем HTTP-ответ
    http::read(stream, buffer, res);
    //закрываем поток
    boost::system::error_code ec;

    if (ec == boost::asio::error::eof) {
      ec.assign(0, ec.category());
    }
    if (ec)
      throw boost::system::system_error{ec};
    //---------СТАВИМ ОТВЕТ В ОЧЕРЕДЬ ДЛЯ ПАРСЕРА------
    parser_queue_.push(res.body());
    //-------------------------------------------------
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
  //доходим до конца хоста
  for (; pos < url_.size(); ++pos) {
    if ((url_[pos] == '/') || (url_[pos] == '?')) break;
  }
  //парсим всё, что после хоста
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
  //Структура, представляющая один атрибут в теге HTML.
  //чтобы указать URL (адресс ссылки), нам необходим атрибут href (Из HTML)
  GumboAttribute* href = nullptr;
  //v - данные узла
  //проверяем что сслыка - href
  if (node->v.element.tag == GUMBO_TAG_A &&(href =
                                                 gumbo_get_attribute
                                             (&node->v.element.attributes,
                                              "href"))) {
    std::string curr_str = href->value;
    if (curr_str.find("https:") == 0) {
      //заполняем вектор ссылками----------------------------------
      urls.push_back(download_pools_.enqueue(&Producer::download_url,
                                   this, parse_url_to_host(curr_str),
                                   parse_url_to_target(curr_str)));
      //----------------------------------------------------------
    }
  }
  //загружаем все дочерние ссылки
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}

void Producer::download_next() {
  unsigned counts = urls.size();
  for (unsigned i = 0; i < counts; ++i){
    //Структура вывода, содержащая результаты парсинга
    GumboOutput* out = gumbo_parse(urls[i].get().c_str());
    //root - указатель на корневой узел. Это тег,
    // который формирует корень документа.
    search_for_links(out->root);
    //освобождаем память, выделенную под парсинг
    gumbo_destroy_output(&kGumboDefaultOptions, out);
  }
}
