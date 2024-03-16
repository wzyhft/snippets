#include <coroutine>
#include <string>
#include <unordered_map>
#include <iostream>
#include <future>
#include <thread>
#include <vector>

using namespace std;

struct Book {
    int a;
    string b;
};

template <typename FN>
std::thread asyncRequest(FN&& callback, int i) {
    return std::thread(
        [callback=std::forward<FN>(callback), i=i]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            callback();
        }
    );
}

unordered_map<string, Book> symbols;

Book* getBook(const string& sym) {
    auto iter = symbols.find(sym);
    if (iter == symbols.end()) {
        return nullptr;
    }
    return &(iter->second);
}

void processBook(const string& s, int i, vector<thread>& threads) {
    cout << "processing: " << s  << "," << std::this_thread::get_id() << endl;
    auto* book = getBook(s);
    // It is WRONG to pass a pointer by reference to another thread
    // the pointer may point to other stuff or dangling.
    threads.push_back(asyncRequest(
        [&book](){
            cout << book->b << "," << book->a 
            << "," << std::this_thread::get_id()
            << endl;
        }, i
    ));
    //book = getBook("d");
    //book->a = 1;
}

int main() {
    Book a{1, "a"};
    Book b{2, "b"};
    Book c{3, "c"};
    Book d{4, "d"};
    symbols["a"] = a;
    symbols["b"] = b;
    symbols["c"] = c;
    symbols["d"] = d;

    vector<thread> threads;
    processBook("a", a.a, threads);
    processBook("b", b.a, threads);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    processBook("b", b.a, threads);
    processBook("c", c.a, threads);

    for(auto& t : threads) {
        if(t.joinable()) t.join();
    }

    return 0;
}
