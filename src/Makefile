CC=g++
EXEC=dreamsim
CPPSOURCES=DReAMSim.cpp rng.cpp main.cpp
HSOURCES=DReAMSim.h rng.h

#Maximum Number of Configuration
MNC?=10

all: $(CPPSOURCES) $(HSOURCES)
	@-$(CC) -g -o $(EXEC) main.cpp -DMAX_NODE_CONFIGS=$(MNC)

run: $(EXEC)
	@-./$(EXEC)

openfiles:
	kate Makefile main.cpp DReAMSim.h DReAMSim.cpp &

valgrind: $(EXEC)
	valgrind --tool=memcheck --log-fd=1 -v --show-reachable=yes --leak-check=yes --track-origins=yes ./$(EXEC)

clean:
	@-rm -f $(EXEC) dump.txt 
	@-rm -f *~  
	
delete:
	@-rm -f *.dsim
	
.PHONY: all clean delete

