#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <climits>
#include <fstream>
#include <sstream>
#include <set>
#include <unordered_set>
#include <ctime>
#include <dirent.h>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <list>
#include <string>
#include "suffix_tree_function.h"
using namespace std;

// 將opcode mapping 成一個integer
void create_table(SuffixTree *st, stringstream &ss)
{
    while (!ss.fail())
    {
        string temp;
        ss >> temp;
        if (temp.empty())
            continue;
        if (table_num[temp] == 0)
        {
            table_num[temp] = table_num.size() + 1;
            table_name[table_num[temp]] = temp;
        }
        st->number_sequence.push_back(table_num[temp]);
    }
    return;
}

// 將資料夾裡面所有文件的名稱都讀進來
vector<string> getFilesList()
{
    vector<string> allpath;
    DIR *directory_pointer;
    struct dirent *entry;
    directory_pointer = opendir("/home/weiren/mal_opcode/");
    while ((entry = readdir(directory_pointer)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        allpath.push_back(entry->d_name);
    }
    closedir(directory_pointer);

    // 用一組亂數決定文件的順序
    vector<string> shuffleAllpath = allpath;
    mt19937 rng(42);
    shuffle(shuffleAllpath.begin(), shuffleAllpath.end(), rng);
    for (size_t i = 0; i < sortedAllpath.size(); ++i)
    {
        // sortedAllpath[i] = allpath[sortOrder[i]];
        sortedAllpath[i] = allpath[i];
    }
    return shuffleAllpath;
}

ofstream log_file("/home/weiren/cout_log.txt", std::ios::app); // debug用 把所有的cout寫到這個文件裡面(方便觀察結果)
int create_tree_data_len = 4;                                  // 建樹文件數量
int testing_data_len = 1500;                                   // 搜尋文件數量
int main()
{
    clock_t a, b;
    ios::sync_with_stdio(false);
    cin.tie(0);
    ifstream myFile;
    char *path = "/home/weiren/mal_opcode/";
    vector<string> allPath = getFilesList();
    int document_size = create_tree_data_len;
    SuffixTree *st = new SuffixTree(document_size); // 建立suffix tree
    int seq_start = 0;
    for (int i = 0; i < create_tree_data_len; i++)
    {
        double START, END;
        if (i % 1000 == 0)
            cout << "already handle " << i << " sample" << endl;
        myFile.open(path + allPath[i]);
        stringstream ss;
        ss << myFile.rdbuf();
        myFile.close();
        create_table(st, ss);
        st->st_build(i, seq_start, document_size);
        seq_start = st->number_sequence.size();
        st->st_continue_initial();
    }

    cout << "In the Create Suffix_Tree Phase need to cost= " << (double)clock() / CLOCKS_PER_SEC << "s" << endl;
    log_file << "In the Create Suffix_Tree Phase need to cost= " << (double)clock() / CLOCKS_PER_SEC << "s" << endl;
    a = clock();
    st->add_visit_idx(st->root);
    b = clock();
    cout << "In the compensate visit doc_idx Phase need to cost= " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;

    log_file << "In the compensate visit doc_idx Phase need to cost= " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;

    // --------------------------------------------- Hash Vec/Bitarray area ---------------------------------------------//

    // ========================= bit_array 用法 ========================= //
    // vector<ull> prev_vector(document_size / 64 + 1, 0);
    // for (auto it = st->root->visit.begin(); it != st->root->visit.end(); ++it)
    //     prev_vector[(*it) / 64] |= ((ull)1 << ((*it) % 64)); //這個prev_vector代表root，所有文件都會走過。
    // st->bit_vec2global(st->root, document_size, prev_vector);
    // ========================= bit_array 用法 ========================= //
    // ========================= vector 用法 ========================= //
    vector<int> prev_vector(document_size, 0);
    for (int k = 0; k < prev_vector.size(); k++)
        prev_vector[k] = k;
    st->bit_vec2global(st->root, document_size, prev_vector);
    // ========================= vector 用法 ========================= //
    b = clock();

    cout << "In the Bit_vec2global Phase need to cost= " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;

    log_file << "In the Bit_vec2global Phase need to cost= " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;
    // --------------------------------------------- Hash Vec/Bitarray area ----------------------------------------------//

    // -------------------------------- Find N-gram area(Testing Data) --------------------------------//
    for (restrict_query = 2; restrict_query <= restrict; restrict_query *= 2)
    {
        cout << "Restrict_query = " << restrict_query << endl;
        log_file << "Restrict_query = " << restrict_query << endl;
        a = clock();
        for (int i = allPath.size() - testing_data_len; i < allPath.size(); i++)
        {
            int lcs_len = 0;
            vector<int> lcs_str;
            unordered_map<int, map<int, int>> gram;                // ID , N-gram , 出現幾次
            unordered_map<int, unordered_map<int, int>> memo;      // 存VecID,N-gram,次數
            unordered_map<int, unordered_map<int, int>> diff_memo; // 存VecID,N-gram,次數

            st->new_a_part_of_substring(i + 1, path + allPath[i], lcs_len, lcs_str, gram, memo, diff_memo);
            ofstream n_gram_file("/home/weiren/ML_experiment/xxxx/" + allPath[i] + "_n-gram.txt");

            for (auto it = memo.begin(); it != memo.end(); ++it)
            { // 取ID
                for (auto itt = it->second.begin(); itt != it->second.end(); ++itt)
                { // 取N-gram的N
                    for (auto ittt = id2vec[it->first].begin(); ittt != id2vec[it->first].end(); ++ittt)
                    {
                        gram[*ittt][itt->first] += itt->second;
                    }
                }
            }
            // for (auto it = memo.begin(); it != memo.end(); ++it)
            // { // 取ID
            //     for (auto itt = it->second.begin(); itt != it->second.end(); ++itt)
            //     { // 取N-gram的N
            //         for (int k = 0; k < id2bit[it->first].size(); k++)
            //         { // 取bit_ID裡的文件
            //             ull doc_val = id2bit[it->first][k];
            //             while (doc_val)
            //             {
            //                 int idx = __builtin_ctzll(doc_val);
            //                 doc_val &= doc_val - 1;
            //                 gram[(k << 6) + idx][itt->first] += itt->second;
            //             }
            //         }
            //     }
            // }
            for (auto it = diff_memo.begin(); it != diff_memo.end(); ++it)
            { // 取ID
                for (auto itt = it->second.begin(); itt != it->second.end(); ++itt)
                { // 取N-gram的N
                    for (auto ittt = id2diffvec[it->first].begin(); ittt != id2diffvec[it->first].end(); ++ittt)
                    {
                        gram[*ittt][itt->first] += itt->second;
                    }
                }
            }

            // for (auto it = diff_memo.begin(); it != diff_memo.end(); ++it)
            // { // 取ID
            //     for (auto itt = it->second.begin(); itt != it->second.end(); ++itt)
            //     { // 取N-gram的N
            //         for (int k = 0; k < id2diffbit[it->first].size(); k++)
            //         { // 取bit_ID裡的文件
            //             ull doc_val = id2diffbit[it->first][k];
            //             while (doc_val)
            //             {
            //                 int idx = __builtin_ctzll(doc_val);
            //                 doc_val &= doc_val - 1;
            //                 gram[(k << 6) + idx][itt->first] += itt->second;
            //             }
            //         }
            //     }
            // }
            for (auto it = gram.begin(); it != gram.end(); ++it)
            {
                n_gram_file << allPath[it->first] << " ";
                for (auto itt = it->second.begin(); itt != it->second.end(); ++itt)
                {
                    n_gram_file << itt->first << " " << itt->second << " ";
                }
                n_gram_file << endl;
            }
            n_gram_file.close();
        }
        b = clock();
        cout << "Find N-gram cost= " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;
        log_file << "Find N-gram cost= " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;

        // -------------------------------- Find N-gram area(Testing Data) --------------------------------//

        system("free -h >> /home/weiren/cout_log.txt");
        log_file << endl;
        // fgetc(stdin);
    }
    return 0;
}
