#include "OrderCache.h"
#include "lib/simdjson.h"
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static std::set<std::string> securityIds;

template <typename Iterator>
int adder(Iterator begin, Iterator end, OrderCache *cache) {

  for (auto item = begin; item != end; item++) {
    cache->addOrder(*item);
  }
  return 0;
}

void runParallelAdding(simdjson::dom::element *data, OrderCache *cache) {
  using JsonVector = std::vector<Order>;
  JsonVector vec_json;

  for (const auto &item : data->get_array()) {
    std::string ord_id{item["order_id"].get_string().value()};
    std::string sec_id{item["security_id"].get_string().value()};
    std::string transaction_type{item["side"].get_string().value()};
    std::string amount{item["quantity"].get_string().value()};
    std::string user{item["user"].get_string().value()};
    std::string company{item["company"].get_string().value()};
    vec_json.emplace_back(Order{ord_id, sec_id, transaction_type,
                          static_cast<unsigned>(std::atoll(amount.c_str())),
                          user, company});
    securityIds.emplace(sec_id);
  }

  std::future<int> result;
  if (vec_json.size() < 1000) {
    result = std::async(std::launch::async, adder<JsonVector::iterator>,
                        vec_json.begin(), vec_json.end(), cache);
  }

  auto middle = vec_json.size() / 2;
  result = std::async(std::launch::async, adder<JsonVector::iterator>,
                      vec_json.begin() + middle, vec_json.end(), cache);
  auto result1 = std::async(std::launch::async, adder<JsonVector::iterator>,
                            vec_json.begin(), vec_json.begin() + middle, cache);
  result1.wait();
  
  return result.wait();
}

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

  runParallelAdding(&json_data, &cache);

  for (auto &item : securityIds) {
    std::cout << item << " | ";
  }

  std::cout << "\n============================================================="
               "========================================\n";

  std::vector<std::future<unsigned>> results;
  if (argc == 3) {
    // second argument
    for (auto &item : securityIds) {
      results.push_back( std::async(std::launch::async, [&cache, item] { return cache.getMatchingSizeForSecurity(item);}));
    }
  }
  for(auto& result : results) {
    std::cout << result.get() << " | ";
  }

  return 0;
}
