
#include <cstdio>
#include <string>

// NOTE: It is strongly recommended that you set this define in your build system and not in source.
//       Failure to do so will lead to undefined behavior due to mismatching definitions between translation units.
#define AUTOCRYPTED_SEED 1234567
#include <autocrypted.hpp>

struct Big {
    int padding[10];
    int value;
};

struct NonTrivial {
    std::string* name;
    NonTrivial() { name = new std::string("empty"); }
    NonTrivial(char const* name) : name(new std::string(name)) {}
    NonTrivial(NonTrivial const& other) : name(new std::string(*other.name)) {  }
    NonTrivial(NonTrivial&& other) : name(new std::string(std::move(*other.name))) {  }
    NonTrivial& operator=(NonTrivial const& other) { name = new std::string(*other.name); return *this; }
    NonTrivial& operator=(NonTrivial && other) { name = new std::string(std::move(*other.name)); return *this; }
    ~NonTrivial() { delete name; }
};

autocrypted<int> small_number = 10;

int main() {
	std::array<autocrypted<char*>, 2> small_numbers = {
		autocrypted<char*>((char*)"1rawerehaq42h"),
		autocrypted<char*>((char*)"2rwbaZHbrWA"),};
		
	for (const auto& number : small_numbers) {
		printf("%s\n", number.get());
	}
	/*for (auto& number : small_numbers) {
		number.set((char*)"aaaaa");
	}
	for (const auto& number : small_numbers) {
		printf("%s\n", number.get_runtime());
	}
	
	
    // initializing small(less than 8 bytes) value
    //autocrypted<int> small_number = 10;
    printf("small_number = %d\n", small_number.get());

    // explicitly enforce runtime decryption get (or take)
    printf("small_number = %d\n", small_number.get_runtime());

    // setting small value
    small_number.set(11);
    printf("small_number = %d\n", small_number.get());

    // initializing big value
    autocrypted<Big> big_struct = Big{.value = 20};
    printf("big_struct value = %d\n", big_struct.get().value);

    // modifiying part of big value
    big_struct.borrow()->padding[0] = 22;
    printf("big_struct padding = %d\n", big_struct.get().padding[0]);
    printf("big_struct value = %d\n", big_struct.get().value);

    // initializing complex value
    autocrypted<NonTrivial> nontrivial = NonTrivial{"Name"};
    printf("nontrivial name = %s\n", nontrivial.get().name->c_str());

    // take value, non trivial will be taken(moved) from
    auto taken = nontrivial.take();
    printf("taken name = %s\n", taken.name->c_str());
    printf("taken from name =%s\n", nontrivial.get().name->c_str());
	
	*/
}
