#define BOOST_DATE_TIME_NO_LIB

#include "boost/interprocess/shared_memory_object.hpp"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include <iostream>
#ifdef WIN32
#include <Windows.h>
#include <rtcapi.h>

int exception_handler(LPEXCEPTION_POINTERS p)
{
	printf("Exception detected during the unit tests!\n");
	exit(-1);
}
int runtime_check_handler(int errorType, const char *filename, int linenumber, const char *moduleName, const char *format, ...)
{
	printf("Error type %d at %s line %d in %s", errorType, filename, linenumber, moduleName);
	exit(1);
}
#endif


struct shared_memory_buffer
{
	size_t offset;
	char name[50];
};

int main(int argc, char **argv) {
	#ifdef WIN32
	DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
	SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&exception_handler);
	_RTC_SetErrorFunc(&runtime_check_handler);
#endif

	using namespace boost::interprocess;
	//Typedefs
	typedef allocator<char, managed_shared_memory::segment_manager>	CharAllocator;
	typedef basic_string<char, std::char_traits<char>, CharAllocator> MyShmString;
	typedef allocator<MyShmString, managed_shared_memory::segment_manager> StringAllocator;
	typedef vector<MyShmString, StringAllocator> MyShmStringVector;

	//Open already created shared memory object.
	shared_memory_object shm(open_only, "cxxparser_sm", read_write);
	managed_shared_memory shm1(open_only, "MySharedMemory");
	//Create allocators
	CharAllocator     charallocator(shm1.get_segment_manager());
	StringAllocator   stringallocator(shm1.get_segment_manager());
	MyShmStringVector *myshmvector = shm1.find_or_construct<MyShmStringVector>("myshmvector")(stringallocator);

	//Map the whole shared memory in this process
	mapped_region region(shm, read_write);

	void *addr = region.get_address();
	shared_memory_buffer *data = new (addr) shared_memory_buffer;
	data->offset++;
	std::string arg = "";
	if (argc > 1)
		arg = argv[1];
	else {
		std::memset(data->name, 0, 50);
		throw;
	}
	MyShmString mystring(charallocator);
	mystring = arg.c_str();
	myshmvector->push_back(mystring);

	std::cout << "################################################\n";
	return 0;
}