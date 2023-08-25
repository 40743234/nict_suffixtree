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
#include "suffix_tree_function.h"

using namespace std;

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
    return allpath;
}

int main()
{
    clock_t a, b, c, d;
    ios::sync_with_stdio(false);
    cin.tie(0);
    ifstream myFile;
    string seq;
    char *path = "/home/weiren/mal_opcode/";
    vector<int> stop_pos;
    vector<string> allPath = getFilesList();
    SuffixTree *st = new SuffixTree();
    int seq_start = 0;
    for (int i = 0; i < allPath.size() - 1384; i++)
    {
        // cout << allPath[i] << endl;
        double START, END;
        if (i % 100 == 0)
            cout << "already handle " << i << " sample" << endl;
        myFile.open(path + allPath[i]);
        stringstream ss;
        ss << myFile.rdbuf();
        myFile.close();
        create_table(st, ss);
        st->st_build(i, seq_start);
        seq_start = st->number_sequence.size();
        st->st_continue_initial();
        stop_pos.push_back(seq_start);
        // cout << endl;
    }
    cout << "In the Create Suffix_Tree Phase need to cost= " << (double)clock() / CLOCKS_PER_SEC << "s" << endl;

    // --------------------------------------------- Check & Debug area ---------------------------------------------//
    // cout << "Node_Parent\tNode_Address\tStart\tEnd\tPreSum" << endl;
    // st->check(st->root);
    // for (auto it = table_num.begin(); it != table_num.end(); ++it)
    //     cout << it->first << " " << it->second << endl;
    // --------------------------------------------- Check & Debug area ---------------------------------------------//
    // --------------------------------------------- Calculate Node area ---------------------------------------------//
    // int count_link = 0;
    // int count_node = 0;
    // st->dfs(st->root,count_link,count_node);
    // cout<<count_node<<" "<<count_link<<endl;
    // --------------------------------------------- Calculate Node area ---------------------------------------------//

    // --------------------------------------------- Find ALL Document's LCS area ---------------------------------------------//
    // unordered_map<int, int> temp;
    // unordered_map<int, unordered_map<int, int>> matrix; // 第一個Key存的是ID，第二個KEY是幾號文件，第三個VALUE是目前的最長string為多少。
    // // 代表說 我會給後面的給一個matrix開一個ID，如果node之後有遇到一樣的matrix，我就存這個matrix的ID就好。
    // vector<pair<int, int>> matrix2; // 編號 docID LCS

    // a = clock();
    // st->dfs_visit_str(st->root, temp, matrix, matrix2);
    // b = clock();
    // cout << "This DFS Phase cost " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;
    // // for (int i = 0; i < matrix2.size(); i++)
    // // {
    // //     cout << i << "= " << matrix2[i].first << " " << matrix2[i].second << endl;
    // // }
    // // st->check(st->root);
    // cout << "----------------------" << endl;
    // for (int i = allPath.size() - 1; i < allPath.size(); i++)
    // {
    //     unordered_map<int, vector<int>> lcs_str; // docID,substring
    //     double for_time = 0;
    //     cout << "Find the " << allPath[i] << " file's LCS." << endl;
    //     a = clock();
    //     st->new_find_LCS_str(path + allPath[i], lcs_str, for_time, matrix, matrix2);
    //     b = clock();
    //     int asdadad = 0;
    //     for (auto it = lcs_str.begin(); it != lcs_str.end(); ++it)
    //     {
    //         if (asdadad % 1 == 0)
    //         {
    //             cout << "In the " << allPath[it->first] << " have length of " << it->second.size() << " LCS" << endl;
    //             cout << "LCS String = ";
    //             // for (int j = 0; j < it->second.size(); j++)
    //             // {
    //             //     cout << it->second[j] << " ";
    //             // }
    //             // cout << endl;
    //             for (int j = 0; j < it->second.size(); j++)
    //                 cout << table_name[it->second[j]] << " ";
    //             cout << endl;
    //         }
    //         asdadad++;
    //     }
    //     double travel_time = double(double(b) - double(a));
    //     cout << "Find LCS all cost " << travel_time / CLOCKS_PER_SEC << "s" << endl;
    //     // cout << "Cal Doc cost " << for_time / CLOCKS_PER_SEC << "s" << endl;
    //     // cout << "Traversal Tree cost " << (travel_time - for_time) / CLOCKS_PER_SEC << "s" << endl;
    //     cout << "------------" << endl;

    //     // cout << "Find LCS cost " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;
    // }
    // --------------------------------------------- Find ALL Document's LCS area ---------------------------------------------//

    // --------------------------------------------- Find a part of Substring area ---------------------------------------------//
    a = clock();
    double cal_gram_time = 0;
    double cal_tree_time = 0;
    for (int i = allPath.size() - 1384; i < allPath.size(); i++)
    {
        // c = clock();
        int lcs_len = 0;
        vector<int> lcs_str;
        unordered_map<int, map<int, int>> gram;
        st->new_a_part_of_substring(path + allPath[i], lcs_len, lcs_str, gram, cal_gram_time, cal_tree_time);
        ofstream n_gram_file("/home/weiren/opcode_test20_gram/" + allPath[i] + "_n-gram.txt");
        for (auto it = gram.begin(); it != gram.end(); ++it)
        {
            // cout << "In the " << allPath[it->first] << "= " << endl;
            n_gram_file << allPath[it->first] << " ";
            for (auto itt = it->second.begin(); itt != it->second.end(); ++itt)
            {
                n_gram_file << itt->first << " " << itt->second << " ";
                // cout << itt->first << "-gram appear " << itt->second << " times" << endl;
            }
            n_gram_file << endl;
        }
        n_gram_file.close();
        // d = clock();
        // cout << "New Find N-gram cost " << double(double(d) - double(c)) / CLOCKS_PER_SEC << "s" << endl;
    }
    b = clock();
    cout << "New Find N-gram cost " << double(double(b) - double(a)) / CLOCKS_PER_SEC << "s" << endl;
    cout << "Calulate N-gram Document cost" << cal_gram_time / CLOCKS_PER_SEC << "s" << endl;
    cout << "Traversal Tree  cost" << cal_tree_time / CLOCKS_PER_SEC << "s" << endl;
    // --------------------------------------------- Find a part of Substring area ---------------------------------------------//

    cout << (double)clock() / CLOCKS_PER_SEC << "s" << endl;
    fgetc(stdin);
    // delete st;
    // cout<<"done"<<endl;
    // fgetc(stdin);
    return 0;
}