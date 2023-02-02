#ifndef A6_SRC_MAIN_COMMON_PERSON_H_
#define A6_SRC_MAIN_COMMON_PERSON_H_

#include <iomanip>

namespace s21 {

struct Person {
  std::string last_name;
  std::string first_name;
  std::string birthday;
  std::string city;
  std::string coins;

  bool operator==(const Person& other) const {
    return (city == other.city || other.city == "-") &&
           (last_name == other.last_name || other.last_name == "-") &&
           (first_name == other.first_name || other.first_name == "-") &&
           (birthday == other.birthday || other.birthday == "-") &&
           (coins == other.coins || other.coins == "-");
  }

  Person& operator=(const Person& other) {
    city = (other.city == "-") ? city : other.city;
    last_name = (other.last_name == "-") ? last_name : other.last_name;
    first_name = (other.first_name == "-") ? first_name : other.first_name;
    birthday = (other.birthday == "-") ? birthday : other.birthday;
    coins = (other.coins == "-") ? coins : other.coins;
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& stream, const Person& data) {
    stream << std::quoted(data.last_name) << " "   // "LastName"
           << std::quoted(data.first_name) << " "  // "FirstName"
           << data.birthday << " "                 //  1999
           << std::quoted(data.city) << " "        // "Moscow"
           << data.coins;                          //  21
    return stream;
  }

  friend std::istream& operator>>(std::istream& stream, Person& data) {
    stream >> std::quoted(data.last_name)  // "LastName"
        >> std::quoted(data.first_name)    // "FirstName"
        >> data.birthday                   //  1999
        >> std::quoted(data.city)          // "Moscow"
        >> data.coins;                     //  21
    return stream;
  }
};

}  // namespace s21

#endif  // A6_SRC_MAIN_COMMON_PERSON_H_
