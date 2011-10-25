CC:=g++
EXEC:=dreamsim
CPPSOURCES=vexsim.cpp rng.cpp kbhit.c main.cpp
HSOURCES=vexsim.h rng.h kbhit.h

all: $(CPPSOURCES) $(HSOURCES)
	@-$(CC) -g -o $(EXEC) main.cpp

run: $(EXEC)
	@-echo "Running Simulation ..."
	@-./$(EXEC)

clean:
	@-rm -f $(EXEC) 
	@-echo "Cleaned Files successfully ..."
	
.PHONY: all clean	
