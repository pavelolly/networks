#pragma once

const char *REQ_HTML = 
        "GET / HTTP/1.1\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
        "Cache-Control: max-age=0\r\n"
        "Connection: keep-alive\r\n"
        "Host: pmk.tversu.ru\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36\r\n\r\n";


const char *REQ_RESET_CSS =
        "GET /css/reset.css HTTP/1.1\r\n"
        "Accept: text/css,*/*;q=0.1\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Host: pmk.tversu.ru\r\n"
        "Pragma: no-cache\r\n"
        "Referer: http://pmk.tversu.ru/\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36\r\n\r\n";

const char *REQ_GRID_CSS =
        "GET /css/grid.css HTTP/1.1\r\n"
        "Accept: text/css,*/*;q=0.1\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Host: pmk.tversu.ru\r\n"
        "Pragma: no-cache\r\n"
        "Referer: http://pmk.tversu.ru/\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36\r\n\r\n";

const char *REQ_STYLE_CSS =
        "GET /css/style.css HTTP/1.1\r\n"
        "Accept: text/css,*/*;q=0.1\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Host: pmk.tversu.ru\r\n"
        "Pragma: no-cache\r\n"
        "Referer: http://pmk.tversu.ru/\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36\r\n\r\n";