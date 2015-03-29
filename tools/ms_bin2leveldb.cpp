// ms_bin2leveldb.cpp : Defines the entry point for the console application.
//
#include <google/protobuf/text_format.h>
#include <glog/logging.h>
#include <leveldb/db.h>

#include <stdint.h>
#include <fstream>  // NOLINT(readability/streams)
#include <string>
#include <iostream>
#include "caffe/proto/caffe.pb.h"
#include <iterator>
#include <algorithm>
#include <sstream>
#include <ctime> // for random generator initialization
#include <boost/filesystem.hpp>

#include <ctime> // for random generator initialization
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>
using namespace std;

namespace fs = boost::filesystem;
namespace rnd = boost::random;

typedef struct
{
  string line;
  int label;
} line_label;

struct less_than_line_label
{
  inline bool operator() (const line_label& s1, const line_label& s2)
  {
    return s1.line < s2.line;
  }
};

const float training_probability = 0.8;
rnd::mt19937_64 gen(std::time(0));
double probs [] = {1.0 - training_probability, training_probability};
rnd::discrete_distribution<> dist(probs);

// is this example going to go to the training or validation dataset?
bool IsTrainingExample()
{
  return dist(gen) == 1;
}

unique_ptr<leveldb::DB> OpenDb(const char * db_dir)
{
  // if the database already exists - remove it
  if(fs::exists(db_dir))
  {
    fs::remove_all(db_dir);
  }

  unique_ptr<leveldb::DB> db;
  leveldb::DB *pdb;
  leveldb::Options options;
  options.create_if_missing = true;
  options.error_if_exists = true;
  leveldb::Status status = leveldb::DB::Open(options, db_dir, &pdb);

  CHECK(status.ok()) << "Failed to open leveldb " << db_dir << "Already exists?";
  db.reset(pdb);
  return db;
}

size_t max_file_size(const char * image_dir)
{
  fs::directory_iterator enddir;
  int size = 0;
  for(auto it = fs::directory_iterator(image_dir); it != enddir; it++)
  {
    size_t newSize =fs::file_size(it->path());
    if(newSize > size)
    {
      size = newSize;
    }
  }
  return size;
}

void convert_ms_dataset(const char * image_dir, const char * label_filename, const char * db_train_dir, const char * db_validation_dir)
{
  try
  {
    if (fs::exists(image_dir) && fs::exists(label_filename))
    {
      clock_t begin = clock();

      fs::directory_iterator enddir;

      auto db_train = OpenDb(db_train_dir);
      auto db_valiation = OpenDb(db_validation_dir);

      // caffe init
      caffe::Datum datum;
      datum.set_channels(1);
      datum.set_height(1);

      // read label file
      ifstream label(label_filename);
      cout<< "Reading label file..."<<endl;
      string line;
      vector<string> lable_lines;
      vector<line_label> labels;

      int i = 0;
      while(getline(label, line))
      {
        stringstream   lineStream(line);
        string         cell;
        if (i == 0)
        {
          i++;
          continue;
        }

        vector<string> label_val;
        while(getline(lineStream, cell, ','))
        {
          label_val.push_back(cell);
        }

        line_label this_label;
        this_label.line = label_val[0].substr(1, label_val[0].size() - 2);
        this_label.label = stoi(label_val[1]);
        labels.push_back(this_label);
      }
      label.close();

      cout << "Finished reading labels" << endl;

      sort(labels.begin(), labels.end(), less_than_line_label());
      cout << "Sorted labels" << endl;

      cout << "Starting to read binary directory" << endl;
      int train_ex = 0;
      int test_ex = 0;
      int max_size = max_file_size(image_dir);
      datum.set_width(max_size);

      cout << "Max size: " << max_size << endl;

      for(auto it = fs::directory_iterator(image_dir); it != enddir; it++)
      {
        string stem = it->path().stem().string();
        string name = it ->path().filename().string();
        string fpath = it -> path().string();

        ifstream bin_file(fpath, ios::in | ios::binary);
        if (bin_file.is_open())
        {
          // read it
          uintmax_t size = fs::file_size(fpath);
          unique_ptr<char []> mem_block(new char [max_size]);
          memset(mem_block.get(), 0xFF, max_size);

          cout << "Read: " << size << " bytes from " << name;
          bin_file.read(mem_block.get(), size);
          cout << "Padding: " << max_size - size << " bytes";
          bin_file.close();

          // match to its label
          auto i_label = find_if(labels.begin(), labels.end(), [stem](const line_label& a) {return a.line == stem;});
          cout << ", label: " << i_label->label;

          //attach to leveldb
          string value;
          datum.set_data(mem_block.get(), (size_t) max_size);
          datum.set_label(i_label->label);
          datum.SerializeToString(&value);
          // depending on what we are using this data for...
          bool isTraining = IsTrainingExample();
          unique_ptr<leveldb::DB>& db = isTraining ? db_train : db_valiation;
          cout << " -> " << (isTraining ? " train" : " validation") << endl;
          db->Put(leveldb::WriteOptions(), i_label->line.c_str(), value);
          if (isTraining) train_ex++;
          else
            test_ex++;
        }
      }
      cout << "Training: " << train_ex << " Test: " << test_ex << " Max size: " << max_size << endl;
      ofstream totals(".\\totals.txt");
      totals << "Training: " << train_ex << " Test: " << test_ex << " Max size: " << max_size << endl;
      totals.close();

      clock_t end = clock();
      double elapsed = double (end - begin) /CLOCKS_PER_SEC;

      cout << "Elapsed: " << elapsed / 60.0 << " min." << endl;
    }
    else
    {
      cout << "Check that label file and examples directory exist" << endl;
    }
  }
  catch(const fs::filesystem_error& ex)
  {
    cout << ex.what() << endl;
  }
}

int main(int argc, char* argv[])
{
  if (argc != 5) {
    printf("This script converts the MNIST dataset to the leveldb format used\n"
           "by caffe to perform classification.\n"
           "Usage:\n"
           "    convert_ms_data input_bins_dir input_label_file train_db_dir validation_db_dir\n");
  } else {
    convert_ms_dataset(argv[1], argv[2], argv[3], argv[4]);
  }
}

