#include <omp.h>
#include <iostream>
#include <time.h>      
#include <chrono>
#include <vector>
#include <cmath>

typedef std::vector<std::vector<int>> Matrix;

Matrix create_identity_matrix(const int &m, const int &q);
Matrix create_test_matrix(const int &m, const int &n);
Matrix MM_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p);
Matrix MM_1D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p);
Matrix MM_2D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &procs);
Matrix MM_sequential(const Matrix &mtx_A, const Matrix &mtx_B);
void print_matrix(const Matrix &mtx);
void verify(const Matrix &mtx_C, const Matrix &control_mtx);

const bool TIMER = true;
const bool DEBUG = false;

int main (int argc, char* argv[]) {

  srand (time(NULL));
  
  // m, n, q must be evenly divisible by p.
  // Use perfect squares for p. 
  // p must be <= m, n, q
  const int m = 2048;
  const int n = 900;
  const int q = 900;
  const int p = 4;

  Matrix matrix_A = create_test_matrix(m, n);
  Matrix matrix_B = create_test_matrix(n, q);
  Matrix control_matrix = MM_sequential(matrix_A, matrix_B);

  std::cout <<"\n2D parallel:\n";
  Matrix parallel_2d = MM_2D_parallel(matrix_A, matrix_B, p);
  verify(parallel_2d, control_matrix);

  std::cout <<"\n1D parallel:\n";
  Matrix parallel_1d = MM_1D_parallel(matrix_A, matrix_B, p);
  verify(parallel_1d, control_matrix);

  std::cout <<"\nAuto-parallel:\n";
  Matrix parallel = MM_parallel(matrix_A, matrix_B, p);
  verify(parallel, control_matrix);

  return 0;

}


Matrix MM_2D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &procs) {

  omp_set_num_threads(procs);

  int m = mtx_A.size();
  int n = mtx_B.size();
  int q = mtx_B[0].size();
  int root_p = (int)sqrt(procs);

  Matrix result(m, std::vector<int>(q, 0));
  
  auto start = std::chrono::high_resolution_clock::now();

  #pragma omp parallel
  {
    int r = omp_get_thread_num();

    int r_idx = r / root_p;
    int s_idx = r % root_p; 

    int start_row_A = r_idx * m / root_p;
    int end_row_A = (r_idx + 1) * m / root_p;
    int start_col_A =  s_idx * n / root_p;
    int end_col_A = (s_idx + 1) * n / root_p;

    int start_row_B = s_idx * n / root_p;
    int end_row_B = (s_idx + 1) * n / root_p;
    int start_col_B =  r_idx * q / root_p;
    int end_col_B = (r_idx + 1) * q / root_p;


    for (int i = start_row_A; i < end_row_A; i++) {

      for (int j = start_col_B; j < start_col_B + q; j++) {
        int sum = 0;
        for (int k = start_col_A; k < end_col_A; k++) {
          sum += mtx_A[i][k] * mtx_B[k][j % q];
        }
        #pragma omp critical
        {
          result[i][j % q] += sum;
        }
      }
    }

  }
  
  auto end = std::chrono::high_resolution_clock::now();
  if (TIMER) {
    std::chrono::duration<double> duration = end - start;
    std::cout << "Parallel execution time: " << duration.count() << " seconds" << std::endl;
  } 

  return result;
}


Matrix MM_1D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p) {

  int m = mtx_A.size();
  int n = mtx_B.size();
  int q = mtx_B[0].size();
  Matrix result(m, std::vector<int>(q));

  omp_set_num_threads(p);
  
  auto start = std::chrono::high_resolution_clock::now();
  
  #pragma omp parallel 
  {
    int r = omp_get_thread_num();

    // #pragma omp critical 
    // {
    //   std::cout << "Hello from thread : " << r << "\n";
    // }

    int start_A = r * (m/p);
    int end_A = (r + 1) * (m/p);
    int start_B = r * (q/p);

    for (int i = start_A; i < end_A; i++) {
      for (int j = 0; j < q; j++) {
        int sum = 0;
        for (int k = 0; k < n; k++) {
          sum += mtx_A[i][k] * mtx_B[k][(start_B + j) % q];
        }
        result[i][(start_B + j) % q] = sum;
      }
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  if (TIMER) {
    std::chrono::duration<double> duration = end - start;
    std::cout << "Parallel execution time: " << duration.count() << " seconds" << std::endl;
  }
  return result;
}

Matrix MM_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p) {

  int m = mtx_A.size();
  int n = mtx_B.size();
  int q = mtx_B[0].size();
  Matrix result(m, std::vector<int>(q, 0));
  
  auto start = std::chrono::high_resolution_clock::now();
  
  #pragma omp parallel for
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < q; ++j) {
      int sum = 0;
      for (int k = 0; k < n; ++k) {
        sum += mtx_A[i][k] * mtx_B[k][j];
      }
      result[i][j] = sum;
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  if (TIMER) {  
    std::chrono::duration<double> duration = end - start;
    std::cout << "Parallel execution time: " << duration.count() << " seconds" << std::endl;
  }
  return result;
}



Matrix MM_sequential(const Matrix &mtx_A, const Matrix &mtx_B) {
  int m = mtx_A.size();
  int n = mtx_B.size();
  int q = mtx_B[0].size();
  Matrix result(m, std::vector<int>(q, 0));
  
  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < q; ++j) {
      int sum = 0;
      for (int k = 0; k < n; ++k) {
        sum += mtx_A[i][k] * mtx_B[k][j];
      }
      result[i][j] = sum;
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  if (TIMER) {  
    std::chrono::duration<double> duration = end - start;
    std::cout << "Sequential execution time: " << duration.count() << " seconds" << std::endl;
  }


  return result;
}


void verify(const Matrix &mtx_C, const Matrix &control_mtx) {
  int errors = 0;
  int m = mtx_C.size();
  int n = mtx_C[0].size();
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      if (mtx_C[i][j] != control_mtx[i][j]) {
        if (DEBUG) {
          std::cout << "Failed at [" << i << "][" << j << "]\n";
        }
        errors++;
      }
    }
  }
  std::cout << "Test complete. " << errors << " errors found.\n";
}


Matrix create_identity_matrix(const int &m, const int &q) {
  Matrix result(m, std::vector<int>(q));
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < q; j++) {
      result[i][j] = (i == j ? 1 : 0);
    }
  }
  return result;
}

Matrix create_test_matrix(const int &m, const int &n) {
  Matrix result(m, std::vector<int>(n));
  
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      result[i][j] = rand() % 5 + 1;
    }
  }
  return result;
}

void print_matrix(const Matrix &mtx) {
  for (int i = 0; i < mtx.size(); i++) {
    for (int j = 0; j < mtx[0].size(); j++) {
      std::cout << mtx[i][j] << " ";
    }
    std::cout << "\n";
  }
}