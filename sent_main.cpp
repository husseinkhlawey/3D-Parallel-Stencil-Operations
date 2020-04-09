/*
	Parallel 3D Stencil Operations for Simulating Heat Dissipation.

	http://faculty.knox.edu/dbunde/teaching/cilk/
*/

#include <iostream>
#include <math.h>
#include <string.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <fstream>
using namespace std;

void calcX(float alpha, float beta, int DIM_X, int DIM_Y, int DIM_Z, float grid_a[][100][100], float grid_b[][100][100], int x, int y, int z) {

	float total = 0;

	int next = x + 1;

	if (x >= DIM_X) {
		return;
	}
	else {

		cilk_spawn calcX(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, next, y, z);

		if (z + 1 == DIM_Z) {
			total += grid_a[0][y][x];
		}
		else {
			total += grid_a[z + 1][y][x];
		}
		if (z - 1 == -1) {
			total += grid_a[DIM_Z-1][y][x];
		}
		else {
			total += grid_a[z - 1][y][x];
		}
		if (y + 1 == DIM_Y) {
			total += grid_a[z][0][x];
		}
		else {
			total += grid_a[z][y + 1][x];
		}
		if (y - 1 == -1) {
			total += grid_a[z][DIM_Y-1][x];
		}
		else {
			total += grid_a[z][y - 1][x];
		}
		if(x + 1 == DIM_X) {
			total += grid_a[z][y][0];
		}
		else {
			total += grid_a[z][y][x + 1];
		}
		if (x - 1 == -1) {
			total += grid_a[z][y][DIM_X-1];
		}
		else {
			total += grid_a[z][y][x - 1];
		}
		total = beta * total;
		total += alpha * grid_a[z][y][x];
		grid_b[z][y][x] = total;
	}
}

void calcY(float alpha, float beta, int DIM_X, int DIM_Y, int DIM_Z, float grid_a[][100][100], float grid_b[][100][100], int x, int y, int z) {
	
	int next = y + 1;
	
	if (y >= DIM_Y) {
		return;
	}
	else {
		
		cilk_spawn calcX(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, x, y, z);
		cilk_spawn calcY(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, x, next, z);
	}
}

void calc(float alpha, float beta, int DIM_X, int DIM_Y, int DIM_Z, float grid_a[][100][100], float grid_b[][100][100], int x, int y, int z) {
	
	int next = z + 1;
	
	if (z >= DIM_Z) {
		return;
	}
	else {
		
		cilk_spawn calcY(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, x, y, z);
		cilk_spawn calc(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, x, y, next);
	}
}

void sweep(float alpha, float beta, int DIM_X, int DIM_Y, int DIM_Z, float	grid_a[][100][100], float grid_b[][100][100], int time) {
	
	if (time == 0) {
		return;
	}
	else {
		
		//run calc first, then send result with recursive call
		cilk_spawn calc(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, 0, 0, 0);
		cilk_sync;
		memcpy(grid_a, grid_b, sizeof(float)*1000000);
		sweep(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, time-1);
	}
}

int main() {

	cout << "Program Start" << endl;

	float alpha = 0.5;
	float beta = 0.0833;
	int const DIM_X = 100;
	int const DIM_Y = 100;
	int const DIM_Z = 100;

	int sweeps = 1000;
	
	float grid_a[DIM_Z][DIM_Y][DIM_X];
	float grid_b[DIM_Z][DIM_Y][DIM_X];
	
	//init values
	for (int i = 0; i < DIM_Z; i++) {
		for (int j = 0; j < DIM_Y; j++) {
			for (int k = 0; k < DIM_X; k++) {
				grid_a[i][j][k] = 20;
			}
		}
	}
	
	//insert warm cube
	for (int i = 80; i < 90; i++) {
		for (int j = 80; j < 90; j++) {
			for (int k = 80; k < 90; k++) {
				grid_a[i][j][k] = 100;
			}
		}
	}
	
	//setting p value
	__cilkrts_set_param("nworkers","8");
	
	//recursive sweep
	sweep(alpha, beta, DIM_X, DIM_Y, DIM_Z, grid_a, grid_b, sweeps);
	
	//output
	//right corner of cube
	for (int y = 0; y < DIM_Y; y++) {
		for (int i = 0; i < DIM_X; i++) {
			cout << trunc(grid_a[DIM_Z-15][y][i]) << " ";
		}
		cout << " ";
		for (int i = 0; i < DIM_Z; i++) {
			cout << trunc(grid_a[i][y][DIM_X-15]) << " ";
		}
		cout << endl;
	}

	return 0;
}
