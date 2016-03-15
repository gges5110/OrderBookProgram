#include <iostream>
#include <sstream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iomanip>

using std::cout;
using std::endl;
using std::stringstream;
using std::string;
using std::unordered_map;
using std::map;

double OLD_PRICE_MAX = std::numeric_limits<double>::max();
// A simple class that holds the data for an order
class Order {
public:
    int timestamp, size;
    string order_id;
    double price;

    Order(): timestamp(0), order_id("0"), price(0.0), size(0) {}
    Order(int ts, string o, double p, int s) : timestamp(ts), order_id(o), price(p), size(s) {}
};

// typedef for iterators
typedef map<double, int>::const_iterator PriceMapIter;
typedef map<double, int>::reverse_iterator PriceMapRevIter;
typedef unordered_map<string, Order>::const_iterator OrderMapIter;


// The class for an order book. It can add or reduce orders, and return the corresponding output.
class OrderBook {
public:
    // An order book has two types: sell and buy. For an buy order book, the price should be as low as possible. On the other hand, an sell order book should have prices as high as possible.
    OrderBook(const int t, const char book_type): target_size(t), bookType(book_type), currentSize(0) {
        if (book_type == 'B') {
            mySide = 'S';
            old_price = 0.0;
        }
        else if (book_type == 'S') {
            mySide = 'B';
            old_price = OLD_PRICE_MAX;
        }
    };

    // addOrder takes in an order and update order_map and price_map. It finally calles print_message
    // to return a string in response to our add order.
    string addOrder(Order order) {
        currentSize += order.size;
        order_map[order.order_id] = order;

        price_map_it = price_map.find(order.price);
        if (price_map_it == price_map.end())    price_map[order.price] = order.size;
        else                                    price_map[order.price] += order.size;

        return print_message(order.timestamp, false);
    }

    // Takes an order_id with its amount, it will first find inside the order_map and see if it really
    // exists inside our database. If so then we will update both order_map and price_map.
    string reduceOrder(int timestamp, string order_id, int amount) {
        order_map_it = order_map.find(order_id);
        if(order_map_it != order_map.end()) {
            // There is a record in the order book
            double price_to_reduce = order_map[order_id].price;
            Order temp = order_map[order_id];

            // Need to check if the order size is larger than the reduce size
            if (temp.size < amount) {
                // Trying to reduce the order more than its original size, error message.
            }
            temp.size -= amount;
            currentSize -= amount;

            order_map[order_id] = temp;
            price_map[price_to_reduce] -= amount;
            return print_message(timestamp, true);
        }
        return "";
    }

private:
    // Declare members for price map
    map<double, int> price_map;
    PriceMapIter price_map_it;
    PriceMapRevIter price_map_rit;
    // Declare members for order map
    unordered_map<string, Order> order_map;
    OrderMapIter order_map_it;
    // Declare members
    int target_size, currentSize;
    char bookType, mySide;
    double old_price;
    bool was_available = false;

    string print_message(int timestamp, bool reduce) {

        // Check if the current size if enough for selling and buying
        if (currentSize >= target_size) {
            double current_price = calculate_price();
            was_available = true;
            std::stringstream buffer;

            // For the buy side, we print message only when the current price is lower.
            // For the sell side, we print message only when the current price is higher.
            if (mySide == 'B') {
                // We also do not want to print message on reduce order and the price did not change.
                if (current_price < old_price || (reduce && current_price != old_price)) {                   buffer << timestamp << " " << mySide << " "  << std::setprecision(2) << std::fixed << calculate_price() << endl;
                }
            }
            else {
                if (current_price > old_price || (reduce && current_price != old_price)) {
                    buffer << timestamp << " " << mySide << " " << std::setprecision(2)  << std::fixed << calculate_price() << endl;
                }
            }
            old_price = current_price;
            return buffer.str();
        }
        else {
            if (mySide == 'B')
                old_price = OLD_PRICE_MAX;
            else
                old_price = 0.0;

            std::stringstream buffer;
            // If it was available then we need to print NA for now
            if (was_available) {
                buffer << timestamp << " " << mySide << " NA" << endl;
            }
            was_available = false;
            return buffer.str();
        }
    }

    double calculate_price() {
        int remain_size = target_size;
        double sum = 0.0;

        // For the buy side, we start from the lowest price, using the reverse_iterator.
        if (bookType == 'B') {
            for (PriceMapRevIter iter = price_map.rbegin(); iter != price_map.rend(); iter++) {
                // Check if we still need to buy more.
                if (remain_size > 0) {
                    int size_to_buy = std::min(remain_size, iter->second);
                    sum += size_to_buy * iter->first;
                    remain_size -= size_to_buy;
                }
                else {
                    break;
                }
            }
        }
        else {
            for (PriceMapIter iter = price_map.begin(); iter != price_map.end(); iter++) {
                if (remain_size > 0) {
                    int size_to_buy = std::min(remain_size, iter->second);
                    sum += size_to_buy * iter->first;
                    remain_size -= size_to_buy;
                }
                else {
                    break;
                }
            }
        }

        return sum;
    }
};
