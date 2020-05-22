# Object files to either reference or create
OBJECTS = NESEmu.o CPU.o
# The executable file that will be created at the end
EXEC = NESEmu.out
# The flags to use for compilation
FLAGS = -g -Wall
# The code compiler to use for compilation
CC = g++

# Perform action on all object files (May or may not exist)
all: $(OBJECTS)
		$(CC) $(FLAGS) -o $(EXEC) $(OBJECTS)

clean:
	\rm *.out
	\rm *.o
