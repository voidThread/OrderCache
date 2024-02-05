#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Order {

public:
  // do not alter signature of this constructor
  Order(const std::string &ordId, const std::string &secId,
        const std::string &side, const unsigned int qty,
        const std::string &user, const std::string &company)
      : m_orderId(ordId), m_securityId(secId), m_side(side), m_qty(qty),
        m_user(user), m_company(company) {}

  // do not alter these accessor methods
  std::string orderId() const { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const { return m_side; }
  std::string user() const { return m_user; }
  std::string company() const { return m_company; }
  unsigned int qty() const { return m_qty; }

private:
  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;    // unique order id
  std::string m_securityId; // security identifier
  std::string m_side;       // side of the order, eg Buy or Sell
  unsigned int m_qty;       // qty for this order
  std::string m_user;       // user name who owns this order
  std::string m_company;    // company for user
};

// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface {

public:
  // implememnt the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string &orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string &user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId,
                                                  unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int
  getMatchingSizeForSecurity(const std::string &securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;
  virtual ~OrderCacheInterface() = default;
};

class OrderCache : public OrderCacheInterface {
  // Now it's tight coupled with Order class, in next iterration this should be
  // extracted to define it in other place. Those types can be used among
  // classes
  using orderIdType = std::invoke_result_t<decltype(&Order::orderId), Order>;
  using userType = std::invoke_result_t<decltype(&Order::user), Order>;
  using securityIdType = std::invoke_result_t<decltype(&Order::user), Order>;

  // Insertin/deletion prefered
  using ordersList = std::list<Order>;
  using orderIterator = ordersList::iterator;
  using ordersIteratorsList = std::list<orderIterator>;
  // SecurityId buckets - one to many
  using securityIdCache =
      std::unordered_map<securityIdType, ordersIteratorsList>;
  // User buckets - one to many
  using userCache = std::unordered_map<userType, ordersIteratorsList>;
  // OrderId buckets - one to one
  using orderIdCache = std::unordered_map<orderIdType, orderIterator>;

  ordersList m_orders;
  orderIdCache m_ordersById;
  userCache m_ordersByUser;
  securityIdCache m_ordersBySecurity;

  const std::string buy_string{"buy"};
  const std::string sell_string{"sell"};

  std::string to_lower(std::string_view input) {
    std::string tolower;
    for (auto character : input) {
      tolower.push_back(static_cast<char>(std::tolower(character)));
    }
    return tolower;
  }

  mutable std::shared_mutex mutex;

public:
  virtual ~OrderCache() = default;

  void addOrder(Order order) override {

    auto validate_order = [this](const Order &order) {
      if (order.orderId().empty() || order.securityId().empty() ||
          order.user().empty() || order.qty() == 0 ||
          to_lower(order.side()).compare(buy_string) &&
              to_lower(order.side()).compare(sell_string)) {
        std::cerr << "Invalid data. Data not added\n";
        return 1;
      }
      return 0;
    };

    if (validate_order(order)) {
      return;
    }

    std::unique_lock<std::shared_mutex> lock(mutex);
    m_orders.emplace_back(order);

    auto last_element = std::prev(m_orders.end());

    auto emplace_data = [&](const Order &order, auto &&container, auto key,
                            auto value) {
      auto [iterator, result] = container.try_emplace(key, value);

      using T = std::decay_t<decltype(container)>;
      if constexpr (std::is_same_v<T, userCache> ||
                    std::is_same_v<T, securityIdCache>) {
        if (!result) {
          iterator->second.emplace_back(last_element);
        }
      } else if constexpr (std::is_same_v<T, orderIdCache>) {
        if (!result) {
          m_orders.pop_back();
          std::cerr << "Error while adding new order. Order exists.\n";
          return 1;
        }
      }
      return 0;
    };

    if (emplace_data(order, m_ordersById, order.orderId(), last_element)) {
      return;
    }
    emplace_data(order, m_ordersByUser, order.user(),
                 ordersIteratorsList{last_element});
    emplace_data(order, m_ordersBySecurity, order.securityId(),
                 ordersIteratorsList{last_element});
  }

  void cancelOrder(const std::string &orderId) override {
    std::unique_lock<std::shared_mutex> lock(mutex);
    try { // reconsidier using find instead catching an exception
      auto orderIterator = m_ordersById.at(orderId);
      auto user = orderIterator->user();
      auto securityId = orderIterator->securityId();
      m_ordersByUser.at(user).remove(orderIterator);
      m_ordersBySecurity.at(securityId).remove(orderIterator);
      m_orders.erase(orderIterator);
      m_ordersById.erase(orderId);
    } catch (const std::out_of_range &exception) {
      std::cerr << "There is no entry with specified order ID";
    }
  };

  void cancelOrdersForUser(const std::string &user) override {
    std::unique_lock<std::shared_mutex> lock(mutex);
    try {
      for (auto item : m_ordersByUser.at(user)) {
        m_ordersBySecurity.at(item->securityId()).remove(item);
        m_ordersById.erase(item->orderId());
        m_orders.erase(item);
      }
      m_ordersByUser.erase(user);
    } catch (const std::out_of_range &exception) {
      std::cerr << "There is no entry with provided user";
    }
  };

  void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId,
                                          unsigned int minQty) override {
    std::unique_lock<std::shared_mutex> lock(mutex);
    std::vector<std::pair<std::string, orderIterator>> clean_this_items;

    try {
      for (const auto &item : m_ordersBySecurity.at(securityId)) {
        if (item->qty() >= minQty) {
          clean_this_items.push_back({item->securityId(), item});
          m_ordersById.erase(item->orderId());
          m_ordersByUser.at(item->user()).remove(item);
          m_orders.erase(item);
        }
      }

      if (!minQty) {
        m_ordersBySecurity.erase(securityId);
      } else {
        for (const auto &item : clean_this_items) {
          m_ordersBySecurity.at(item.first).remove(item.second);
        }
      }
    } catch (const std::out_of_range &exception) {
      std::cerr << "There is no entry with specified security ID";
    }
  };

  unsigned int
  getMatchingSizeForSecurity(const std::string &securityId) override {
    using quantity = unsigned;
    using company = std::string;
    using short_order = std::pair<quantity, company>;
    using orders = std::vector<short_order>;
    orders sales;
    orders purchases;

    auto split_orders = [&](auto &sales, auto &purchases) {
      // split orders to sales and purchases
      std::unique_lock<std::shared_mutex> lock(mutex);
      try {
        for (const auto &order : m_ordersBySecurity.at(securityId)) {
          if (!to_lower(order->side()).compare(sell_string)) {
            sales.emplace_back(order->qty(), order->company());
          } else if (!to_lower(order->side()).compare(buy_string)) {
            purchases.emplace_back(order->qty(), order->company());
          }
        }
      } catch (const std::out_of_range &exception) {
        std::cerr << "There is no entry with specified ID";
        return 1;
      }

      if (sales.empty() || purchases.empty()) {
        std::cerr << "No enought purchases and sales to compare\n";
        return 1;
      }
      return 0;
    };

    if (split_orders(sales, purchases)) {
      return 0;
    }

    // this gather all purchases and sales from the one company
    auto accumulate_orders = [](orders &current_order) {
      orders temp;
      auto item = current_order.rbegin();
      while (item != current_order.rend()) {
        auto position =
            std::find_if(temp.begin(), temp.end(), [&](short_order order) {
              return !order.second.compare(item->second);
            });
        if (position != std::end(temp)) {
          position->first += item->first;
        } else {
          temp.emplace_back(*item);
        }
        ++item;
        current_order.pop_back();
      }
      current_order = temp;
    };

    accumulate_orders(sales);
    accumulate_orders(purchases);

    // sort orders in the descendant way
    std::function sort_short_orders = [](orders &orders) {
      std::sort(orders.begin(), orders.end(),
                [](const short_order &a, const short_order &b) {
                  return a.first > b.first;
                });
    };

    sort_short_orders(sales);
    sort_short_orders(purchases);

    auto match_orders = [](auto &sales, auto &purchases) {
      // this is the matching part - each order is checked
      unsigned long accumulator{0};
      for (auto &[buy_quantity, buy_company] : purchases) {
        for (auto &[sell_quantity, sell_company] : sales) {
          if (buy_company == sell_company) {
            continue;
          }
          bool sell_is_bigger{sell_quantity > buy_quantity};
          long match = sell_is_bigger ? sell_quantity - buy_quantity
                                      : buy_quantity - sell_quantity;
          if (sell_is_bigger) {
            accumulator += buy_quantity;
            sell_quantity = match;
            buy_quantity = 0;
          } else {
            accumulator += sell_quantity;
            buy_quantity = match;
            sell_quantity = 0;
          }
        }
      }
      return accumulator;
    };

    return match_orders(sales, purchases);
  };

  std::vector<Order> getAllOrders() const override {
    std::unique_lock<std::shared_mutex> lock(mutex);
    return {m_orders.begin(), m_orders.end()};
  };

  // need this accessor for unit testing
  const ordersList &lookAtList() const { return m_orders; }
};
