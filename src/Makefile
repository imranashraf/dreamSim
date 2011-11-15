CC:=g++
EXEC:=dreamsim
CPPSOURCES=vexsim.cpp rng.cpp main.cpp
HSOURCES=vexsim.h rng.h

all: $(CPPSOURCES) $(HSOURCES)
	@-$(CC) -g -o $(EXEC) main.cpp -DMAX_NODE_CONFIGS=$(MNC)

run: $(EXEC)
	@-echo "Running Simulation ..."
	@-./$(EXEC)
openfiles:
	kate Makefile main.cpp vexsim.h vexsim.cpp &

valgrind: $(EXEC)
	valgrind --tool=memcheck --log-fd=1 -v --show-reachable=yes --leak-check=yes --track-origins=yes ./$(EXEC)

clean:
	@-rm -f $(EXEC) 
	@-rm -f *~ &> /dev/null 
	@-echo "Cleaned Files successfully ..."
	
cleanall:
	@-rm -f *.dsim
	
.PHONY: all clean cleanres
