#include <catch2/catch_test_macros.hpp>
#include <veloxla/veloxla.hpp>
#include <string>
#include <iterator>
#include <numeric>
#include <algorithm>

using veloxla::Storage;

TEST_CASE("Storage default construction is empty", "[storage]") {
    Storage<int> s;
    REQUIRE(s.size() == 0);
    REQUIRE(s.empty());
    REQUIRE(s.data() == nullptr);
}

TEST_CASE("Storage sized construction default-initializes elements", "[storage]") {
    Storage<int> s(5);
    REQUIRE(s.size() == 5);
    REQUIRE_FALSE(s.empty());

    for(std::size_t i = 0; i < s.size(); ++i) {
        REQUIRE(s[i] == 0);
    }
}

TEST_CASE("Storage fill construction sets all elements", "[storage]") {
    Storage<int> s(4, 7);
    for(std::size_t i = 0; i < s.size(); ++i) {
        REQUIRE(s[i] == 7);
    }
}

TEST_CASE("Storage zero sized contruction", "[Storage]") {
    // Branch coverage for `allocate(0)` → `nullptr` (see `storage.hpp`).
    // Without this test, the branch n == 0 in allocate() is never executed
    // by the test suite at all.

    Storage<int> s(0);

    REQUIRE(s.size() == 0);
    REQUIRE(s.empty());
    REQUIRE(s.data() == nullptr);
}

TEST_CASE("Storage operator[] allows read and write", "[storage]") {
    Storage<int> s(3);
    s[0] = 10;
    s[1] = 20;
    s[2] = 30;

    REQUIRE(s[0] == 10);
    REQUIRE(s[1] == 20);
    REQUIRE(s[2] == 30);
}

TEST_CASE("Storage at() throws on out-of-range index", "[storage]") {
    Storage<int> s(3);
    REQUIRE_THROWS_AS(s.at(3), std::out_of_range);
    REQUIRE_THROWS_AS(s.at(100), std::out_of_range);
    REQUIRE_NOTHROW(s.at(2));
}

TEST_CASE("Storage move constructor transfers ownership", "[storage]") {
    Storage<int> a(3, 42);
    int* original_ptr = a.data();

    Storage<int> b(std::move(a));

    REQUIRE(b.size() == 3);
    REQUIRE(b.data() == original_ptr);
    REQUIRE(b[0] == 42);
    
    // The source must be in a valid empty state after being moved from.
    REQUIRE(a.size() == 0);
    REQUIRE(a.data() == nullptr);
}

TEST_CASE("Storage move construct from empty storage", "[storage]") {
    // Edge case: the source itself is already empty (data_ == nullptr).
    // Make sure the move constructor doesn't wrongly assume data_ is always non-null.
    
    Storage<int> a;

    Storage<int> b(std::move(a));

    REQUIRE(b.empty());
    REQUIRE(b.data() == nullptr);

    REQUIRE(a.empty());
    REQUIRE(a.data() == nullptr);
}

TEST_CASE("Storage move assignment from empty storage releases destination buffer", "[storage]") {
    // The opposite of the existing move-assignment test: here’s the SOURCE
    // (b) which is empty, not the goal. This validates destroy_all() +
    // deallocate() is still correctly called on a's OLD buffer even though
    // hasil akhirnya a jadi kosong juga.
    Storage<int> a(10, 1);
    Storage<int> b;

    a = std::move(b);

    REQUIRE(a.empty());
    REQUIRE(a.size() == 0);
    REQUIRE(a.data() == nullptr);
    
}

TEST_CASE("Storage move assignment releases old buffer and nulls source", "[storage]") {
    // This test specifically catches a bug that used to exist when
    // operator=(Storage&&) is declared = default: no implementation
    // manual, old buffer *this leaks (never deallocated) and
    // other.data_ wasn't set to null, causing a double free when the destructor runs
    // other terpanggil di akhir scope test ini.

    Storage<int> a(10, 1);  // A's old buffer — must be released when move-assigning
    Storage<int> b(3, 99);  // sumber yang akan di-move

    int* b_ptr = b.data();

    a = std::move(b);

    REQUIRE(a.size() == 3);
    REQUIRE(a.data() == b_ptr);
    REQUIRE(a[0] == 99);

    // b has to be a valid empty state — if not, b's destructor will
    // the end of the scope will try to deallocate the pointer that has already been moved to a,
    // alias double free.
    REQUIRE(b.size() == 0);
    REQUIRE(b.data() == nullptr);

    // The scope ends here: destructors a and b run. If there is
    // double-free or leak, the sanitizer (ASan) will catch it when
    // run with -fsanitize=address.
}

TEST_CASE("Storage move assignment to self is a no-op", "[storage]") {
    Storage<int> a(3, 5);
    int* ptr_before = a.data();
    

    // Self-move assignment must be safe: no crashes, no state corruption.
    // Use an alias pointer so the compiler doesn't warn -Wself-move about
    // "a = std::move(a)" literally — but still test the same code path
    // (operator= called with this == &other).

    Storage<int>& self_ref = a;
    a = std::move(self_ref);

    REQUIRE(a.data() == ptr_before);
    REQUIRE(a.size() == 3);
}

TEST_CASE("Storage works with non-trivial type (std::string)", "[storage]") {
    // Validate construct_all/destroy_all correctly for types with destructors
    // non-trivial — if there's a bug in the construct/destroy pairing, this will
    // crash or leak that appears in the sanitizer.
    Storage<std::string> s(3, std::string("hello"));

    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == "hello");
    REQUIRE(s[1] == "hello");
    REQUIRE(s[2] == "hello");

    s[1] = "world";
    REQUIRE(s[1] == "world");
}

TEST_CASE("Storage is not copyable", "[storage]") {
    // This is a static_assert, not a runtime check — it just makes sure this code
    // compiles and REQUIRE(true) as a placeholder assertion.
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<Storage<int>>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<Storage<int>>);
    REQUIRE(true);
}

TEST_CASE("Storage swap exchanges buffers without copying", "[storage]") {
    Storage<int> a(3, 1);
    Storage<int> b(5, 2);

    int* a_ptr = a.data();
    int* b_ptr = b.data();

    a.swap(b);
    
    // After the swap, the pointers really have to be swapped (not a deep copy).
    REQUIRE(a.data() == b_ptr);
    REQUIRE(b.data() == a_ptr);
    REQUIRE(a.size() == 5);
    REQUIRE(b.size() == 3);
    REQUIRE(a[0] == 2);
    REQUIRE(b[0] == 1);
}

TEST_CASE("Storage iterators cover the full range in order", "[storage]") {
    Storage<int> s(4);
    int counter = 0;

    for(auto it = s.begin(); it != s.end(); ++it) {
        *it = counter++;
    }

    REQUIRE(s[0] == 0);
    REQUIRE(s[1] == 1);
    REQUIRE(s[2] == 2);
    REQUIRE(s[3] == 3);
    
    // Range-based for loops must work through the same begin()/end().
    int sum = 0;
    for(int v : s) {
        sum += v;
    }
    REQUIRE(sum == 0 + 1 + 2 + 3);
}

TEST_CASE("Storage const iterators (cbegin/cend) work on const object", "[storage]") {
    const Storage<int> s(3, 9);

    int sum = 0;
    for(auto it = s.cbegin(); it != s.cend(); ++it) {
        sum += *it;
    }

    REQUIRE(sum == 27);
    REQUIRE(std::distance(s.begin(), s.end()) == 3);
}

TEST_CASE("Storage constructed with explicit allocator instance", "[storage]") {
    std::allocator<int> alloc;
    Storage<int> s(4, alloc);

    REQUIRE(s.size() == 4);
    for(std::size_t i = 0; i < s.size(); ++i) {
        REQUIRE(s[i] == 0);
    }
}


namespace {
// Helper type that intentionally throws an exception on the N-th construction to
// validate the exception safety of construct_all(): if any element
// fails to be constructed halfway, the elements that have ALREADY been constructed
// before must be destroyed in reverse order, and the buffer must be deallocated —
// no partially-constructed objects should leak.
//
// Counter is manually reset at the start of each TEST_CASE that uses it (not
// through static init constructor) because Catch2 runs all
// TEST_CASEs in the same process; without reset, the state of test
// previously it will leak to the next test and cause the results to be inaccurate
// deterministic depending on the execution order.
struct ThrowingType
{
    static int contruction_count;
    static int throw_after_n;

    ThrowingType()
    {
        if(contruction_count++ == throw_after_n)
        {
            throw std::runtime_error("ThrowingType: simulated construction failure");
        }
    }

    ~ThrowingType() = default;
};

int ThrowingType::contruction_count = 0;
int ThrowingType::throw_after_n = 0;

}  // namespace

TEST_CASE("Storage construct_all rolls back partially constructed elements on exception", "[storage]") {
    ThrowingType::contruction_count = 0;
    ThrowingType::throw_after_n = 3;       // the 4th element (index 3) that will throw

    // If construct_all() does NOT roll back correctly (destroying elements
    // [0, i) that were successfully constructed before the exception, then
    // deallocating the buffer), elements 0,1,2 that have been constructed will
    // leak — both as a memory leak (buffer is never freed) and as an object
    // leak (destructors for 0,1,2 are never called). This test itself only
    // verifies that the exception is actually thrown and propagates to the caller;
    // CONFIRMATION that there are no leaks is only valid when this binary is run
    // under AddressSanitizer/LeakSanitizer (see the sanitizer workflow in
    // scripts/profile_perf.sh or run manually with ASAN_OPTIONS=detect_leaks=1).

    REQUIRE_THROWS_AS(Storage<ThrowingType>(10), std::runtime_error);
}

TEST_CASE("Storage construct_all succeeds when no exception is thrown", "[storage]") {
    // Sanity check companion for the test above: make sure ThrowingType
    // itself is not wrongly designed — if throw_after_n is set outside
    // the construction range, all elements are constructed successfully.

    ThrowingType::contruction_count = 0;
    ThrowingType::throw_after_n = 100;     // will never be achieved for small sizes

    REQUIRE_NOTHROW(storage<ThrowingType>(5));
}

TEST_CASE("Storage works with STL algorithms (std::accumulate)", "[storage]") {
    // The tensor engine will frequently use STL algorithms over the Storage
    // range (reduction, transform, etc.) — this test validates that
    // begin()/end() strictly satisfy the LegacyForwardIterator contract
    // required by std::accumulate and similar STL algorithms.

    storage<int> s(5,1);

    int sum = std::accumulate(s.begin(). s.end(), 0);

    REQUIRE(sum == 5);
}

TEST_CASE("Storage works with STL algorithms (std::fill and std::count)", "[storage]") {
    Storage<int> s(6, 0);

    std::fill(s.begin(), s.end(), 3);
    auto count = std::count(s.begin(), s.end(), 3);

    REQUIRE(count == 6);
}
