# zip_iterator
C++14 zip_iterator

```example
    std::vector<int> v = {1,2,4,8,16};
    std::vector<std::size_t> key = {2,1,5,4,3};
    std::sort(
        tuple_utils::make_zip_iterator(v.begin(), key.begin()),
        tuple_utils::make_zip_iterator(v.end(), key.end()),
        [](const auto& a, const auto& b){
            return std::get<1>(a) < std::get<1>(b);
        }
    );

    // v   = {2, 1, 16, 8, 4}
    // key = {1, 2,  3, 4, 5}
```