#include <iostream>
#include <time.h>  
#include <cmath>
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


int send_to_rank(const int my_r_idx, const int my_s_idx, const int round_num, const int root_p);

int receive_from_rank(const int my_r_idx, const int my_s_idx, const int round_num, const int root_p);

int** create_test_matrix(const int &rows, const int &cols);
void print_matrix(int** matrix, const int &rows, const int &cols);
void mm_1d_distributed(int** matrix_a, int** matrix_b, const Dimensions &dims);
void mm_2d_distributed(int** matrix_a, int** matrix_b, const Dimensions &dims);
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
    
    // if (dims.m % p != 0 || dims.n % p != 0 || dims.q % p != 0) {
    //   if (r == 0){
    //     std::cout << "Warning - m,n, or q is not evenly divisible by p.\n";
    //   }
    // }
  }

  int** matrix_a = create_test_matrix(dims.m, dims.n);
  int** matrix_b = create_test_matrix(dims.n, dims.q);

  mm_2d_distributed(matrix_a, matrix_b, dims);


  // mm_1d_distributed(matrix_a, matrix_b, dims);
  
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


void mm_2d_distributed(int** matrix_a, int** matrix_b, const Dimensions &dims) {

  const int r = dims.r;
  const int m = dims.m;
  const int n = dims.n;
  const int q = dims.q;
  const int p = dims.p;
  const int root_p = (int)sqrt(p);

  if (r == 0) {
    std::cout << "matrix_a\n";
    print_matrix(matrix_a, m, n);
    std::cout << "matrix_b\n";
    print_matrix(matrix_b, n, q);
  }

  // This processes' rs grid values. 
  const int r_idx = r / root_p;
  const int s_idx = r % root_p;

  // Row indices from A assigned to processor r.
  const int start_row_a = r_idx * (m/root_p);
  const int end_row_a = (r_idx + 1) * (m/root_p);

  // Column indices from A assigned to processor r. 
  const int start_col_a = s_idx * (n/root_p);
  const int end_col_a = (s_idx + 1) * (n/root_p);
  
  // // Row indices from B assigned to processor r. 
  const int start_row_b = s_idx * (n/root_p);
  const int end_row_b = (s_idx + 1) * (n/root_p);

  // Column indices from B assigned to processor r. 
  const int start_col_b = r_idx * (q/root_p);
  const int end_col_b = (r_idx + 1) * (q/root_p);

  // Assign process r's portion of A.
  int** a_portion = new int*[m/root_p];
  int idx = 0;
  for (int i = start_row_a; i < end_row_a; i++) {
    int* row = new int[n/root_p];
    int row_index = 0;
    for (int j = start_col_a; j < end_col_a; j++) {
      row[row_index++] = matrix_a[i][j];
    }
    a_portion[idx++] = row;
  }

  // Assign process r's portion of B.
  int** b_portion = new int*[n/root_p];
  idx = 0;
  for (int i = start_row_b; i < end_row_b; i++) {
    int* row = new int[q/root_p];
    int row_index = 0;
    for (int j = start_col_b; j < end_col_b; j++) {
      row[row_index++] = matrix_b[i][j];
    }
    b_portion[idx++] = row;
  } 

  // Partial sum matrix.
  int** mtx_c = new int*[m/root_p];
  for (int i = 0; i < m/root_p; i++) {
    mtx_c[i] = new int[q/root_p];
  }

  // Result matrix - not relevant for r != 0. 
  int m_prime = (r == 0) ? m : 0;
  int q_prime = (r == 0) ? q : 0;
  int** result_matrix = new int*[m_prime];
  for (int i = 0; i < m_prime; i++) {
    result_matrix[i] = new int[q_prime];
  }
  
  // Flatten process r's portion of B as a 1d list. 
  int* b_list = new int[n/root_p * q/root_p];
  int flat_idx = 0;
  for (int i = start_col_b; i < end_col_b; i++) {
    for (int j = start_row_b; j < end_row_b; j++) {
      b_list[flat_idx++] = matrix_b[j][i];
    }
  }

    
  int send_buffer[m/root_p * q/root_p];
  int receive_buffer[m/root_p * q/root_p];  
 
  for (int t = 0; t < root_p; t++) {

    if (t != 0) {
      int send_to = send_to_rank(r_idx, s_idx, t, root_p);
      int receive_from = receive_from_rank(r_idx, s_idx, t, root_p);
      MPI_Send(b_list, n/root_p * q/root_p, MPI_INT, send_to, 0, MPI_COMM_WORLD);
      MPI_Recv(receive_buffer, n/root_p * q/root_p, MPI_INT, receive_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Matrix computations.  
    for (int i = 0; i < m/root_p; i++) {
      int buff_idx = 0;
      for (int j = 0; j < q/root_p; j++) {
        int sum = 0;
        for (int k = 0; k < n/root_p; k++) {
          if (t == 0) {
            sum += a_portion[i][k] * b_portion[k][j];
          } else {
            sum += a_portion[i][k] * receive_buffer[buff_idx++];
          }
        }  
        mtx_c[i][j] = sum;
      }
    }

    // Load mtx_c values into send buffer for sending to aggregator process. 
    idx = 0;
    for (int i = 0; i < m/root_p; i++) {
      for (int j = 0; j < q/root_p; j++) {
        send_buffer[idx++] = mtx_c[i][j];
      }
    }

    // Copy master process mtx_c values into result matrix. 
    if (t == 0 && r == 0) {
      for (int i = 0; i < m/root_p; i++) {
        for (int j = 0; j < q/root_p; j++) {
          result_matrix[i][j] = mtx_c[i][j];
        }
      }
    } 

    // Aggregate mtx_c values in aggregator process. 
    int c_proc = (r_idx + t) % root_p;
    int receiver_rank = c_proc + root_p * r_idx;
    if (c_proc == s_idx) {

      for (int u = 0; u < root_p; u++) {
        
        int send_s = (s_idx + u) % root_p;
        int sender_rank = send_s + root_p * r_idx;
        
        if (sender_rank != c_proc + root_p * r_idx) {
          MPI_Recv(receive_buffer, m/root_p * q/root_p, MPI_INT, sender_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 

          // Aggregate values in mtx_c          
          idx = 0;
          for (int i = 0; i < m/root_p; i++) {
            for (int j = 0; j < q/root_p; j++) {
              mtx_c[i][j] += receive_buffer[idx++];
            }
          }
        }
      } 
    } else {
      MPI_Send(send_buffer, m/root_p * q/root_p, MPI_INT, receiver_rank, 0, MPI_COMM_WORLD);
    }

    // // -------------------------------------------------------------------------------------
    // if (receiver_rank == r && r != 0) {
      
    //   // std::cout << " rank sending to 0 to consolidate : " << r << std::endl;

    //   // Flatten mtx_c into send_buffer.
    //   idx = 0;
    //   for (int s = 0; s < m/root_p; s++) {
    //     for (int e = 0; e < q/root_p; e++) {
    //       send_buffer[idx++] = mtx_c[s][e];
    //     }
    //   }
      
    //   MPI_Send(send_buffer, m/root_p * q/root_p, MPI_INT, 0, 0, MPI_COMM_WORLD);

    // }


    // if (r == 0) {
    //   for (int g = 1; g < root_p; g++) {

    //     int received_r = g / root_p;
    //     int received_s = g % root_p;

    //     int start_row_c = received_r * (m/root_p);
    //     int end_row_c = (received_r + 1) * (m/root_p);
    //     int start_col_c = received_s * (q/root_p);
    //     int end_col_c = (received_s + 1) * (q/root_p);
        
    //     std::cout << " 0 receiving from  : " << g << std::endl;

    //     // MPI_Recv(receive_buffer, m/root_p * q/root_p, MPI_INT, g, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    //     idx = 0;
    //     for (int x = start_row_c; x < end_row_c; x++) {
    //       for (int y = start_col_c; y < end_col_c; y++) {
    //         result_matrix[x][y] = receive_buffer[idx++]; 
    //       }
    //     }
    //   }
    // }
    // // -------------------------------------------------------------------------------------



    // -------------------------------------------------------------------------------------
    // else if (r == 0) {
    //   MPI_Recv(receive_buffer, m/root_p * q/root_p, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //   std::cout << "r " << r << " receive buffer " << receive_buffer[0] << " from r " << r << " i " << i << " t " <<  t << std::endl;
    //   // rank to rs grid values. 
    //   int received_r = i / root_p;
    //   int received_s = i % root_p;

    //   int start_row_c = received_r * (m/root_p);
    //   int end_row_c = (received_r + 1) * (m/root_p);
    //   int start_col_c = received_s * (q/root_p);
    //   int end_col_c = (received_s + 1) * (q/root_p);
      
    //   // Deposit values from adjacent threads into result matrix. 
      
    //   idx = 0;
    //   for (int x = start_row_c; x < end_row_c; x++) {
    //     for (int y = start_col_c; y < end_col_c; y++) {
    //       result_matrix[x][y] = receive_buffer[idx++]; 
    //     }
    //   }
      
    //   std::cout << "After consolidating:\n";
    //   print_matrix(result_matrix, m, q);
    //   std::cout << "\n";
      
    // }
    // -------------------------------------------------------------------------------------  
    








    // -------------------------------------------------------------------------------------
    // Flatten mtx_c into send_buffer.
    idx = 0;
    for (int i = 0; i < m/root_p; i++) {
      for (int j = 0; j < q/root_p; j++) {
        send_buffer[idx++] = mtx_c[i][j];
      }
    }

    // Send mtx_c values to consolidate in master process result_matrix. 
    for (int i = 0; i < p; i++) {

      if (r == i) {
        MPI_Send(send_buffer, m/root_p * q/root_p, MPI_INT, 0, 0, MPI_COMM_WORLD);
      }
      
      if (r == 0) {
        
        MPI_Recv(receive_buffer, m/root_p * q/root_p, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
      
        // rank to rs grid values. 
        int received_r = i / root_p;
        int received_s = i % root_p;

        int start_row_c = received_r * (m/root_p);
        int end_row_c = (received_r + 1) * (m/root_p);
        int start_col_c = received_s * (q/root_p);
        int end_col_c = (received_s + 1) * (q/root_p);
        
        // Deposit values from adjacent threads into result matrix. 
        idx = 0;
        for (int i = start_row_c; i < end_row_c; i++) {
          for (int j = start_col_c; j < end_col_c; j++) {
            result_matrix[i][j] = receive_buffer[idx++]; 
          }
        }
      }
    }

    if (r == 0) {
      std::cout << "After consolidating:\n";
      print_matrix(result_matrix, m, q);
      std::cout << "\n";
    }
    // -------------------------------------------------------------------------------------


  }


  if (r == 0) {
    std::cout << "\nmatrix:\n";
    print_matrix(result_matrix, m, q);
  }

  for (int i = 0; i < m_prime; i++) {
    delete [] result_matrix[i];
  }
  delete [] result_matrix;
  
  for (int i = 0; i < m/root_p; i++) {
    delete [] mtx_c[i];
  }
  delete [] mtx_c;
  
  for (int i = 0; i < m/root_p; i++) {
    delete [] a_portion[i];
  }
  delete [] a_portion;

  delete [] b_portion;


}


void mm_1d_distributed(int** matrix_a, int** matrix_b, const Dimensions &dims) {

  const int r = dims.r;
  const int m = dims.m;
  const int n = dims.n;
  const int q = dims.q;
  int p = dims.p;

  if (m < p || q < p) {
    p = std::min(m, q);
  }

  // // Row indexes from A assigned to processor r.
  // int start_a = r * (m/p);
  // int end_a = (r + 1) * (m/p);
  // // Column indexes from B assigned to processor r.
  // int start_b = r * (q/p);
  // int end_b = (r + 1) * (q/p);

  // Row indexes from A assigned to processor r.
  int start_a = (m/p > 0) ? r * (m/p) : r;
  int end_a = (m/p > 0) ? (r + 1) * (m/p) : r + 1;
  // Column indexes from B assigned to processor r.
  int start_b = (q/p > 0) ? (r * (q/p)) : r;
  int end_b = (q/p > 0) ?  (r + 1) * (q/p) : r + 1;


  if (r < p) {

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
}

int send_to_rank(const int my_r_idx, const int my_s_idx, const int round_num, const int root_p) {
  int x_coord = (my_r_idx + round_num) % root_p;
  return x_coord * root_p + my_s_idx; 
}

int receive_from_rank(const int my_r_idx, const int my_s_idx, const int round_num, const int root_p) {
  int x_coord = (my_r_idx - round_num + root_p) % root_p;
  return x_coord * root_p + my_s_idx; 
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