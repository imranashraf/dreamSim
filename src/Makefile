CC:=g++
EXEC:=dreamsim
CPPSOURCES=vexsim.cpp rng.cpp main.cpp
HSOURCES=vexsim.h rng.h

all: $(CPPSOURCES) $(HSOURCES)
	@-$(CC) -o $(EXEC) main.cpp
	@-echo "Compilation Done Successfully ..."

run: $(EXEC)
	@-echo "Running Simulation ..."
	@-./$(EXEC)

clean:
	@-rm -f $(EXEC) *.txt
	@-echo "Cleaned Files successfully ..."
	
.PHONY: all clean	