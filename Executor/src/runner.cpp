#define BOOST_DATE_TIME_NO_LIB

#include "process.hpp"
#include "boost/interprocess/shared_memory_object.hpp"
#include <boost/interprocess/mapped_region.hpp>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <iostream>
#include <vector>

using namespace TinyProcessLib;

struct shared_memory_buffer
{
	size_t offset;
	char name[50];
};

struct Command {
	std::string _exeName;
	std::vector<std::string> _parameters;
	Command(std::string params = "")
		: _exeName("/home/hemant/test-src/cxxparser-as-process/bin/helloWorld")
	{
		_parameters.push_back(params);
	}
	std::string ToString() {
		std::string commandLine = _exeName;
		for (std::string param : _parameters)
			commandLine += " " + param;
		return commandLine;
	}
};

int main() {
	using namespace boost::interprocess;
	//Typedefs
	typedef allocator<char, managed_shared_memory::segment_manager>	CharAllocator;
	typedef basic_string<char, std::char_traits<char>, CharAllocator> MyShmString;
	typedef allocator<MyShmString, managed_shared_memory::segment_manager> StringAllocator;
	typedef vector<MyShmString, StringAllocator> MyShmStringVector;

	//Open shared memory
	//Remove shared memory on construction and destruction
	struct shm_remove
	{
		shm_remove() { shared_memory_object::remove("MySharedMemory"); }
		~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
	} remover;

	managed_shared_memory shm(create_only, "MySharedMemory", 10000);

	//Create allocators
	CharAllocator     charallocator(shm.get_segment_manager());
	StringAllocator   stringallocator(shm.get_segment_manager());

	//This string is in only in this process (the pointer pointing to the
	//buffer that will hold the text is not in shared memory).
	//But the buffer that will hold "this is my text" is allocated from
	//shared memory
	MyShmString mystring(charallocator);
	mystring = "this is my text";

	//This vector is fully constructed in shared memory. All pointers
	//buffers are constructed in the same shared memory segment
	//This vector can be safely accessed from other processes.
	MyShmStringVector *myshmvector =
		shm.construct<MyShmStringVector>("myshmvector")(stringallocator);
	//myshmvector->insert(myshmvector->begin(), 10, mystring);

	//Destroy vector. This will free all strings that the vector contains
	shm.destroy_ptr(myshmvector);

	shared_memory_object shm_obj
	(open_or_create					//only create
		, "cxxparser_sm"				//name
		, read_write					//read-write mode
	);

	shm_obj.truncate(sizeof(shared_memory_buffer));

	//Map the whole shared memory in this process
	mapped_region region(shm_obj, read_write);

	void *addr = region.get_address();
	shared_memory_buffer *data = new (addr) shared_memory_buffer;

	std::set_terminate([]() { std::cout << "Unhandled exception\n"; std::exit(-1); });

	std::vector<Command> commands = {
		 Command("Hemant"),
		 Command("Bhagat"),
		 Command(),
		 Command("Chetan"),
		 Command()
	};

	for (Command cmd : commands) {
		std::cout << "Process started.\n";
		Process process1(cmd.ToString(), "", [](const char *bytes, size_t n) {
			//std::cout << "Output from stdout:\n" << std::string(bytes, n);
		});

		auto exit_status = process1.get_exit_status();
		std::string str = std::string(myshmvector->back().begin(), myshmvector->back().end());
		std::cout << "Vector content: ";
		for (MyShmString mystring : *myshmvector) {
			std::string str = std::string(mystring.begin(), mystring.end());
			std::cout << str << ", ";
		}
		std::cout << "CXXParser returned: " << exit_status << " (" << (exit_status == 0 ? "success" : "failure") << ")" << std::endl;
	}
	bool isRemoved = shared_memory_object::remove("cxxparser_sm");
	if (isRemoved)
		std::cout << "Shared memory is destroyed\n";

	//Destroy vector. This will free all strings that the vector contains
	shm.destroy_ptr(myshmvector);
	getchar();
}
