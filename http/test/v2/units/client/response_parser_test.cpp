// Copyright (C) 2013 by Glyn Matthews
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iterator>
#include <gtest/gtest.h>
#include <boost/algorithm/string/trim.hpp>
#include "network/http/v2/client/response.hpp"
#include "network/http/v2/client/response_parser.hpp"

namespace http = network::http::v2;

namespace {
  const std::string input =
    "HTTP/1.0 200 OK\r\n"
    "Date: Wed, 11 Sep 2013 05:50:12 GMT\r\n"
    "Server: Apache/2.2.15 (Red Hat)\r\n"
    "Last-Modified: Fri, 28 Mar 2008 17:26:33 GMT\r\n"
    "ETag: \"240a0b-53a-449829a786440\"\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 1338\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Boost Software License - Version 1.0 - August 17th, 2003\n"
    "\n"
    "Permission is hereby granted, free of charge, to any person or organization\n"
    "obtaining a copy of the software and accompanying documentation covered by\n"
    "this license (the \"Software\") to use, reproduce, display, distribute,\n"
    "execute, and transmit the Software, and to prepare derivative works of the\n"
    "Software, and to permit third-parties to whom the Software is furnished to\n"
    "do so, all subject to the following:\n"
    "\n"
    "The copyright notices in the Software and this entire statement, including\n"
    "the above license grant, this restriction and the following disclaimer,\n"
    "must be included in all copies of the Software, in whole or in part, and\n"
    "all derivative works of the Software, unless such copies or derivative\n"
    "works are solely in the form of machine-executable object code generated by\n"
    "a source language processor.\n"
    "\n"
    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
    "FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT\n"
    "SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE\n"
    "FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,\n"
    "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
    "DEALINGS IN THE SOFTWARE.\n"
    ;

  template <typename Iterator>
  inline
  std::string trimmed_string(Iterator first, Iterator last) {
    return boost::trim_copy(std::string(first, last));
  }

  template <class Rng>
  inline
  std::string trimmed_string(const Rng &rng) {
    return trimmed_string(std::begin(rng), std::end(rng));
  }
} // namespace

TEST(response_parser_test, parse_version) {
  http::response_parser parser;

  boost::logic::tribool parsed_ok = false;
  boost::iterator_range<std::string::const_iterator> version;
  std::tie(parsed_ok, version) = parser.parse_until(http::response_parser::http_version_done, input);

  ASSERT_TRUE(parsed_ok);
  ASSERT_EQ(http::response_parser::http_version_done, parser.state());
  ASSERT_EQ("HTTP/1.0", trimmed_string(version));
}

TEST(response_parser_test, parse_status_code) {
  http::response_parser parser;

  boost::logic::tribool parsed_ok = false;
  boost::iterator_range<std::string::const_iterator> status;
  std::tie(parsed_ok, status) = parser.parse_until(http::response_parser::http_version_done, input);
  ASSERT_TRUE(parsed_ok);

  std::tie(parsed_ok, status) = parser.parse_until(http::response_parser::http_status_done,
                                                   boost::make_iterator_range(std::end(status),
                                                                              std::end(input)));
  ASSERT_TRUE(parsed_ok);
  ASSERT_EQ(http::response_parser::http_status_done, parser.state());
  ASSERT_EQ("200", trimmed_string(status));
}

TEST(response_parser_test, parse_status_message) {
  http::response_parser parser;

  boost::logic::tribool parsed_ok = false;
  boost::iterator_range<std::string::const_iterator> status;
  std::tie(parsed_ok, status) = parser.parse_until(http::response_parser::http_status_done, input);
  ASSERT_TRUE(parsed_ok);

  std::tie(parsed_ok, status) = parser.parse_until(http::response_parser::http_status_message_done,
                                                   boost::make_iterator_range(std::end(status),
                                                                              std::end(input)));
  ASSERT_TRUE(parsed_ok);
  ASSERT_EQ(http::response_parser::http_status_message_done, parser.state());
  ASSERT_EQ("OK", trimmed_string(status));
}

TEST(response_parser_test, parse_first_header) {
  http::response_parser parser;

  boost::logic::tribool parsed_ok = false;
  boost::iterator_range<std::string::const_iterator> header;
  std::tie(parsed_ok, header) = parser.parse_until(http::response_parser::http_status_message_done, input);
  ASSERT_TRUE(parsed_ok);

  std::tie(parsed_ok, header) = parser.parse_until(http::response_parser::http_header_colon,
                                                   boost::make_iterator_range(std::end(header),
                                                                              std::end(input)));
  ASSERT_TRUE(parsed_ok);
  ASSERT_EQ(http::response_parser::http_header_colon, parser.state());
  ASSERT_EQ("Date:", trimmed_string(header));

  std::tie(parsed_ok, header) = parser.parse_until(http::response_parser::http_header_line_done,
                                                   boost::make_iterator_range(std::end(header),
                                                                              std::end(input)));
  ASSERT_TRUE(parsed_ok);
  ASSERT_EQ(http::response_parser::http_header_line_done, parser.state());
  ASSERT_EQ("Wed, 11 Sep 2013 05:50:12 GMT", trimmed_string(header));
}

TEST(response_parser_test, parse_headers) {
  http::response_parser parser;

  boost::logic::tribool parsed_ok = false;
  boost::iterator_range<std::string::const_iterator> header;
  std::tie(parsed_ok, header) = parser.parse_until(http::response_parser::http_status_message_done, input);
  ASSERT_TRUE(parsed_ok);

  while (!header) {
    std::tie(parsed_ok, header) = parser.parse_until(http::response_parser::http_header_colon,
                                                     boost::make_iterator_range(std::end(header),
                                                                                std::end(input)));
    ASSERT_TRUE(parsed_ok);
    ASSERT_EQ(http::response_parser::http_header_colon, parser.state());

    std::tie(parsed_ok, header) = parser.parse_until(http::response_parser::http_header_colon,
                                                     boost::make_iterator_range(std::end(header),
                                                                                std::end(input)));
    ASSERT_TRUE(parsed_ok);
    ASSERT_EQ(http::response_parser::http_header_line_done, parser.state());
  }
}
