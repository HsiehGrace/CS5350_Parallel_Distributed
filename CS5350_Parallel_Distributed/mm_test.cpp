#include <omp.h>
#include <iostream>
#include <time.h>      
#include <chrono>
#include <vector>
#include <cmath>
#include <fstream>
#include <iomanip>

typedef std::vector<std::vector<int>> Matrix;

struct Result {
    Matrix matrix;
    std::chrono::duration<double> execution_time;
};

Matrix create_identity_matrix(const int &m, const int &q);
Matrix create_test_matrix(const int &m, const int &n);
Result MM_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p);
Result MM_1D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p);
Result MM_2D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &procs);
Result MM_sequential(const Matrix &mtx_A, const Matrix &mtx_B);
void print_matrix(const Matrix &mtx);
bool verify(const Matrix &mtx_C, const Matrix &control_mtx);


const bool TIMER = true;
const bool DEBUG = false;

int main (int argc, char* argv[]) {

  srand (time(NULL));
  
  // m, n, q must be evenly divisible by p.
  // Use perfect squares for p. 
  // p must be <= m, n, q
  int m = 100;
  int n = 100;
  int q = 100;
  int p = 4;
  
  std::ofstream outputFile("output.txt");
  if (!outputFile.is_open()) {
    std::cerr << "Error opening file." << std::endl;
    return 1; 
  }
  
  outputFile << std::left 
  << std::setw(10) << "m"
  << std::setw(10) << "n" 
  << std::setw(10) << "q" 
  << std::setw(10) << "p" 
  << std::setw(20) << "Time_2D"
  << std::setw(20) << "Erors_2D"
  << std::setw(20) << "Time_1D"
  << std::setw(20) << "Errors_1D"
  << std::setw(20) << "Time_auto-parallel"
  << std::setw(30) << "Errors_auto-parallel"
  << std::setw(20) << "Time_sequential" << std::endl;


  for (int i = 0; i < 10; i++) {
    Matrix matrix_A = create_test_matrix(m, n);
    Matrix matrix_B = create_test_matrix(n, q);
    Result control_matrix = MM_sequential(matrix_A, matrix_B);

    std::cout <<"\n2D parallel:\n";
    Result parallel_2d = MM_2D_parallel(matrix_A, matrix_B, p);
    std::cout <<"\n1D parallel:\n";
    Result parallel_1d = MM_1D_parallel(matrix_A, matrix_B, p);
    std::cout <<"\nAuto-parallel:\n";
    Result parallel = MM_parallel(matrix_A, matrix_B, p);
    
    int errors_2d = verify(parallel_2d.matrix, control_matrix.matrix);
    int errors_1d = verify(parallel_1d.matrix, control_matrix.matrix);
    int errors_parallel = verify(parallel.matrix, control_matrix.matrix);

    outputFile << std::left 
    << std::setw(10) << m 
    << std::setw(10) << n 
    << std::setw(10) << q 
    << std::setw(10) << p 
    << std::setw(20) << parallel_2d.execution_time.count()
    << std::setw(20) << errors_2d
    << std::setw(20) << parallel_1d.execution_time.count()
    << std::setw(20) << errors_1d
    << std::setw(20) << parallel.execution_time.count()
    << std::setw(30) << errors_parallel
    << std::setw(20) << control_matrix.execution_time.count() << std::endl;

    // Manipulate these values.
    // m, n, q must be evenly divisible by p.
    // Use perfect squares for p. 
    // p must be <= m, n, q
    m += 200;
    n += 200;
    q += 200;
    
  }

  outputFile.close();

  return 0;

}


Result MM_2D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &procs) {

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

  Result res;
  res.matrix = result;
  res.execution_time = end - start;

  return res;
}


Result MM_1D_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p) {

  int m = mtx_A.size();
  int n = mtx_B.size();
  int q = mtx_B[0].size();
  Matrix result(m, std::vector<int>(q));

  omp_set_num_threads(p);
  
  auto start = std::chrono::high_resolution_clock::now();
  
  #pragma omp parallel 
  {
    int r = omp_get_thread_num();

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

  Result res;
  res.matrix = result;
  res.execution_time = end - start;

  return res;
}

Result MM_parallel(const Matrix &mtx_A, const Matrix &mtx_B, const int &p) {

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

  Result res;
  res.matrix = result;
  res.execution_time = end - start;

  return res;
}


Result MM_sequential(const Matrix &mtx_A, const Matrix &mtx_B) {
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

  Result res;
  res.matrix = result;
  res.execution_time = end - start;

  return res;
}


bool verify(const Matrix &mtx_C, const Matrix &control_mtx) {
  int errors = 0;
  int m = mtx_C.size();
  int n = mtx_C[0].size();

  #pragma omp parallel for
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
  
  if (DEBUG) {
    std::cout << "Test complete. " << errors << " errors found.\n";
  }

  return errors;
}


Matrix create_identity_matrix(const int &m, const int &q) {
  Matrix result(m, std::vector<int>(q));
  #pragma omp parallel for
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < q; j++) {
      result[i][j] = (i == j ? 1 : 0);
    }
  }

  return result;
}

Matrix create_test_matrix(const int &m, const int &n) {
  Matrix result(m, std::vector<int>(n));
  
  #pragma omp parallel for
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