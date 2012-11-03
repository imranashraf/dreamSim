// a simple test for the Vex Simulator
// written by:
// Arash Ostadzadeh (ostadzadeh@gmail.com)
// Imran Ashraf	(imran.ashraf@ymail.com) 

#include "rng.cpp"
#include "DReAMSim.cpp"

int main(int argc, char **argv)
{
	unsigned long int TT=10000,TN=200,TC=100;
/*	VexSim VS(TN,TC,TT);
	VS.Start();*/
	
	//unsigned long int Sim_Tasks [10] = {500, 1000, 1500, 2000, 2500,3000,3500,4000,4500,5000}; 
	for (int i=5;i<=45;i+=10)
	{
		//TT=Sim_Tasks[i];
		VexSim VS(i,10,50);
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
