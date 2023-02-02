#include "program.h"

#include "bp-tree/b_plus_tree.h"
#include "console.h"
#include "hashtable/hash_table.h"
#include "rb-tree/self_balancing_binary_search_tree.h"

namespace s21 {

int Program::Exec() {
  int mode = Console::ReadInt(
      "Enter mode: [1 - HashTable, 2 - B+ Tree, 3 - RB Tree]\n> ");

  if (mode == 1) {
    int capacity = Console::ReadInt("Enter HashTable capacity:\n> ");
    storage_ = new HashTable(capacity < 1 ? 1 : capacity);
  } else if (mode == 2) {
    storage_ = new BPlusTree();
  } else {
    storage_ = new SelfBalancingBinarySearchTree();
  }

  Console::WriteLine("> Ready to use");

  while (true) {
    std::vector<std::string> tokens = Console::ReadTokens("> ");
    std::string command = ToUpper(tokens[0]);

    if (command == "Q" || command == "QUIT") {
      break;
    } else if (command == "SET") {
      ProceedSet(tokens);
    } else if (command == "GET") {
      ProceedGet(tokens);
    } else if (command == "EXISTS") {
      ProceedExists(tokens);
    } else if (command == "DEL") {
      ProceedDel(tokens);
    } else if (command == "UPDATE") {
      ProceedUpdate(tokens);
    } else if (command == "KEYS") {
      ProceedKeys(tokens);
    } else if (command == "RENAME") {
      ProceedRename(tokens);
    } else if (command == "TTL") {
      ProceedTtl(tokens);
    } else if (command == "FIND") {
      ProceedFind(tokens);
    } else if (command == "SHOWALL") {
      ProceedShowAll(tokens);
    } else if (command == "UPLOAD") {
      ProceedUpload(tokens);
    } else if (command == "EXPORT") {
      ProceedExport(tokens);
    }
  }

  return 0;
}

std::string Program::ToUpper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](const char& c) { return std::toupper(c); });
  return s;
}

bool Program::IsNumber(std::string s) {
  return std::all_of(s.begin(), s.end(), ::isdigit);
}

void Program::ProceedSet(const std::vector<std::string>& tokens) {
  if (tokens.size() < 7) {
    Console::Error("invalid input");
    return;
  }

  V value{tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]};

  if (!IsNumber(value.birthday) || !IsNumber(value.coins)) {
    Console::Error("invalid input");
    return;
  }

  bool status = false;
  if (tokens.size() == 9 && ToUpper(tokens[7]) == "EX") {
    if (!IsNumber(tokens[8]))
      Console::Error("invalid input");
    else
      status = storage_->Set(tokens[1], value, stoi(tokens[8]));
  } else {
    status = storage_->Set(tokens[1], value);
  }

  if (status == 1)
    Console::WriteLine("> OK");
  else
    Console::Error("key exists");
}

void Program::ProceedGet(const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    Console::Error("invalid input");
    return;
  }

  V value = storage_->Get(tokens[1]);

  if (!value.birthday.empty()) {
    std::stringstream stream;
    stream << "> " << value.last_name << " " << value.first_name << " "
           << value.birthday << " " << value.city << " " << value.coins;
    Console::WriteLine(stream.str());
  } else {
    Console::WriteLine("> (null)");
  }
}

void Program::ProceedExists(const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    Console::Error("invalid input");
    return;
  }

  std::string result = storage_->Exists(tokens[1]) ? "true" : "false";
  Console::WriteLine("> " + result);
}

void Program::ProceedDel(const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    Console::Error("invalid input");
    return;
  }

  std::string result = storage_->Delete(tokens[1]) ? "true" : "false";
  Console::WriteLine("> " + result);
}

void Program::ProceedUpdate(const std::vector<std::string>& tokens) {
  if (tokens.size() != 7) {
    Console::Error("invalid input");
    return;
  }

  V value{tokens[2], tokens[3], tokens[4], tokens[5], tokens[6]};

  if (!(IsNumber(value.birthday) || value.birthday == "-") ||
      !(IsNumber(value.coins) || value.coins == "-")) {
    Console::Error("invalid input");
    return;
  }

  if (storage_->Update(tokens[1], value)) {
    Console::WriteLine("> OK");
  }
}

void Program::ProceedKeys(const std::vector<std::string>& tokens) {
  auto keys = storage_->Keys();
  if (keys.empty()) {
    Console::WriteLine("> Empty");
    return;
  }

  for (int i = 0; i < keys.size(); ++i)
    Console::WriteLine(std::to_string(i + 1) + ") " + keys[i]);
}

void Program::ProceedRename(const std::vector<std::string>& tokens) {
  if (tokens.size() != 3) {
    Console::Error("invalid input");
    return;
  }

  if (storage_->Rename(tokens[1], tokens[2])) {
    Console::WriteLine("> OK");
  }
}

void Program::ProceedTtl(const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    Console::Error("invalid input");
    return;
  }

  if (storage_->Exists(tokens[1])) {
    int lifetime = storage_->Ttl(tokens[1]);
    Console::WriteLine(
        "> " + (lifetime > -1 ? std::to_string(lifetime) : "unlimited"));
  } else {
    Console::WriteLine("> (null)");
  }
}

void Program::ProceedFind(const std::vector<std::string>& tokens) {
  if (tokens.size() != 6) {
    Console::Error("invalid input");
    return;
  }

  V value{tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]};
  auto keys = storage_->Find(value);
  for (int i = 0; i < keys.size(); ++i)
    Console::WriteLine(std::to_string(i + 1) + ") " + keys[i]);
}

void Program::ProceedShowAll(const std::vector<std::string>& tokens) {
  Console::WriteLine("> # | Фамилия | Имя | Год | Город | Количество коинов |");
  auto values = storage_->ShowAll();
  for (int i = 0; i < values.size(); ++i) {
    std::stringstream stream;
    stream << i + 1 << ") " << values[i];
    Console::WriteLine(stream.str());
  }
}

void Program::ProceedUpload(const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    Console::Error("invalid input");
    return;
  }

  int number_of_lines = storage_->Upload(tokens[1]);
  Console::WriteLine("> OK " + std::to_string(number_of_lines));
}

void Program::ProceedExport(const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    Console::Error("invalid input");
    return;
  }

  int number_of_lines = storage_->Export(tokens[1]);
  Console::WriteLine("> OK " + std::to_string(number_of_lines));
}

}  // namespace s21
