#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <numa.h>
#include <streambuf>
#include <string>
#include <unistd.h>
#include <chrono>
#include <snappy.h>

using namespace std;

constexpr uint32_t kCompressedFileSize = 507860747;
constexpr uint32_t kNumCompressedFiles = 30;
void *buffers[kNumCompressedFiles - 1];

string read_file_to_string(const string &file_path) {
  ifstream fs(file_path);
  string ret = string((std::istreambuf_iterator<char>(fs)),
                std::istreambuf_iterator<char>());
  fs.close();
  if(ret.empty()){
    printf("Error , file not found!\n");
    exit(1);
  }

  return ret;
}

void write_file_to_string(const string &file_path, const string &str) {
  std::ofstream fs(file_path);
  fs << str;
  fs.close();
}

void uncompress_files_bench(const string &in_file_path,
                            const string &out_file_path) {
  string in_str = read_file_to_string(in_file_path);
  string out_str;

  for (uint32_t i = 0; i < kNumCompressedFiles - 1; i++) {
    buffers[i] = numa_alloc_onnode(kCompressedFileSize, 1);
    if (buffers[i] == nullptr) {
       printf("Failed to alloc\n");
       exit(1);
    }
    memcpy(buffers[i], in_str.data(), in_str.size());
  }

  auto start = chrono::steady_clock::now();
  for (uint32_t i = 0; i < kNumCompressedFiles; i++) {
    std::cout << "Uncompressing file " << i << std::endl;
    if (i == 0) {
      snappy::Uncompress(in_str.data(), in_str.size(), &out_str);
    } else {
      snappy::Uncompress((const char *)buffers[i - 1], kCompressedFileSize,
                         &out_str);
    }
  }
  auto end = chrono::steady_clock::now();
  cout << "Elapsed time in microseconds : "
       << chrono::duration_cast<chrono::microseconds>(end - start).count()
       << " µs" << endl;

  for (uint32_t i = 0; i < kNumCompressedFiles - 1; i++) {
    numa_free(buffers[i], kCompressedFileSize);
  }

  // write_file_to_string(out_file_path, out_str);
}

void do_work(void *arg) {
  uncompress_files_bench("/mnt/enwik9.compressed",
                         "/mnt/enwik9.uncompressed.tmp");
}

int main(int argc, char *argv[]) {
  do_work(NULL);
  return 0;
}
