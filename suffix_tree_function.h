#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <climits>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <chrono>
#define ull unsigned long long
using namespace std;

struct HashVec
{
    size_t operator()(const vector<int> &myVector) const
    {
        std::hash<int> hasher;
        size_t seed = 0;
        for (int i : myVector)
        {
            seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
struct hashBitarr
{
    size_t operator()(const vector<ull> &myVector) const
    {
        std::hash<ull> hasher;
        size_t answer = 0;
        for (int i = 0; i < myVector.size(); i++)
        {
            ull diff_value = myVector[i];
            while (diff_value)
            {
                answer ^= hasher((i << 6) + __builtin_ctzll(diff_value)) + 0x9e3779b9 +
                          (answer << 6) + (answer >> 2);
                diff_value &= diff_value - 1;
            }
        }
        return answer;
    }
};
struct hashull
{
    size_t operator()(const vector<ull> &myVector) const
    {
        std::hash<ull> hasher;
        size_t answer = 0;
        for (const ull &i : myVector)
        {
            answer ^= hasher(i) + 0x9e3779b9 +
                      (answer << 6) + (answer >> 2);
        }
        return answer;
    }
};
struct hashset
{
    size_t operator()(const vector<int> &myVector) const
    {
        std::hash<int> hasher;
        size_t answer = 0;
        for (int i : myVector)
        {
            answer ^= hasher(i) + 0x9e3779b9 + (answer << 6) + (answer >> 2);
        }
        return answer;
    }
};

unordered_map<string, unsigned short> table_num;  // 把opcode mapping成int
unordered_map<unsigned short, string> table_name; // 把int mapping回opcode

unordered_map<vector<int>, int, HashVec> vec2id;     // 把文件編號(vec) mapping成int
unordered_map<int, vector<int>> id2vec;              // 把int mapping回文件編號(vec)
unordered_map<vector<int>, int, HashVec> diffvec2id; // 把差集的文件編號(vec) mapping成int
unordered_map<int, vector<int>> id2diffvec;          // 把int mapping回差集的文件編號(vec)
// unordered_map<vector<ull>, int, hashull> bit2id; //把文件編號(bitarray) mapping成int
// unordered_map<int, vector<ull>> id2bit; 把int mapping回文件編號(bitarray)
// unordered_map<vector<ull>, int, hashBitarr> diffbit2id; //把差集的文件編號(bitarray) mapping成int
// unordered_map<int, vector<ull>> id2diffbit; 把int mapping回差集的文件編號(bitarray)
int restrict = 256;     // 限制建樹深度
int restrict_query = 2; // 限制搜尋深度
class SuffixNode
{
public:
    int start, end, prev_ct, node_idx;
    int doc_id;                              // 透過id2docid的全域變數來找到我這個node有哪些文件出現過。
    unordered_set<int> visit;                // 主要是去紀錄在node被那些文件訪問過
    vector<int> visit_vec;                   // 跟visit是一樣的 是去紀錄在node被那些文件訪問過
    unordered_map<int, int> check_substring; // n-gram doc 在搜尋時，去記錄說這個node下的docID跟N-GRAM長度已經看過了。
    SuffixNode *SuffixLink;                  // 指向suffixlink的node
    SuffixNode *SuffixLink_rev;              // 指回suffixlink的node
    SuffixNode *parent;
    unordered_map<unsigned short, SuffixNode *> child;
    int diff_id;                                 // 這個變數主要是去紀錄作完差集之後的vector會對應回我的docid2id這樣我只要存一個integer，然後要用時透過table來取就好。
    SuffixNode(int s, int e, int &document_size) // 設為INT_MAX是到時候要online 可以動態調整。
    {
        start = s;
        end = e;
        node_idx = 0;
        SuffixLink = NULL;
        SuffixLink_rev = NULL;
    }
    ~SuffixNode()
    {
        visit.clear();
        visit_vec.clear();
        parent = NULL;
        child.clear();
    }
    int get_len()
    {
        return end - start + 1;
    }
};
class SuffixTree
{
public:
    int idx;
    int remaining;
    int active_edge, active_length;
    vector<int> number_sequence;

    SuffixNode *LastInternalNode; // 紀錄最近新增的節點，因為當有內部節點產生，就會有suffix link。 而suffix linek就是去指向上一個新增的點
    SuffixNode *LastLeafNode;
    SuffixNode *root;
    SuffixNode *active_node;
    SuffixTree(const SuffixTree &tree)
    {
        idx = tree.idx;
        remaining = tree.remaining;
        active_edge = tree.active_edge;
        active_length = tree.active_length;
        number_sequence = tree.number_sequence;
        LastInternalNode = tree.LastInternalNode;
        LastLeafNode = tree.LastLeafNode;
        root = tree.root;
        active_node = tree.active_node;
    }
    SuffixTree(int &document_size)
    {
        st_initial(document_size);
    }

    void st_initial(int &document_size)
    {
        remaining = active_edge = active_length = 0;
        idx = -1;
        root = new SuffixNode(-1, -1, document_size);
        root->SuffixLink = root;
        root->SuffixLink_rev = root;
        root->parent = NULL;
        root->prev_ct = -1;
        active_node = root;
    }
    void st_continue_initial()
    {
        remaining = active_edge = active_length = 0;
        active_node = root;
    }
    void add_internal_slink(SuffixNode *node)
    {
        if (LastInternalNode != root)
        {
            LastInternalNode->SuffixLink = node;
            node->SuffixLink_rev = LastInternalNode;
        }
        LastInternalNode = node;
    }

    int get_nowchar()
    {
        return number_sequence[active_edge];
    }
    // 這裡主要做的事情是當active_len大於了目前這個node的長度，那這樣就要跳到下一個node。
    bool walk_down(int doc_idx, SuffixNode *node)
    {
        if (active_length >= node->get_len() && node->child.size() > 0)
        {
            active_edge += node->get_len();
            active_length -= node->get_len();
            active_node = node;

            node->visit.insert(doc_idx);
            if (node->visit_vec.empty() || node->visit_vec[node->visit_vec.size() - 1] != doc_idx)
                node->visit_vec.push_back(doc_idx);
            return true;
        }
        else
            return false;
    }
    void st_build(int doc_idx, int seq_start, int &document_size)
    {
        root->visit.insert(doc_idx);
        if (root->visit_vec.empty() || root->visit_vec[root->visit_vec.size() - 1] != doc_idx)
            root->visit_vec.push_back(doc_idx);
        number_sequence.push_back(INT_MAX);
        active_node = root;
        LastLeafNode = root;

        for (int str_idx = seq_start; str_idx < number_sequence.size(); str_idx++)
        {

            idx++;
            LastInternalNode = root;
            remaining++;
            // dfs(this->root);
            // cout << endl;
            while (remaining > 0)
            {

                //  如果active_length==0，就代表沒有隱藏字元，所以就直接從這個position開始找就可以了。
                if (active_length == 0)
                {
                    active_edge = str_idx;
                }
                // 代表現在這個position的字元沒辦法在當下的node的children被找到，所以就要創新的點
                if (active_node->child.find(get_nowchar()) == active_node->child.end())
                {
                    if (active_node->prev_ct + (active_node->end - active_node->start + 1) < restrict)
                    {
                        active_node->child[get_nowchar()] = new SuffixNode(str_idx, (int)number_sequence.size() - 1, document_size);
                        SuffixNode *node = active_node->child[get_nowchar()];
                        node->parent = active_node;
                        node->prev_ct = active_node->prev_ct + (active_node->end - active_node->start + 1);
                        if (node->prev_ct + (node->end - node->start + 1) > restrict)
                        {
                            node->end = node->start + (restrict - node->prev_ct - 1);
                        }
                        //  如果有新建節點就去add suffix link，但是除了root之外，其他節點如果新增的child，就會變成內部節點
                        //  所以在add_slink裡面，會去判斷說上一個node是不是root，如果是就直接把這個新建節點的父節點(也就是active_node)
                        //  讓lastaddnode改成父節點就好。如果到時候遇到了內部節點，就可以讓這個節點指向下次新建的node。
                        add_internal_slink(active_node);

                        node->visit.insert(doc_idx);
                        if (node->visit_vec.empty() || node->visit_vec[node->visit_vec.size() - 1] != doc_idx)
                            node->visit_vec.push_back(doc_idx);
                    }
                }
                // 這邊是如果已經存在這個點了
                else
                {
                    SuffixNode *NextNode = active_node->child[get_nowchar()];
                    if (walk_down(doc_idx, NextNode) == true)
                        continue;
                    if (number_sequence[NextNode->start + active_length] == number_sequence[str_idx])
                    {
                        active_length++;
                        // active_node = NextNode; 這邊不用加是因為妳walkdown就已經走到這個node了，所以就不用再寫一次。
                        add_internal_slink(active_node);

                        if (NextNode->child.size() == 0 && active_length >= NextNode->get_len())
                        {

                            NextNode->visit.insert(doc_idx);
                            if (NextNode->visit_vec.empty() || NextNode->visit_vec[NextNode->visit_vec.size() - 1] != doc_idx)
                                NextNode->visit_vec.push_back(doc_idx);
                            remaining--;
                            if (active_node == root && active_length > 0)
                            {
                                active_length--;
                                active_edge += 1;
                            }
                            else
                            {
                                if (active_node->SuffixLink != NULL)
                                {
                                    if (active_node->SuffixLink != this->root)
                                    {
                                        active_node->SuffixLink->visit.insert(doc_idx);
                                        if (active_node->SuffixLink->visit_vec.empty() || active_node->SuffixLink->visit_vec[active_node->SuffixLink->visit_vec.size() - 1] != doc_idx)
                                            active_node->SuffixLink->visit_vec.push_back(doc_idx);
                                    }
                                    active_node = active_node->SuffixLink;
                                }
                                else
                                {
                                    active_node = root;
                                }
                            }
                        }
                        break;
                    }
                    // 如果走到底發現說我要的字元跟現在路徑上的字元不一樣，就知道要分開了。會有新的一條路徑產生。
                    else
                    {
                        if (NextNode->prev_ct + NextNode->end - NextNode->start + 1 <= restrict)
                        {
                            SuffixNode *split = new SuffixNode(NextNode->start, NextNode->start + active_length - 1, document_size);

                            active_node->child[get_nowchar()] = split;
                            SuffixNode *leaf = new SuffixNode(str_idx, (int)number_sequence.size() - 1, document_size);
                            split->child[number_sequence[str_idx]] = leaf;
                            NextNode->start += active_length;
                            split->child[number_sequence[NextNode->start]] = NextNode;

                            split->parent = active_node;
                            leaf->parent = split;
                            NextNode->parent = split;

                            split->prev_ct = active_node->prev_ct + (active_node->end - active_node->start + 1);
                            leaf->prev_ct = split->prev_ct + (split->end - split->start + 1);
                            NextNode->prev_ct = split->prev_ct + (split->end - split->start + 1);

                            if (leaf->prev_ct + (leaf->end - leaf->start + 1) > restrict)
                                leaf->end = leaf->start + (restrict - leaf->prev_ct - 1);
                            if (NextNode->prev_ct + (NextNode->end - NextNode->start + 1) > restrict)
                                NextNode->end = NextNode->start + (restrict - NextNode->prev_ct - 1);

                            add_internal_slink(split);

                            // 將原本node的資訊都丟給split。
                            split->visit = NextNode->visit;
                            split->visit_vec = NextNode->visit_vec;

                            split->visit.insert(doc_idx);
                            leaf->visit.insert(doc_idx);
                            if (split->visit_vec.empty() || split->visit_vec[split->visit_vec.size() - 1] != doc_idx)
                                split->visit_vec.push_back(doc_idx);
                            if (leaf->visit_vec.empty() || leaf->visit_vec[leaf->visit_vec.size() - 1] != doc_idx)
                                leaf->visit_vec.push_back(doc_idx);
                        }
                    }
                }
                remaining--;
                if (active_node == root && active_length > 0)
                {
                    active_length--;
                    active_edge += 1;
                }
                else
                {
                    if (active_node->SuffixLink != NULL)
                    {
                        if (active_node->SuffixLink != this->root)
                        {
                            active_node->SuffixLink->visit.insert(doc_idx);
                            if (active_node->SuffixLink->visit_vec.empty() || active_node->SuffixLink->visit_vec[active_node->SuffixLink->visit_vec.size() - 1] != doc_idx)
                                active_node->SuffixLink->visit_vec.push_back(doc_idx);
                        }
                        active_node = active_node->SuffixLink;
                    }
                    else
                        active_node = root;
                }
            }
        }

        while (remaining > 0)
        {
            int start = number_sequence.size() - remaining;
            SuffixNode *temp = root;
            while (start < number_sequence.size())
            {
                temp = temp->child[number_sequence[start]];
                temp->visit.insert(doc_idx);
                if (temp->visit_vec.empty() || temp->visit_vec[temp->visit_vec.size() - 1] != doc_idx)
                    temp->visit_vec.push_back(doc_idx);
                start += temp->get_len();
            }
            remaining--;
        }
    }

    // for bitarray的用法
    // void add_visit_idx(SuffixNode *node)
    // {
    //     if (node == NULL)
    //         return;
    //     // 遍歷所有子節點
    //     for (auto &childPair : node->child)
    //     {
    //         SuffixNode *childNode = childPair.second;
    //         add_visit_idx(childNode);
    // 子節點的東西在父節點都要出現
    //         for (int docId : childNode->visit)
    //         {
    //             node->visit.insert(docId);
    //         }
    //     }
    // }

    // for vector的用法
    void add_visit_idx(SuffixNode *node)
    {
        if (node == NULL)
            return;

        // 遍歷所有子節點
        for (auto &childPair : node->child)
        {
            SuffixNode *childNode = childPair.second;
            add_visit_idx(childNode);

            // 把node跟node子節點的文件編號都加到merged裡面
            std::vector<int> merged;
            merged.reserve(node->visit_vec.size() + childNode->visit_vec.size());
            auto parentIter = node->visit_vec.begin();
            auto childIter = childNode->visit_vec.begin();
            // 因為我原本的vector就是排序過後的，所以用divide and conquer的概念去合併
            while (parentIter != node->visit_vec.end() && childIter != childNode->visit_vec.end())
            {
                if (*parentIter < *childIter)
                {
                    merged.push_back(*parentIter);
                    ++parentIter;
                }
                else if (*childIter < *parentIter)
                {
                    merged.push_back(*childIter);
                    ++childIter;
                }
                else
                {
                    merged.push_back(*parentIter);
                    ++parentIter;
                    ++childIter;
                }
            }

            while (parentIter != node->visit_vec.end())
            {
                merged.push_back(*parentIter);
                ++parentIter;
            }
            while (childIter != childNode->visit_vec.end())
            {
                merged.push_back(*childIter);
                ++childIter;
            }

            // 將最後的結果複製回visit_vec
            node->visit_vec = std::move(merged);
        }
    }

    // hash vector
    void bit_vec2global(SuffixNode *node, int &document_size, vector<int> prev_vector)
    {
        vector<int> cur_vec = node->visit_vec;
        vector<int> diff_vec;
        int cur_idx;
        // 計算node跟前一個node的文件編號差集。
        set_difference(prev_vector.begin(), prev_vector.end(), cur_vec.begin(), cur_vec.end(), back_inserter(diff_vec));

        unordered_set<int>().swap(node->visit);
        if (vec2id[cur_vec] == 0)
        {
            cur_idx = vec2id.size();
            vec2id[cur_vec] = cur_idx;
            id2vec[cur_idx] = cur_vec;
            node->doc_id = cur_idx;
        }
        else
            node->doc_id = vec2id[cur_vec];
        if (node != root)
        {
            if (diffvec2id[diff_vec] == 0)
            {
                cur_idx = diffvec2id.size();
                diffvec2id[diff_vec] = cur_idx;
                id2diffvec[cur_idx] = diff_vec;
                node->diff_id = cur_idx;
            }
            else
                node->diff_id = diffvec2id[diff_vec];
        }
        for (auto it = node->child.begin(); it != node->child.end(); ++it)
            bit_vec2global(it->second, document_size, cur_vec);
        return;
    }

    // hash bitarray
    // void bit_vec2global(SuffixNode *node, int &document_size, vector<ull> prev_vector)
    // {
    //     vector<ull> cur_vec(document_size / 64 + 1, 0);
    //     vector<ull> diff_vec = prev_vector;
    //     int cur_idx;

    //     // 把該node的visit(走過的doc_id)都拿出來轉成bit_array的型態。
    //     for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
    //     {
    //         ull mask = ((ull)1 << ((*it) % 64));
    //         cur_vec[(*it) >> 6] |= mask;
    //         diff_vec[(*it) >> 6] ^= mask;
    //     }
    //     unordered_set<int>().swap(node->visit);
    //     if (bit2id[cur_vec] == 0)
    //     {
    //         cur_idx = bit2id.size();
    //         bit2id[cur_vec] = cur_idx;
    //         id2bit[cur_idx] = cur_vec;
    //         node->doc_id = cur_idx;
    //     }
    //     else
    //         node->doc_id = bit2id[cur_vec];
    //     if (node != root)
    //     {
    //         if (diffbit2id[diff_vec] == 0)
    //         {
    //             cur_idx = diffbit2id.size();
    //             diffbit2id[diff_vec] = cur_idx;
    //             id2diffbit[cur_idx] = diff_vec;
    //             node->diff_id = cur_idx;
    //         }
    //         else
    //             node->diff_id = diffbit2id[diff_vec];
    //     }
    //     for (auto it = node->child.begin(); it != node->child.end(); ++it)
    //         bit_vec2global(it->second, document_size, cur_vec);
    //     return;
    // }

    // 這邊主要要去重新定位substring的node位置，會回傳現在要找的字元當下的node。
    void init_path(vector<int> &vec, SuffixNode *&node, int &node_len)
    {
        int n = vec.size();

        if (n == 0)
        {
            node_len = 0;
            node = this->root;
            return;
        }
        else if (n < node_len)
        {
            node_len--;
        }
        int start = n - node_len;
        if (node_len != (node->end - node->start + 1) || node->child.size() == 0)
        {
            node = node->parent;
        }
        else
        {
            if (node->SuffixLink != NULL)
            {
                node = node->SuffixLink;
                node_len = node->end - node->start + 1; // 要加這行! 因為跑到這個CASE的時候 node的長度不一定是一樣的，所以要重新設定node_len。
            }
            else
            {
                node = this->root;
                node_len = vec.size();
                int i = 0;
                while (i < vec.size())
                {
                    if (i + node->child[vec[i]]->get_len() <= vec.size())
                    {
                        i += node->child[vec[i]]->get_len();
                        node_len -= node->child[vec[i]]->get_len();
                        node = node->child[vec[i]];
                    }
                    else
                        break;
                }
            }
            return;
        }

        if (node != this->root)
        {
            node = node->SuffixLink;
        }
        node = node->child[vec[start]];

        while (start < n)
        {
            if (start + (node->end - node->start + 1) < n)
            {
                start += (node->end - node->start + 1);
                node = node->child[vec[start]];
                node_len = 0;
            }
            else
            {
                node_len = n - start;

                break;
            }
        }
        return;
    }

    void new_a_part_of_substring(int documentIdx, const string &str, int &max_len, vector<int> &max_vec, unordered_map<int, map<int, int>> &gram, unordered_map<int, unordered_map<int, int>> &memo, unordered_map<int, unordered_map<int, int>> &diff_memo)
    {
        ifstream myFile;
        vector<int> find_str;
        unordered_set<vector<int>, hashset> m;
        stringstream ss;
        int vec_start = 0, vec_end = 0;
        // 將opcode sequence 轉成integer sequence
        if (access(str.c_str(), 0) == F_OK) // 判斷這個路徑是否存在 (為了要區分傳進來的是字串還是檔案的路徑)
        {
            myFile.open(str);
            ss << myFile.rdbuf();
            myFile.close();
            convert2num(ss, find_str);
        }
        else
        {
            cout << "this type is a string!!  " << str << endl;
            for (int i = 0; i < str.size(); i++)
                find_str.push_back(table_num[str.substr(i, 1)]);
            for (int i = 0; i < find_str.size(); i++)
                cout << find_str[i] << " ";
        }

        int node_len = 0; // 紀錄現在走到node的哪一個index
        vector<int> vec;
        SuffixNode *node = this->root;
        for (int i = 0; i < find_str.size(); i++)
        {
            int cur = find_str[i];

            // 下面這個區塊是不要讓node走到超過使用者限制的深度
            if (vec.size() == restrict_query)
            {
                if (node->check_substring[vec.size()] != documentIdx)
                {
                    node->check_substring[vec.size()] = documentIdx;
                    memo[node->doc_id][vec.size()]++;
                }
                vec.erase(vec.begin());
                vec_start++;
                init_path(vec, node, node_len);
            }

            if ((node == this->root && node->child.find(cur) != node->child.end()) || (node_len > node->end - node->start && node->child.find(cur) != node->child.end()))
            {
                if (node->child[cur] == NULL)
                {
                    node = this->root;
                    if (vec.size() > 0)
                    {
                        vec.erase(vec.begin());
                        vec_start++;
                    }
                    node_len = 0;
                    i--;
                    continue;
                }
                else
                {
                    if (node != root)
                    {
                        // 判斷之前這個substring有沒有加入過
                        if (node->check_substring[node->prev_ct + node->get_len()] != documentIdx)
                        {
                            node->check_substring[node->prev_ct + node->get_len()] = documentIdx;
                            diff_memo[node->child[cur]->diff_id][node->prev_ct + node->get_len()]++;
                        }
                    }
                    node = node->child[cur];
                }
                node_len = 0;
            }
            if (node != this->root && node->start + node_len <= node->end && (node->start + node_len) < this->number_sequence.size() && cur == this->number_sequence[node->start + node_len])
            {
                vec.push_back(this->number_sequence[node->start + node_len]);
                vec_end = node->start + node_len;
                vec_start = vec_end - vec.size() + 1;
                node_len++;
            }
            // 把vec裡面的suffix都判斷完了，發現完全沒有路可以走了，就繼續往下走
            else if (node == this->root && node->child.find(cur) == node->child.end())
            {
                continue;
            }
            else if (vec.size() > 0)
            {
                if (node->check_substring[vec.size()] != documentIdx)
                {
                    node->check_substring[vec.size()] = documentIdx;
                    memo[node->doc_id][vec.size()]++;
                }
                vec.erase(vec.begin());
                vec_start++;
                init_path(vec, node, node_len);
                i--;
            }
        }
        if (vec.size() > 0)
        {
            if (node->check_substring[vec.size()] != documentIdx)
            {
                node->check_substring[vec.size()] = documentIdx;
                memo[node->doc_id][vec.size()]++;
            }
        }
        return;
    }

    void convert2num(stringstream &ss, vector<int> &find_str)
    {
        while (!ss.fail())
        {
            string temp;
            ss >> temp;
            if (temp.empty())
                continue;
            find_str.push_back(table_num[temp]);
        }
        return;
    }
};