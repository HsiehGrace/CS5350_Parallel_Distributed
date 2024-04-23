#include <iostream>
#include <time.h>  
#include "mpi.h"

struct Dimensions {
  int m, n, q, p, r;
};

struct Matrix {

  int rows, cols;
  int **matrix_array = new int*[rows];

  ~Matrix() {
    for (int i = 0; i < rows; i++) {
      delete [] matrix_array[i];
    }
    delete [] matrix_array;
  }

};

int** create_test_matrix(const int &rows, const int &cols);
void print_matrix(int** matrix, const int &rows, const int &cols);
void mm_1d_distributed(int** matrix_a, int** matrix_b, const Dimensions &dims);
int** mm_sequential(int **matrix_a, int** matrix_b, const Dimensions &dim);
void reshape_matrix (const int &n, const int &q, const int &p, int** reshaped_b_matrix, int* receive_buffer);
int* flatten_matrix(const int &n, const int &q, const int &p, const int &start, const int &end, int** matrix_to_flatten);
void verify(int** test_matrix, int** sequential_matrix, const int &rows, const int &cols);

int main(int argc, char* argv[]) {

  srand (time(NULL));

  Dimensions dims;

  MPI_Init(&argc, &argv);

  int p;
	MPI_Comm_size(MPI_COMM_WORLD, &p);

  int r;
	MPI_Comm_rank(MPI_COMM_WORLD, &r);

  if (argc != 4) {
    if (r == 0){
      std::cerr << "Incorrect number of command line arguments.\n";
      MPI_Finalize();
      return 1;
    }
  } else {
    dims.m = std::atoi(argv[1]);
    dims.n = std::atoi(argv[2]);
    dims.q = std::atoi(argv[3]);
    dims.p = p;
    dims.r = r;
    

    if (dims.m % p != 0 || dims.n % p != 0 || dims.q % p != 0) {
      if (r == 0){
        std::cerr << "m,n, and q must be evenly divisible by p\n";
        MPI_Finalize();
        return 1;
      }
    }
  }

  int** matrix_a = create_test_matrix(dims.m, dims.n);
  int** matrix_b = create_test_matrix(dims.n, dims.q);

  mm_1d_distributed(matrix_a, matrix_b, dims);
  
  for (int i = 0; i < dims.m; i++) {
    delete [] matrix_a[i];
  }
  delete [] matrix_a;
  
  for (int i = 0; i < dims.n; i++) {
    delete [] matrix_b[i];
  }
  delete [] matrix_b;  

  MPI_Finalize();

  return 0;
}

void mm_1d_distributed(int** matrix_a, int** matrix_b, const Dimensions &dims) {

  const int r = dims.r;
  const int m = dims.m;
  const int n = dims.n;
  const int q = dims.q;
  const int p = dims.p;

  // Row indexes from A assigned to processor r.
  int start_a = r * (m/p);
  int end_a = (r + 1) * (m/p);
  // Column indexes from B assigned to processor r.
  int start_b = r * (q/p);
  int end_b = (r + 1) * (q/p);

  // Allocate process r's portion of matrix_a.
  int** a_portion = new int* [m/p];
  int a_idx = 0;
  for (int b = start_a; b < end_a; b++) {
    int* row = new int[n];
    for (int h = 0; h < n; h++) {
      row[h] = matrix_a[b][h];
    }
    a_portion[a_idx++] = row;
  } 

  // Allocate memory for mtx_c.
  int** mtx_c = new int*[m];
  for (int w = 0; w < m; w++) {
    mtx_c[w] =  new int[q];
  }

  int* b_portion =  new int[n * q/p];
  int flat_index = 0;
  for (int j = start_b; j < end_b; j++) {
    for (int i = 0; i < n; i++) {
      b_portion[flat_index++] = matrix_b[i][j];   
    }
  }

  // Buffer that will receive data from other processes. 
  int receive_buffer[n * q/p];
  
  for (int t = 0; t < p; t++) {

    int send_to = (r + t) % p;
    int receive_from = (r - t + p) % p;

    if (t != 0) {
      MPI_Send(b_portion, n * q/p, MPI_INT, send_to, 0, MPI_COMM_WORLD);
      MPI_Recv(receive_buffer, n * q/p, MPI_INT, receive_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // Matrix multiplication with matrix_b values - in receive buffer.
    for (int i = 0; i < m/p; i++) {
      int idx = 0;
      for (int j = 0; j < q/p; j++) {
        int sum = 0;
        for (int k = 0; k < n; k++) {
          if (t == 0) {
            sum += a_portion[i][k] * matrix_b[k][j + start_b];
          } else {
            sum += a_portion[i][k] * receive_buffer[idx++];
          }
        }
        mtx_c[start_a + i][j + receive_from * q/p] = sum;
      }
    }
  }
  for (int x = 0; x < m/p; x++) {
    delete [] a_portion[x];
  }
  delete [] a_portion;
  delete [] b_portion;

  // Flatten this processes values / portion of matrix_c.
  int* buffer_list = new int[m/p * q];
  int idx = 0;
  for (int i = r * m/p; i < (r + 1) * m/p; i++) {
    for (int j = 0; j < q; j++) {
      buffer_list[idx++] = mtx_c[i][j];
    }
  }

  // Aggregate all processes values in process 0's matrix_c.
  for (int t = 1; t < p; t++) {
    if (r == t) {
      MPI_Send(buffer_list, m/p * q, MPI_INT, 0, 0, MPI_COMM_WORLD);
      
    }
    if (r == 0) {
      MPI_Recv(buffer_list, m/p * q, MPI_INT, t, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int indx = 0;
      for (int i = t * m/p; i < (t+1) * m/p; i++) {
        for (int j = 0; j < q; j++) {
          mtx_c[i][j] = buffer_list[indx++];
        }
      }
    }
  }
  delete [] buffer_list;


  if (r == 0) {
    int **sequential = new int*[m];
    sequential = mm_sequential(matrix_a, matrix_b, dims);
    verify(mtx_c, sequential, m, q);
    for (int i = 0; i < m; i++) {
      delete [] sequential[i];
    }
    delete [] sequential;
  }
  for (int w = 0; w < m; w++) {
    delete [] mtx_c[w];
  }
  delete mtx_c;
}


// Not needed + incorrect. 
int* flatten_matrix(const int &n, const int &q, const int &p, const int &start, const int &end, int** matrix_to_flatten) {

  // Flatten process r's portion of matrix_b into a 1d array to send.
  int* flattened_matrix =  new int[n * q/p];
  int flat_index = 0;
  for (int j = 0; j < n; j++) {
    for (int i = start; i < end; i++) {
      flattened_matrix [flat_index++] = matrix_to_flatten[j][i];   
    }
  }

  return flattened_matrix;
}

// Not needed + time consuming + inefficient. 
void reshape_matrix (const int &n, const int &q, const int &p, int** reshaped_b_matrix, int* receive_buffer) {

  int idx = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < q/p; j++) {
      reshaped_b_matrix[i][j] = receive_buffer[idx++];
    }
  }
}


int** create_test_matrix(const int &rows, const int &cols) {
  int** matrix = new int*[rows];
  int idx = 0;
  for (int i = 0; i < rows; i++) {
    int* row = new int[cols];
    for (int j = 0; j < cols; j++){
      row[j] = rand() % 5 + 1;
    }
    matrix[i] = row;
  }
  return matrix;
}


void print_matrix(int** matrix, const int &rows, const int &cols) {

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++){
      std::cout << matrix[i][j] << " ";
    }
    std::cout << std::endl;
  }
}


int** mm_sequential(int **matrix_a, int** matrix_b, const Dimensions &dim) {

  int** matrix_c = new int*[dim.m];
  for (int i = 0; i < dim.m; i++) {
    int* row = new int[dim.q];
    for (int j = 0; j < dim.q; j++){
      int sum = 0;
      for (int k = 0; k < dim.n; ++k) {
        sum += matrix_a[i][k] * matrix_b[k][j];
      }
      row[j] = sum;
    }
    matrix_c[i] = row;
  }

  return matrix_c;
}

void verify(int** test_matrix, int** sequential_matrix, const int &rows, const int &cols) {
  
  int errors = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++){
      if (test_matrix[i][j] != sequential_matrix[i][j]) {
        errors++;
      }
    }
  }
  if (errors > 0) {
    std::cout << "Fail\n";
  }
  else{
    std::cout << "Pass\n";
  }
}