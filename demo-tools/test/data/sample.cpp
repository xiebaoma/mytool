#include <iostream>
#include <vector>
#include <string>

class SampleClass {
private:
    std::vector<std::string> data_;
    
public:
    SampleClass(const std::vector<std::string>& data) : data_(data) {}
    
    void print_data() const {
        for (const auto& item : data_) {
            std::cout << item << std::endl;
        }
    }
    
    size_t size() const {
        return data_.size();
    }
};

int main() {
    std::vector<std::string> test_data = {
        "First line",
        "Second line", 
        "Third line"
    };
    
    SampleClass sample(test_data);
    sample.print_data();
    std::cout << "Total items: " << sample.size() << std::endl;
    
    return 0;
}
