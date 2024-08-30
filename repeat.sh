#!/bin/bash

# 定义数组
nums=(2 4 8 16 32 64 128 256 512 1024 2048 4096)
# nums=(8192)
ngram=(2 4 8 16 32 64 128 256 512 1024)
TARGET_DIR="/home/weiren/ML_experiment_all_mal/testing_gram/"
# 循环遍历数组
for i in "${nums[@]}"
# for j in "${ngram[@]}"
do
   for j in "${ngram[@]}"
  #  for i in "${nums[@]}"
   do
    # 检查restrict_depth是否不大于ngram
        rm -r "$TARGET_DIR"
        mkdir -p "$TARGET_DIR"
        echo "Create Tree data len = $i , n-gram = $j:" >> cout_log.txt
        sed -i "s/int create_tree_data_len = [0-9]*;/int create_tree_data_len = $i;/" "suffix_bool_map_int_0201(diff_set)_vector.cpp"
        # sed -i "s/int testing_data_len = [0-9]*;/int testing_data_len = $i;/" "suffix_bool_map_int_0201(diff_set)_vector.cpp"
        sed -i "s/int restrict = [0-9]*;/int restrict = $j;/" "SuffixTree_bool_map_int_0201(diff_set)_vector.h"
        # 编译程序
        g++ "suffix_bool_map_int_0201(diff_set)_vector.cpp" -o program
        # 执行程序
      #   ./program
        rm -f ./massif.out.*
        valgrind --tool=massif ./program
        # valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./program
        python "/home/weiren/extract_memory.py" >> cout_log.txt
        python "/home/weiren/ML_experiment/n_gram_detector.py" >> cout_log.txt
   done
done
