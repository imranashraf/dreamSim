// a simple test for the Vex Simulator
// written by:
// Arash Ostadzadeh (ostadzadeh@gmail.com)
// Imran Ashraf	(imran.ashraf@ymail.com) 

#include "rng.cpp"
#include "vexsim.cpp"

int main(int argc, char **argv)
{
	unsigned long int TT=5,TN=1,TC=1;
/*	VexSim VS(TN,TC,TT);
	VS.Start();*/
	
	unsigned long int Sim_Tasks [10] = {1000, 2000, 3000, 4000, 5000,6000,7000,8000,9000,10000}; 
	
	for (int i=0;i<5;i++)
	{
		TT=Sim_Tasks[i];
		VexSim VS(TN,TC,TT);
		VS.Start();
	}
	
	return 0;
}

// switch (argc)
// {
// 	case 4: TT=atoi(argv[3]);
// 	case 3: TC=atoi(argv[2]);
// 	case 2: TN=atoi(argv[1]);
// 	default:
// 		break;
// }
