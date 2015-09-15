#include "src/hsa/page_rank_hsa/page_rank_benchmark.h"

#include <memory>

PageRankBenchmark::PageRankBenchmark() {
  workGroupSize = 64;
  maxIter = 1000;
}

void PageRankBenchmark::InitBuffer() {
  rowOffset = new int[nr + 1]; 
  rowOffset_cpu = new int[nr + 1]; 
  col = new int[nnz];
  col_cpu = new int[nnz];
  val = new float[nnz];
  val_cpu = new float[nnz];
  vector = new float[nr];
  eigenV = new float[nr];
  vector_cpu = new float[nr];
  eigenv_cpu = new float[nr];
}

void PageRankBenchmark::FreeBuffer() {
  csrMatrix.close();
  denseVector.close();
}

void PageRankBenchmark::FillBuffer() {
  FillBufferCpu();
  FillBufferGpu();
}

void PageRankBenchmark::FillBufferGpu() {
}


void PageRankBenchmark::FillBufferCpu() {
  while(!csrMatrix.eof()) {
    for (int j = 0; j < nr+1; j++) {
      csrMatrix >> rowOffset[j];
      rowOffset_cpu[j] = rowOffset[j];
    }
    for (int j = 0; j < nnz; j++) {
      csrMatrix >> col[j];
      col_cpu[j] = col[j];
    }   
    for (int j = 0; j < nnz; j++) {
      csrMatrix >> val[j];
      val[j] = (float)val[j];
      val_cpu[j] = (float)val[j];
    }   
  }
  if(isVectorGiven) {
    while(!denseVector.eof()) {
      for (int j = 0; j < nr; j++) {
        denseVector >> vector[j];
        vector_cpu[j] = vector[j];
        eigenV[j] = 0.0;
        eigenv_cpu[j] = 0.0;
      }
    }
  } else {
    for (int j = 0; j < nr; j++) {
      vector[j] = (float)1/(float)nr;
      vector_cpu[j] = vector[j];
      eigenV[j] = 0.0;
      eigenv_cpu[j] = 0.0;
    }
  }
}

void PageRankBenchmark::ReadCsrMatrix()
{
  csrMatrix.open(fileName1);
  if(!csrMatrix.good()) {
    std::cout << "cannot open csr matrix file" << std::endl;
    exit(-1);
  }
  csrMatrix >> nnz >> nr; 
}

void PageRankBenchmark::ReadDenseVector()
{
  if(isVectorGiven) {
    denseVector.open(fileName2);
    if(!denseVector.good()) {
      std::cout << "Cannot open dense vector file" << std::endl;
      exit(-1);
    }
  }
}

void PageRankBenchmark::PrintOutput() {
  std::cout << std::endl << "Eigen Vector: " << std::endl;
  for (int i = 0; i < nr; i++)
    std::cout << eigenV[i] << "\t";
  std::cout << std::endl;
}

void PageRankBenchmark::Print()
{
  std::cout << "nnz: " << nnz << std::endl;
  std::cout << "nr: " << nr << std::endl;
  std::cout << "Row Offset: " << std::endl;
  for (int i = 0; i < nr+1; i++)
    std::cout << rowOffset[i] << "\t";
  std::cout << std::endl << "Columns: " << std::endl;
  for (int i = 0; i < nnz; i++)
    std::cout << col[i] << "\t";
  std::cout << std::endl << "Values: " << std::endl;
  for (int i = 0; i < nnz; i++)
    std::cout << val[i] << "\t";
  std::cout << std::endl << "Vector: " << std::endl;
  for (int i = 0; i < nr; i++)
    std::cout << vector[i] << "\t";
  std::cout << std::endl << "Eigen Vector: " << std::endl;
  for (int i = 0; i < nr; i++)
    std::cout << eigenV[i] << "\t";
  std::cout << std::endl;
}	

void PageRankBenchmark::ExecKernel() {
  global_work_size[0] = nr * workGroupSize;
  local_work_size[0] = workGroupSize;

  SNK_INIT_LPARM(lparm, 0);
  lparm->ldims[0] = local_work_size[0];
  lparm->gdims[0] = nr * workGroupSize;

  for (int j = 0; j < maxIter; j++) {
    if (j % 2 == 0) {
      pageRank_kernel(nr, rowOffset, col, val, 
          sizeof(float)*64, vector, eigenV,
          lparm);
    } else {
      pageRank_kernel(nr, rowOffset, col, val, 
          sizeof(float)*64, eigenV, vector,
          lparm);
    }
  }
}

void PageRankBenchmark::CpuRun() {
  for(int i = 0; i < maxIter; i++) {
    PageRankCpu();
    if(i != maxIter - 1) {
      for(int j = 0; j < nr; j++) {
        vector_cpu[j] = eigenv_cpu[j];
        eigenv_cpu[j] = 0.0;
      }
    }
  }
}

float* PageRankBenchmark::GetEigenV() {
  return eigenV;
}

void PageRankBenchmark::PageRankCpu() {
  for(int row = 0; row < nr; row++) {
    eigenv_cpu[row] = 0;
    float dot = 0;
    int row_start = rowOffset_cpu[row];
    int row_end = rowOffset_cpu[row+1];

    for(int jj = row_start; jj < row_end; jj++)
      dot += val_cpu[jj] * vector_cpu[col_cpu[jj]];

    eigenv_cpu[row] += dot;
  }
}

void PageRankBenchmark::Initialize() {
  ReadCsrMatrix();
  ReadDenseVector();
  InitBuffer();
  FillBuffer();
}


void PageRankBenchmark::Run()
{
  //Execute the kernel
  ExecKernel();
}

void PageRankBenchmark::Verify() {
  CpuRun();
  for (int i = 0; i < nr; i++) {
    if (abs(eigenv_cpu[i] - eigenV[i]) >= 1e-5) {
      std::cerr << "Not correct!\n";
      std::cerr << "Index: " << i << ", expected: " << eigenv_cpu[i]
        << ", but get: " << eigenV[i] << ", error: "
        << abs(eigenv_cpu[i] - eigenV[i]) << "\n";
    }
  }
}

void PageRankBenchmark::Summarize() {
}

void PageRankBenchmark::Cleanup() {
  FreeBuffer();
}

int PageRankBenchmark::GetLength()
{
  return nr;
}

float PageRankBenchmark::abs(float num)
{
  if (num < 0) {
    num = -num;
  }
  return num;
}


/*
   int main(int argc, char const *argv[])
   {
   uint64_t diff;
   struct timespec start, end;
   if (argc < 2) {
   std::cout << "Usage: pagerank input_matrix [input_vector]" << std::endl;
   exit(-1);
   }
   clock_gettime(CLOCK_MONOTONIC, &start);

   std::unique_ptr<PageRankBenchmark> pr;
   std::unique_ptr<PageRankBenchmark> prCpu;
   if (argc == 2) {
   pr.reset(new PageRankBenchmark(argv[1]));
   prCpu.reset(new PageRankBenchmark(argv[1]));
   pr->Run();
   prCpu->CpuRun();
   }
   else if (argc == 3) {
   pr.reset(new PageRankBenchmark(argv[1], argv[2]));
   prCpu.reset(new PageRankBenchmark(argv[1], argv[2]));
   pr->Run();
   prCpu->CpuRun();
   }
   float* eigenGpu = pr->GetEigenV();
   float* eigenCpu = prCpu->GetEigenV();
   for(int i = 0; i < pr->GetLength(); i++) {
//		if( eigenGpu[i] != eigenCpu[i] ) {
if( pr->abs(eigenGpu[i] - eigenCpu[i]) >= 1e-5 ) {
std::cout << "Not Correct!" << std::endl;
std::cout.precision(20);
std::cout << pr->abs(eigenGpu[i] - eigenCpu[i]) << std::endl;
//std::cout << std::abs(1.23f) << std::endl;
//std::cout << eigenGpu[i] << "\t" << eigenCpu[i] << std::endl;
}
}
clock_gettime(CLOCK_MONOTONIC, &end);

diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
printf("Total elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
return 0;
}
*/
