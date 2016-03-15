# OrderBookProgram
To compile the program, simply use the command "make". You can use the command "make clean" to remove Pricer.o and Pricer.exe. To execute Pricer.exe, one need to specify the target size and also download the input data from the RGM website. You can use this link: http://www.rgmadvisors.com/problems/orderbook/pricer.in.gz. The output filename is set inside the Pricer.cpp file. 


#Implementation
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
