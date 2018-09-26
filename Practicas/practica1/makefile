.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe
FLAGS := -std=c++11 -pthread -I.
compilador:=g++


pc_lifo: prodcons_LIFO.cpp
		$(compilador) $(FLAGS) Semaphore.cpp prodcons_LIFO.cpp -o pc_lifo_exe && ./pc_lifo_exe

pc_fifo : prodcons_FIFO.cpp
	$(compilador) $(FLAGS) Semaphore.cpp prodcons_FIFO.cpp -o pc_fifo_exe && ./pc_fifo_exe 

fu: fumadores.cpp
		$(compilador) $(FLAGS) Semaphore.cpp fumadores.cpp -o fu_exe
clean:
	rm -f *_exe
