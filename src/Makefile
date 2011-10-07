CC:=g++
EXEC:=dreamsim
CPPSOURCES=rng.cpp main.cpp vexsim.cpp 
HSOURCES=vexsim.h rng.h

all: $(CPPSOURCES) $(HSOURCES)
	@-$(CC) -o $(EXEC) main.cpp
	@-echo "Compilation Done Successfully ..."

run: $(EXEC)
	@-echo "Running Simulation ..."
	@-./$(EXEC)

openfiles: $(CPPSOURCES) $(HSOURCES) 
	@-echo -e "\n Opening Files in kate editor ... \n";
	@-kate $(HSOURCES) $(CPPSOURCES) &
	@-echo -e "\n Opened Files Successfully ! \n"

openfolder:
	@-echo -e "\n Opening Folder ... \n";
	@-nautilus ../src &
	@-echo -e "\n Opened Files Successfully ! \n"
clean:
	@-echo -e "\n Cleaning Files ...\n"
	@-rm -f $(EXEC) *.txt
	@-echo -e "\nCleaned Files Successfully ! \n"

	
.PHONY: all clean openfiles openfolders	
