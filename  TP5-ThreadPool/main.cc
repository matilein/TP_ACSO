#include "thread-pool.h"
#include <iostream>
#include <vector>
#include <numeric>  
#include <functional>

using namespace std;


void computeSum(const vector<int>& data, int start, int end, int* result) {

    *result = accumulate(data.begin() + start, data.begin() + end, 0);
}

int main() {
    
    vector<int> data = {100, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int numThreads = 3;
    ThreadPool pool(numThreads);

    
    vector<int> results(numThreads, 0);

   
    int n = data.size();
    int chunkSize = (n + numThreads - 1) / numThreads;  

    
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = min(start + chunkSize, n);
        if (start < n) {
            pool.schedule([start, end, i, &data, &results](void)-> void {computeSum(data, start, end, &results[i]);});
        }
    }


    pool.wait();

    int totalSum = accumulate(results.begin(), results.end(), 0);
    cout << "Total sum of elements: " << totalSum << endl;

    return 0;
}