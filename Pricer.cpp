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
#include <Pricer.h>

#include <iostream>
#include <fstream>
#include <vector>

using std::cout;
using std::endl;
using std::vector;
using std::ifstream;
using std::ofstream;

// Predefined constants
const bool DEBUG = false;
const string filename = "pricer.in";
const string out_filename = "my.pricer.out.10000";

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        // print message to tell the user to specify target size
        cout << "Please specify the target size you want to consider." << endl;
        return 1;
    }
    else {
        int target_size = atoi(argv[1]);

        ofstream outfile;
        ifstream myfile(filename);
        if (myfile.is_open()) {
            OrderBook buyOrderBook(target_size, 'B');
            OrderBook sellOrderBook(target_size, 'S');

            string line, output;

            outfile.open(out_filename);
            char order_id_char_arr[20];
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

        else cout << "Unable to open file\n";
    }
    return 0;
}
