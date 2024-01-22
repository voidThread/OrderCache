# Order matching task

The repostiory containes the solution for the in-memory caching problem. The main goal is to handle the cache as quickly as possible. Additionaly there is a matching problem that should also be performed as quick as posibile.
The in-memory cache is based on combination of two containers: linked list for fast adding and removal objects, and hash map for fast data searching.
This implementation is using `std::shared_mutex` for thread safety and performance.

## Usage

The `main.cpp` file loads data from a JSON file, and performs matching calculations.
There are 3 flags/arguments for this program.

* first is a path to the json file with data.
* second could be anything - this is a flag to enable the matching calculations.
* third could be anyting - this is a flag to enable extra output from processing data.

### Build

To build `main.cpp` you should create a build directory in the main project path:

> mkdir build

And next run build command:

> clang++ -O3 -std=c++17 -I/usr/local/include lib/simdjson.cpp main.cpp -L/usr/local/lib -pthread -o build/orders_calculation

After this you should have a binary file in the build directory.

### Run

To run these program you need to provide path to the json file as a first argument.
I.e:

> ./build/orders_calculation *path/to/json_file.json*

This will only load data into memory. To perform matching calculations, you need to add flag after path.
I.e:

> ./build/orders_calculation path/to/json_file.json *match*

## Tests

The project uses googletest as a framework. Also there are unit tests in `test` directory. Additionaly there is a python script file which generates test data that can by used with main program. The output data is provided as a json file.

### Run unit tests

The prerequisite to run unit test localy is correctly installed googletest framework.
If you meet the requirement then you can run this line to build the unit test:

> clang++ -std=c++17 -I/usr/local/include test/OrderCache_test.cpp -L/usr/local/lib -lgtest -lgtest_main -pthread -o build/test

After that just run test:

> ./build/test

### Generate test data

The test data could be generated by calling the `data_generator.py` script. It's creating a json file in the same place where script is invoked. If you call this from the main project directory, and have builded main program, then just run this command:

> ./build/orders_calculation test/random_data.json match