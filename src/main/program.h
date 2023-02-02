#ifndef A6_SRC_MAIN_PROGRAM_H_
#define A6_SRC_MAIN_PROGRAM_H_

#include <string>
#include <vector>

#include "key_value_storage.h"

namespace s21 {

class Program {
 public:
  Program() = default;
  ~Program() {
    delete storage_;
    storage_ = nullptr;
  }
  Program(const Program&) = delete;
  Program(Program&&) = delete;
  void operator=(const Program&) = delete;
  void operator=(Program&&) = delete;

  int Exec();

 private:
  using V = KeyValueStorage::V;

  KeyValueStorage* storage_ = nullptr;

  std::string ToUpper(std::string s);
  bool IsNumber(std::string s);

  void ProceedSet(const std::vector<std::string>& tokens);
  void ProceedGet(const std::vector<std::string>& tokens);
  void ProceedExists(const std::vector<std::string>& tokens);
  void ProceedDel(const std::vector<std::string>& tokens);
  void ProceedUpdate(const std::vector<std::string>& tokens);
  void ProceedKeys(const std::vector<std::string>& tokens);
  void ProceedRename(const std::vector<std::string>& tokens);
  void ProceedTtl(const std::vector<std::string>& tokens);
  void ProceedFind(const std::vector<std::string>& tokens);
  void ProceedShowAll(const std::vector<std::string>& tokens);
  void ProceedUpload(const std::vector<std::string>& tokens);
  void ProceedExport(const std::vector<std::string>& tokens);
};

}  // namespace s21

#endif  // A6_SRC_MAIN_PROGRAM_H_
