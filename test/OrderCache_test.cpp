#include "../OrderCache.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class OrderCache_test : public testing::Test {
protected:
  OrderCache cache;
};

TEST_F(OrderCache_test, add_valid_data_Result_data_is_in_cache) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};

  // Act
  cache.addOrder(order);

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 1);
  ASSERT_EQ(cache.lookAtList().back().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().securityId(), "1");
  ASSERT_EQ(cache.lookAtList().back().side(), "Buy");
  ASSERT_EQ(cache.lookAtList().back().qty(), 200);
  ASSERT_EQ(cache.lookAtList().back().user(), "David");
  ASSERT_EQ(cache.lookAtList().back().company(), "Zero");
}

TEST_F(OrderCache_test, add_two_times_same_data_Result_no_new_data_in_cache) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"1", "1", "Buy", 200, "David", "Zero"};

  // Act
  cache.addOrder(order);
  cache.addOrder(order1);

  // Assert
  ASSERT_LE(cache.lookAtList().size(), 1);
  ASSERT_EQ(cache.lookAtList().back().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().securityId(), "1");
  ASSERT_EQ(cache.lookAtList().back().side(), "Buy");
  ASSERT_EQ(cache.lookAtList().back().qty(), 200);
  ASSERT_EQ(cache.lookAtList().back().user(), "David");
  ASSERT_EQ(cache.lookAtList().back().company(), "Zero");
}

TEST_F(OrderCache_test,
       same_data_but_different_Id_Result_no_new_data_in_cache) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"1", "9", "Sell", 600, "Dede", "Flames"};

  // Act
  cache.addOrder(order);
  cache.addOrder(order1);

  // Assert
  ASSERT_LE(cache.lookAtList().size(), 1);
  ASSERT_EQ(cache.lookAtList().back().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().securityId(), "1");
  ASSERT_EQ(cache.lookAtList().back().side(), "Buy");
  ASSERT_EQ(cache.lookAtList().back().qty(), 200);
  ASSERT_EQ(cache.lookAtList().back().user(), "David");
  ASSERT_EQ(cache.lookAtList().back().company(), "Zero");
}

TEST_F(OrderCache_test, data_without_orderId_Result_no_new_data) {
  // Arrange
  Order order{"", "1", "Buy", 200, "David", "Zero"};

  // Act
  cache.addOrder(order);

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 0);
}

TEST_F(OrderCache_test, site_invalid_Result_no_new_data) {
  // Arrange
  Order order{"1", "1", "test", 200, "David", "Zero"};

  // Act
  cache.addOrder(order);

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 0);
}

TEST_F(OrderCache_test, remove_data_using_orderId_Result_data_is_removed) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  cache.addOrder(order);

  // Act
  cache.cancelOrder("1");

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 0);
}

TEST_F(OrderCache_test,
       multiple_data_Remove_data_using_orderId_Result_data_is_removed) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"2", "9", "Sell", 600, "Dede", "Flames"};
  Order order2{"3", "1337", "Sell", 800, "Dexter", "Point"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);

  // Act
  cache.cancelOrder("2");

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 2);
  ASSERT_EQ(cache.lookAtList().front().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().orderId(), "3");
}

TEST_F(OrderCache_test,
       multiple_data_Remove_same_data_using_orderId_Result_nothing_changed) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"2", "9", "Sell", 600, "Dede", "Flames"};
  Order order2{"3", "1337", "Sell", 800, "Dexter", "Point"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);

  // Act
  cache.cancelOrder("2");
  cache.cancelOrder("2");

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 2);
  ASSERT_EQ(cache.lookAtList().front().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().orderId(), "3");
}

TEST_F(OrderCache_test,
       cancel_all_orders_for_user_Result_no_user_orders_in_cache) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"2", "9", "Sell", 600, "Dede", "Flames"};
  Order order2{"3", "1337", "Sell", 800, "Dexter", "Point"};
  Order order3{"4", "1337", "Buy", 1800, "Dexter", "Zero"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);

  // Act
  cache.cancelOrdersForUser("Dexter");

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 2);
  ASSERT_EQ(cache.lookAtList().front().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().orderId(), "2");
}

TEST_F(
    OrderCache_test,
    cancel_all_orders_for_securityId_Result_no_orders_with_securityId_in_cache) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"2", "9", "Sell", 600, "Dede", "Flames"};
  Order order2{"3", "1337", "Sell", 800, "Dexter", "Point"};
  Order order3{"4", "1337", "Buy", 1800, "Dexter", "Zero"};
  Order order4{"4", "1337", "Sell", 1300, "Dexter", "Zero"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);

  // Act
  cache.cancelOrdersForSecIdWithMinimumQty("1337", 0);

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 2);
  ASSERT_EQ(cache.lookAtList().front().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().orderId(), "2");
}

TEST_F(
    OrderCache_test,
    cancel_orders_with_qty_for_securityId_Result_no_orders_with_securityId_in_cache) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"2", "9", "Sell", 600, "Dede", "Flames"};
  Order order2{"3", "1337", "Sell", 800, "Dexter", "Point"};
  Order order3{"4", "1337", "Buy", 1800, "Dexter", "Zero"};
  Order order4{"4", "1337", "Sell", 1300, "Dexter", "Zero"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);

  // Act
  cache.cancelOrdersForSecIdWithMinimumQty("1337", 1000);

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), 3);
  ASSERT_EQ(cache.lookAtList().front().orderId(), "1");
  ASSERT_EQ(cache.lookAtList().back().orderId(), "3");
}

TEST_F(OrderCache_test, try_to_obtain_all_data_Result_all_data_is_shown) {
  // Arrange
  Order order{"1", "1", "Buy", 200, "David", "Zero"};
  Order order1{"2", "9", "Sell", 600, "Dede", "Flames"};
  Order order2{"3", "1337", "Sell", 800, "Dexter", "Point"};
  Order order3{"4", "1337", "Buy", 1800, "Dexter", "Zero"};
  Order order4{"4", "1337", "Sell", 1300, "Dexter", "Zero"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);

  // Act
  auto output = cache.getAllOrders();

  // Assert
  ASSERT_EQ(cache.lookAtList().size(), output.size());
  ASSERT_EQ(cache.lookAtList().front().orderId(), output.front().orderId());
  ASSERT_EQ(cache.lookAtList().back().orderId(), output.back().orderId());
}

TEST_F(
    OrderCache_test,
    get_matching_size_for_security1_with_valid_data_Result_same_company_empty_result) {
  // Arrange
  Order order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"};
  Order order1{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"};
  Order order2{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"};
  Order order3{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"};
  Order order4{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"};
  Order order5{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"};
  Order order6{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"};
  Order order7{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);
  cache.addOrder(order5);
  cache.addOrder(order6);
  cache.addOrder(order7);

  // Act
  auto quantity = cache.getMatchingSizeForSecurity("SecId1");

  // Assert
  ASSERT_EQ(quantity, 0);
}

TEST_F(OrderCache_test,
       get_matching_size_for_security2_with_valid_data_Result_total_quantity) {
  // Arrange
  Order order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"};
  Order order1{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"};
  Order order2{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"};
  Order order3{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"};
  Order order4{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"};
  Order order5{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"};
  Order order6{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"};
  Order order7{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);
  cache.addOrder(order5);
  cache.addOrder(order6);
  cache.addOrder(order7);

  // Act
  auto quantity = cache.getMatchingSizeForSecurity("SecId2");

  // Assert
  ASSERT_EQ(quantity, 2700);
}

TEST_F(
    OrderCache_test,
    get_matching_size_for_security3_with_valid_data_Result_only_one_order_empty_result) {
  // Arrange
  Order order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"};
  Order order1{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"};
  Order order2{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"};
  Order order3{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"};
  Order order4{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"};
  Order order5{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"};
  Order order6{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"};
  Order order7{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"};
  cache.addOrder(order);
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);
  cache.addOrder(order5);
  cache.addOrder(order6);
  cache.addOrder(order7);

  // Act
  auto quantity = cache.getMatchingSizeForSecurity("SecId3");

  // Assert
  ASSERT_EQ(quantity, 0);
}

TEST_F(OrderCache_test,
       get_matching_size_example1_Result_three_valid_matching_sizes) {
  // Arrange
  Order order1{"OrdId1", "SecId1", "Sell", 100, "User10", "Company2"};
  Order order2{"OrdId2", "SecId3", "Sell", 200, "User8", "Company2"};
  Order order3{"OrdId3", "SecId1", "Buy", 300, "User13", "Company2"};
  Order order4{"OrdId4", "SecId2", "Sell", 400, "User12", "Company2"};
  Order order5{"OrdId5", "SecId3", "Sell", 500, "User7", "Company2"};
  Order order6{"OrdId6", "SecId3", "Buy", 600, "User3", "Company1"};
  Order order7{"OrdId7", "SecId1", "Sell", 700, "User10", "Company2"};
  Order order8{"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"};
  Order order9{"OrdId9", "SecId2", "Buy", 900, "User6", "Company2"};
  Order order10{"OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"};
  Order order11{"OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"};
  Order order12{"OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"};
  Order order13{"OrdId13", "SecId1", "Sell", 1300, "User1", "Company"};
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);
  cache.addOrder(order5);
  cache.addOrder(order6);
  cache.addOrder(order7);
  cache.addOrder(order8);
  cache.addOrder(order9);
  cache.addOrder(order10);
  cache.addOrder(order11);
  cache.addOrder(order12);
  cache.addOrder(order13);

  // Act
  auto quantity1 = cache.getMatchingSizeForSecurity("SecId1");
  auto quantity2 = cache.getMatchingSizeForSecurity("SecId2");
  auto quantity3 = cache.getMatchingSizeForSecurity("SecId3");

  // Assert
  ASSERT_EQ(quantity1, 300);
  ASSERT_EQ(quantity2, 1000);
  ASSERT_EQ(quantity3, 600);
}

TEST_F(OrderCache_test,
       get_matching_size_example2_Result_three_valid_matching_sizes) {
  // Arrange
  Order order1{"OrdId1", "SecId3", "Sell", 100, "User1", "Company1"};
  Order order2{"OrdId2", "SecId3", "Sell", 200, "User3", "Company2"};
  Order order3{"OrdId3", "SecId1", "Buy", 300, "User2", "Company1"};
  Order order4{"OrdId4", "SecId3", "Sell", 400, "User5", "Company2"};
  Order order5{"OrdId5", "SecId2", "Sell", 500, "User2", "Company1"};
  Order order6{"OrdId6", "SecId2", "Buy", 600, "User3", "Company2"};
  Order order7{"OrdId7", "SecId2", "Sell", 700, "User1", "Company1"};
  Order order8{"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"};
  Order order9{"OrdId9", "SecId1", "Buy", 900, "User5", "Company2"};
  Order order10{"OrdId10", "SecId1", "Sell", 1000, "User1", "Company1"};
  Order order11{"OrdId11", "SecId2", "Sell", 1100, "User6", "Company2"};
  cache.addOrder(order1);
  cache.addOrder(order2);
  cache.addOrder(order3);
  cache.addOrder(order4);
  cache.addOrder(order5);
  cache.addOrder(order6);
  cache.addOrder(order7);
  cache.addOrder(order8);
  cache.addOrder(order9);
  cache.addOrder(order10);
  cache.addOrder(order11);

  // Act
  auto quantity1 = cache.getMatchingSizeForSecurity("SecId1");
  auto quantity2 = cache.getMatchingSizeForSecurity("SecId2");
  auto quantity3 = cache.getMatchingSizeForSecurity("SecId3");

  // Assert
  ASSERT_EQ(quantity1, 900);
  ASSERT_EQ(quantity2, 600);
  ASSERT_EQ(quantity3, 0);
}
