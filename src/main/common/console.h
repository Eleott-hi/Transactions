#ifndef A6_SRC_MAIN_COMMON_CONSOLE_H_
#define A6_SRC_MAIN_COMMON_CONSOLE_H_

#include <iostream>
#include <string>
#include <vector>

namespace s21 {

class Console {
 public:
  static void Write(const std::string& text) {
    std::cout << "\033[0;32m" << text << "\033[0m";
  }

  static void WriteLine(const std::string& text) { Write(text + "\n"); }

  static void Info(const std::string& message) {
    std::cout << "\033[0;36m" << message << "\033[0m";
  }

  static int ReadInt(const std::string& message) {
    Info(message);
    int result;
    std::cin >> result;

    ClearInput();

    return result;
  }

  static void Error(const std::string& message) {
    std::cout << "\033[0;31m[ERROR]\033[0m - " << message << std::endl;
  }

  static std::string ReadLine(const std::string& message) {
    Info(message);
    std::string result;
    std::getline(std::cin, result);
    return result;
  }

  static std::vector<std::string> ReadTokens(const std::string& message) {
    std::string line = ReadLine(message);

    std::vector<std::string> tokens;
    tokens.reserve(10);
    std::stringstream stream(line);
    std::string token;
    while (getline(stream, token, ' ')) tokens.push_back(token);

    return tokens;
  }

  static void ClearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
};

}  // namespace s21

#endif  // A6_SRC_MAIN_COMMON_CONSOLE_H_
