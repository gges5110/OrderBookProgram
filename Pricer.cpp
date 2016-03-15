/*
    Pricer.cpp
    Pricer

    Created by Yu Chia Wu on 3/4/16.
    Copyright © 2016 Yu Chia Wu. All rights reserved.

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
const string filename = "Test_Files/pricer.in";
const string out_filename = "My_Output/my.pricer.out.10000";

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
