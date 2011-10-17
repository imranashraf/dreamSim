// a simple test for the Vex Simulator

#include "rng.cpp"
#include "vexsim.cpp"

int main(int argc, char **argv)
{
	unsigned long int TT=1000,TN=10,TC=10;
	unsigned long int Sim_Tasks [20] = {100, 2000, 3000, 4000, 5000,10000,20000,30000,40000,50000,100000,200000,300000,400000,500000,1000000 }; 
	
	switch (argc)
	{
		case 4: TT=atoi(argv[3]);
		case 3: TC=atoi(argv[2]);
		case 2: TN=atoi(argv[1]);
		default:
		break;
	}
		
 		for (int i=0;i<=1;i++)
 		{
 		TT=Sim_Tasks[i];
		VexSim VS(TN,TC,TT);
		VS.Start();
 		}
	
	
	return 0;
}
