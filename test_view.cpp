#include <iostream>
#include "Cluster.h"
#include "utils.h"
#include "numerics.h"
#include "View.h"
#include "RandomNumberGenerator.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

typedef boost::numeric::ublas::matrix<double> matrixD;

std::vector<double> extract_row(matrixD data, int row_idx) {
  std::vector<double> row;
  for(int j=0;j < data.size2(); j++) {
    row.push_back(data(row_idx, j));
  }
  return row;
}

void print_cluster_memberships(View& v) {
  std::cout << "Cluster memberships" << std::endl;
  std::set<Cluster<double>*>::iterator it = v.clusters.begin();
  for(; it!=v.clusters.end(); it++) {
    Cluster<double> &cd = **it;
    std::cout << cd.get_global_row_indices() << std::endl;
  }
}

void insert_and_print(View& v, matrixD data,
		      int cluster_idx, int row_idx) {
  std::vector<double> row = extract_row(data, row_idx);
  Cluster<double>& cluster = v.get_cluster(cluster_idx);
  v.insert_row(row, cluster, row_idx);
  std::cout << "v.insert_row(" << row << ", " << cluster_idx << ", " \
	    << row_idx << ")" << std::endl;
  std::cout << "v.get_score(): " << v.get_score() << std::endl;
}

void remove_all_data(View &v, matrixD data) {
  std::set<Cluster<double>*>::iterator it = v.clusters.begin();
  for(; it!=v.clusters.end(); it++) {
    Cluster<double> &cd = **it;
    std::set<int> int_set = cd.get_global_row_indices();
    std::set<int>::iterator it2 = int_set.begin();
    for(; it2!=int_set.end(); it2++) {
      int idx_to_remove = *it2;
      std::vector<double> row = extract_row(data, idx_to_remove);
      v.remove_row(row, cd, idx_to_remove);
    }
  }
  std::cout << "removed all data" << std::endl;
  v.print();
  //
  it = v.clusters.begin();
  it = v.clusters.begin();
  for(; it!=v.clusters.end(); it++) {
    v.remove_if_empty(**it);
  }
  std::cout << "removed empty clusters" << std::endl; 
  v.print();
}

int main(int argc, char** argv) {
  std::cout << std::endl << "Hello World!" << std::endl;

  // load some data
  matrixD data;
  LoadData("test_data.csv", data);

  View v = View(data.size2(), 10);

  int row_idx, cluster_idx;
  row_idx = 0; cluster_idx = 0;
  insert_and_print(v, data, cluster_idx, row_idx);
  row_idx = 1; cluster_idx = 1;
  insert_and_print(v, data, cluster_idx, row_idx);
  row_idx = 2; cluster_idx = 0;
  insert_and_print(v, data, cluster_idx, row_idx);
  //
  row_idx = 3; cluster_idx = 1;
  insert_and_print(v, data, cluster_idx, row_idx);
  row_idx = 4; cluster_idx = 0;
  insert_and_print(v, data, cluster_idx, row_idx);
  row_idx = 5; cluster_idx = 1;
  insert_and_print(v, data, cluster_idx, row_idx);
  v.print();

  print_cluster_memberships(v);

  std::cout << "====================" << std::endl;
  std::cout << "Manually sampling" << std::endl;

  int num_vectors = v.get_num_vectors();
  std::cout << "num_vectors: " << v.get_num_vectors() << std::endl;

  RandomNumberGenerator rng = RandomNumberGenerator();
  for(int iter=0; iter<10; iter++) {
    row_idx = rng.nexti(num_vectors);
    print_cluster_memberships(v);
    std::vector<double> row = extract_row(data, row_idx);
    std::cout << "sampling row_idx: " << row_idx << " :: ";
    std::cout << row << std::endl << std::flush;
    Cluster<double> &cd = *(v.cluster_lookup[row_idx]);
    v.remove_row(row, cd, row_idx);
    // FIXME : make sure calc_cluster_vector_logps gets same order as dict iter
    std::vector<double> cluster_logps = v.calc_cluster_vector_logps(row);
    double rand_u = rng.next();
    std::cout << "cluster_logps: " << cluster_logps << std::endl << std::flush;
    int draw = numerics::draw_sample_unnormalized(cluster_logps, rand_u);
    std::cout << "rand_u: " << rand_u << std::endl;
    std::cout << "draw: " << draw << std::endl;
    Cluster<double>& to_cluster = v.get_cluster(draw);
    v.insert_row(row, to_cluster, row_idx);
    std::cout << "Done iter: " << iter << std::endl;
    std::cout << std::endl;
  }
  
  remove_all_data(v, data);
  v.print();

  std::cout << std::endl << "Goodbye World!" << std::endl;
}
