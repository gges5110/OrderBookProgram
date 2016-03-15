/*
    Pricer.cpp
    Pricer

    Created by Yu Chia Wu on 3/4/16.
    Copyright © 2016 Yu Chia Wu. All rights reserved.


    Implementation:
        I used an ordered_map and an unordered_map to store the information of price and order
        respectively. The goal is to calculate the sell and buy price for different target sizes. Using
        an ordered_map and keep track of the current size of the book can make the calculation very
        fast. I also separated the sell and buy book to two different books so that the maps does not mix
        two data together and thus the map will be more efficient.

    1. How did you choose your implementation language?
        I chose to implement in C++ because I did not need to use a lot of advanced data structures and
        C++ is more efficient.

    2. What is the time complexity for processing an Add Order message?
        For adding an order, the program insert the order directly into the order_map and check if the
        order is in the price map. If not then it will set the key price in price_map to the order
        size, otherwise it will add to the current size. The complexity for this part is O(lg(n)), where
        n is the number of entries inside the price_map, since we are using an ordered map.

        After dealing with the two maps, it calculatest the current price if the current size is large
        enough. It loops over the entries in the price map, and the time complexity is proportional to
        the entries in the map, which is always smaller than the number of add order operations. If
        we have a map of size n, then the complexity would be O(nlg(n)), because again look up in
        price_map is O(lg(n)) and we at most need to iterate n times.

    3. What is the time complexity for processing a Reduce Order message?
        The complexity for reducing an order is very similar to adding an order. The complexity for
        modifying the records in the maps is O(lg(n)). Calculating the price will take O(nlg(n)), n
        being the elements inside the price_map.

    4. If your implementation were put into production and found to be too slow, what ideas would you try out to improve its performance? (Other than reimplementing it in a different language such as C or C++.)
        I am using a unordered_map to record the whole transaction. However, if we only want the prices,
        using a hashset is sufficient to check whether an order_id exists or not. I think our bottleneck
        lies in calculating the prices. If we can somehow predict the in comping order does not affect
        our current price, then there is no need to calculate and compare it. Let me deomstrate with an
        example using an buy order book. If we have a current price at 42.00 in average and the lowest
        price I have used previously is 43.00, then an new add order of price 45.00 will not make the
        new price any lower, thus we should not proceed on calculating the price.

*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <limits>

using std::cout;
using std::endl;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::string;
using std::unordered_map;
using std::map;

// Predefined constants
bool DEBUG = false;
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

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        // print message to tell the user to specify target size
        cout << "Please specify the target size you want to consider." << endl;
        return 1;
    }
    else {
        int target_size = atoi(argv[1]);
        string filename = "pricer.in";
        string out_filename = "my.pricer.out.10000";

        ofstream outfile;
        ifstream myfile(filename);
        if (myfile.is_open()) {
            OrderBook buyOrderBook(target_size, 'B');
            OrderBook sellOrderBook(target_size, 'S');

            string line, output;

            outfile.open(out_filename);
            char order_id_char_arr [20];
            char type, side;
            int ts, size;
            double price;

            while ( getline (myfile, line) ) {
                if (DEBUG)
                    cout << line << "\n";

                sscanf(line.c_str(), "%d %c", &ts, &type);
                // Check the type input ('A' or 'R')
                if (type == 'A') {
                    sscanf(line.c_str(), "%d %c %s %c %lf %d", &ts, &type, order_id_char_arr, &side, &price, &size);

                    string order_id(order_id_char_arr);
                    Order order(ts, order_id, price, size);

                    if (side == 'S') {
                        output = sellOrderBook.addOrder(order);
                    }
                    else if (side == 'B') {
                        output = buyOrderBook.addOrder(order);
                    }
                    outfile << output;
                }
                else if (type == 'R') {
                    sscanf(line.c_str(), "%d %c %s %d", &ts, &type, order_id_char_arr, &size);

                    string order_id(order_id_char_arr);
                    // Need to call both ask and bid
                    output = sellOrderBook.reduceOrder(ts, order_id, size);
                    outfile << output;
                    output = buyOrderBook.reduceOrder(ts, order_id, size);
                    outfile << output;
                }
                if (DEBUG)
                    cout << "\n";
            }
            myfile.close();
            outfile.close();
        }

        else cout << "Unable to open file";
    }
    return 0;
}
