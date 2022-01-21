#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdbool.h>

#define COUNT_READERS 5
#define COUNT_WRITERS 3
#define ITERATIONS_NUMBER 8

#define READER_DELAY 400
#define WRITER_DELAY 400
#define DELAY 1000

HANDLE reader_threads[COUNT_READERS];
HANDLE writer_threads[COUNT_WRITERS];
HANDLE can_read;
HANDLE can_write;
HANDLE mutex;

int id_readers[COUNT_READERS];
int id_writers[COUNT_WRITERS];

int value = 0;

LONG count_waiting_writers = 0;
LONG count_waiting_readers = 0;
LONG count_active_readers = 0;

bool is_writer_active  = false;


int init();
int create_threads();

DWORD WINAPI Writer(LPVOID lp_param);
void start_write();
void stop_write();

DWORD WINAPI Reader(LPVOID lp_param);
void start_read();
void stop_read();

void close();

int main(void)
{
	setbuf(stdout, NULL);
	
	int error = init();
	if (error){
		return -1;
	}
	
	error = create_threads();
	if (error){
		return -1;
	}
	
	WaitForMultipleObjects(COUNT_WRITERS, writer_threads, TRUE, INFINITE);
	WaitForMultipleObjects(COUNT_READERS, reader_threads, TRUE, INFINITE);
	
	close();
	printf("\nFinish.");
	
	return 0;
}

int init()
{
	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		perror("CreateMutex error.");
		return -1;
	}
	
	can_write = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (can_write == NULL)
	{
		perror("CreateEvent (can_write) error.");
		return -1;
	}
	
	can_read = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (can_read == NULL)
	{
		perror("CreateEvent (can_read) error.");
		return -1;
	}
	
	return 0;
}

int create_threads()
{
	for (int i = 0; i < COUNT_WRITERS; i++)
	{
		id_writers[i] = i;
		if ((writer_threads[i] = CreateThread(NULL, 0, &Writer, (LPVOID)id_writers + i, 0, NULL)) == NULL)
		{
			perror("CreateThread (writer) error.");
			return -1;
		}
	}
	
	for (int i = 0; i < COUNT_READERS; i++)
	{
		id_readers[i] = i;
		if ((reader_threads[i] = CreateThread(NULL, 0, &Reader, (LPVOID)id_readers + i, 0, NULL)) == NULL)
		{
			perror("CreateThread (reader) error.");
			return -1;
		}
	}
		
	return 0;
}

DWORD WINAPI Writer(LPVOID lp_param)
{
	int id = (long long)lp_param;
	int sleep_time;
	srand(time(NULL) + id);
	
	for (int i = 0; i < ITERATIONS_NUMBER; i++)
	{
		sleep_time = rand() % DELAY + READER_DELAY;
		Sleep(sleep_time);
		
		start_write();
		value++;
		printf("Writer: %d; value = %d;\n", id, value);
		stop_write();
	}
}

void start_write()
{
	InterlockedIncrement(&count_waiting_writers);
	
	if (is_writer_active || count_active_readers > 0){
		WaitForSingleObject(can_write, INFINITE);
	}
	
	InterlockedDecrement(&count_waiting_writers);
	is_writer_active = true;
}

void stop_write()
{
	is_writer_active = false;
	if (count_waiting_writers == 0){
		SetEvent(can_read);
	}
	else{
		SetEvent(can_write);
	}
}
	
DWORD WINAPI Reader(LPVOID lp_param)
{
	int id = (long long)lp_param;
	int sleep_time;
	srand(time(NULL) + id);

	for (int i = 0; i < ITERATIONS_NUMBER; i++)
	{
		sleep_time = rand() % DELAY + READER_DELAY;
		Sleep(sleep_time);
		
		start_read();
		printf("Reader: %d; value = %d;\n", id, value);
		stop_read();
	}
}

void start_read()
{
	InterlockedIncrement(&count_waiting_readers);
	
	WaitForSingleObject(mutex, INFINITE);
	if(is_writer_active || count_waiting_writers){
		WaitForSingleObject(can_read, INFINITE);
	}
	
	InterlockedIncrement(&count_active_readers);
	InterlockedDecrement(&count_waiting_readers);
	
	SetEvent(can_read);
	ReleaseMutex(mutex);
}

void stop_read()
{
	InterlockedDecrement(&count_active_readers);
	if (count_waiting_readers == 0){
		ResetEvent(can_read);
		SetEvent(can_write);
	}
}

void close()
{
	for (int i = 0; i < COUNT_WRITERS; i++){
		CloseHandle(writer_threads[i]);
	}
	
	for (int i = 0; i < COUNT_READERS; i++){
		CloseHandle(reader_threads[i]);
	}
	
	CloseHandle(can_read);
	CloseHandle(can_write);
	CloseHandle(mutex);
}