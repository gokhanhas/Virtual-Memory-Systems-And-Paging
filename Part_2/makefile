target : final_part2

final_part2 : part2.cpp  pageentry.cpp queue.cpp clockQueue.cpp pageentry.h queue.h clockQueue.h

	g++ part2.cpp pageentry.cpp queue.cpp clockQueue.cpp pageentry.h queue.h clockQueue.h -std=c++11 -lpthread -o sortArrays -Wall

clean:
	rm sortArrays