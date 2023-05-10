/* File:     mpi_trap1.c
 * Compile:  mpicc -g -Wall -o mpi_trap1 mpi_trap1.c
 * Run:      mpiexec -n <number of processes> ./mpi_trap1
 */
#include <stdio.h>
#include <mpi.h>

/* To run:
            $ mpicc -o mpi_trap1 mpi_trap1.c
            $ mpirun -np 4 ./mpi_trap1
*/


// Defining the trapezoidal integration function and the target function to integrate.
double Trap(double left_endpt, double right_endpt, int trap_count, double base_len);
double f(double x); 

int main(void) {
   int my_rank, comm_sz, n = 1024, local_n, source;   
   double a = 0.0, b = 3.0, h, local_a, local_b, local_int, total_int;

   MPI_Init(NULL, NULL);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

   // Finding out how many processes are being used
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
   
   // Defining 'h', which is the base length of each trapezoid in the integration approximation.
   h = (b - a) / n;

   // Defining 'local_n' which is the number of trapezoids each process will compute.
   local_n = n / comm_sz;

   local_a = a + my_rank * local_n * h;         // 'local_a' is the lower limit of the integral.
   local_b = local_a + local_n * h;             // 'local_b' is the upper limit of the integral.
   
   // 'local_int' is the estimate of the integral over the local interval.
   local_int = Trap(local_a, local_b, local_n, h);

   // Sending local integral to process 0.
   if (my_rank != 0)
      MPI_Send(&local_int, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

   // Collecting local integrals and compute total integral.
   else {
      total_int = local_int;
      for (source = 1; source < comm_sz; source++) {
         MPI_Recv(&local_int, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         total_int += local_int;
      }
   }

   // Printing the final result from process 0
   if (my_rank == 0) {
      printf("With n = %d trapezoids, our estimate\n", n);
      printf("of the integral from %f to %f = %.15e\n", a, b, total_int);
   }

   MPI_Finalize();
   return 0;
}


double Trap(double left_endpt, double right_endpt, int trap_count, double base_len) {
   // This function approximates the integral of pre-defined 'f()' using the trapezoidal rule.
   double estimate, x; 
   int i;

   // Initializing 'estimate' to the average value of the function 'f()' at the left and right endpoints of the local interval.
   estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
   
   // Computing the midpoint of each trapezoid to approximate the value of the function being integrated.
   for (i = 1; i <= trap_count-1; i++) {
      x = left_endpt + i * base_len;         // Using left endpoint because trapezoidal method uses so.
      // Summing it with 'i * base_len' is to obtain the correct x-coordinate for each trapezoid.
      estimate += f(x);
   }

   // Scaling the estimation and finding the area by multiplying the height approximation with width.
   estimate = estimate * base_len;

   return estimate;
}


double f(double x) {
   // This function returns the square of the input, target function to integrate.
   return x*x;
}
