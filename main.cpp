#include "OrderCache.h"
#include "lib/simdjson.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  using namespace std::string_literals;
  // Path to your JSON file
  std::string filename = argv[1];
  if (filename.empty())
    return 1;

  // Read the entire file into a string
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return 1;
  }
  std::string json_str((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

  // Parse the JSON string
  simdjson::dom::parser parser;
  simdjson::dom::element json_data;
  auto error = parser.parse(json_str).get(json_data);
  if (error) {
    std::cerr << "Failed to parse JSON: " << error << std::endl;
    return 1;
  }

  OrderCache cache;
  std::set<std::string> securityIds;

  // Iterate over the JSON array
  for (const auto &item : json_data.get_array()) {

    std::string ord_id{item["OrdId"].get_string().value()};
    std::string sec_id{item["SecId"].get_string().value()};
    std::string transaction_type{item["TransactionType"].get_string().value()};
    std::string amount{item["Amount"].get_string().value()};
    std::string user{item["User"].get_string().value()};
    std::string company{item["Company"].get_string().value()};
    cache.addOrder(Order{ord_id, sec_id, transaction_type,
                         static_cast<unsigned>(std::atoll(amount.c_str())),
                         user, company});
    securityIds.emplace(sec_id);
    if (argc == 4) {
      // thirdth argument is like verbose flag
      std::cout << "Order ID: " << ord_id << ", ";
      std::cout << "Security ID: " << sec_id << ", ";
      std::cout << "Transaction Type: " << transaction_type << ", ";
      std::cout << "Amount: " << amount << ", ";
      std::cout << "User: " << user << ", ";
      std::cout << "Company: " << company << std::endl;
    }
  }
  for (auto &item : securityIds) {
    std::cout << item << " | ";
  }

  std::cout << "\n============================================================="
               "========================================\n";

  if (argc == 3) {
    // second argument 
    for (auto &item : securityIds) {
      std::cout << cache.getMatchingSizeForSecurity(item) << " | ";
    }
    std::cout << '\n';
  }

  return 0;
}
