#include <omp.h>
#include <iostream>
#include <time.h>      
#include <chrono>
#include <vector>

typedef std::vector<std::vector<int>> Matrix;

Matrix create_identity_matrix(const int &m, const int &n);
Matrix create_test_matrix(const int &m, const int &n);
Matrix MM_1D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p);
Matrix MM_sequential(const Matrix &mtx_A, const Matrix &mtx_B);
void print_matrix(const Matrix &mtx);
void verify(const Matrix &mtx_C, const Matrix &control_mtx);

const bool TIMER = true;
const bool DEBUG = false;

int main (int argc, char* argv[]) {

  // Please use powers of 2 for all values or it doesn't work. 
  const int m = 512;
  const int n = 512;
  const int q = 256;
  const int p = 8;


  Matrix matrix_A = create_identity_matrix(m, n);
  Matrix matrix_B = create_test_matrix(n, q);
  Matrix matrix_C = MM_1D_parallel(matrix_A, matrix_B, p);
  Matrix matrix_D = MM_sequential(matrix_A, matrix_B);
  verify(matrix_C, matrix_D);


  return 0;

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

  if (TIMER) {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Sequential execution time: " << duration.count() << " seconds" << std::endl;
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
    
    for (int i = 0; i < m/p; i ++) {
      for (int j = 0; j < q; j++) {
        int sum = 0;
        for (int k = 0; k < n; k++) {
          sum += mtx_A[i + r*(m/p)][k] * mtx_B[k][(j + r) % q];
        }
        result[i + r*(m/p)][(j + r) % q] = sum;
      }
    }
  }

  if (TIMER) {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Parallel execution time: " << duration.count() << " seconds" << std::endl;
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


Matrix create_identity_matrix(const int &m, const int &n) {
  Matrix result(m, std::vector<int>(n));
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      result[i][j] = (i == j ? 1 : 0);
    }
  }
  return result;
}

Matrix create_test_matrix(const int &m, const int &n) {
  Matrix result(m, std::vector<int>(n));
  srand (time(NULL));
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