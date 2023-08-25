#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <climits>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <unistd.h>

using namespace std;

struct hashFunction
{
    size_t operator()(const vector<int>
                          &myVector) const
    {
        std::hash<int> hasher;
        size_t answer = 0;

        for (int i : myVector)
        {
            answer ^= hasher(i) + 0x9e3779b9 +
                      (answer << 6) + (answer >> 2);
        }
        return answer;
    }
};

unordered_map<string, unsigned short> table_num;
unordered_map<unsigned short, string> table_name;
int restrict = 20; // 限制樹的深度
int restrict_doc = 20;
class SuffixNode
{
public:
    int start, end, prev_ct, node_idx;
    unordered_set<int> visit; // 這個變數主要是去紀錄在node被那些文件訪問過
    unordered_map<int, int> visit_len;
    unordered_map<int, vector<int>> visit_str;
    vector<int> matrix_ID;
    SuffixNode *SuffixLink;
    SuffixNode *SuffixLink_rev;
    SuffixNode *parent;
    unordered_map<unsigned short, SuffixNode *> child;
    SuffixNode(int s, int e = INT_MAX) // 設為INT_MAX是到時候要online 可以動態調整。
    {
        start = s;
        end = e;
        node_idx = 0;
        SuffixLink = NULL;
        SuffixLink_rev = NULL;
        child.clear();
    }
    ~SuffixNode()
    {
        visit.clear();
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
    SuffixTree()
    {
        st_initial();
    }

    void st_initial()
    {
        remaining = active_edge = active_length = 0;
        idx = -1;
        root = new SuffixNode(-1, -1);
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
            return true;
        }
        else
            return false;
    }
    void st_build(int doc_idx, int seq_start)
    {
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
                        active_node->child[get_nowchar()] = new SuffixNode(str_idx, (int)number_sequence.size() - 1);
                        SuffixNode *node = active_node->child[get_nowchar()];
                        node->parent = active_node;
                        node->prev_ct = active_node->prev_ct + (active_node->end - active_node->start + 1);
                        // cout << "before= " << node << " " << node->start << " " << node->end << endl;
                        if (node->prev_ct + (node->end - node->start + 1) > restrict)
                        {
                            node->end = node->start + (restrict - node->prev_ct - 1);
                        }
                        // cout << "after= " << node << " " << node->start << " " << node->end << endl;
                        //  如果有新建節點就去add suffix link，但是除了root之外，其他節點如果新增的child，就會變成內部節點
                        //  所以在add_slink裡面，會去判斷說上一個node是不是root，如果是就直接把這個新建節點的父節點(也就是active_node)
                        //  讓lastaddnode改成父節點就好。如果到時候遇到了內部節點，就可以讓這個節點指向下次新建的node。
                        add_internal_slink(active_node);

                        node->visit.insert(doc_idx);
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
                        // cout << NextNode->end << " " << NextNode->start << endl;
                        active_length++;
                        // active_node = NextNode; 這邊不用加是因為妳walkdown就已經走到這個node了，所以就不用再寫一次。
                        add_internal_slink(active_node);

                        if (NextNode->child.size() == 0 && active_length >= NextNode->get_len())
                        {

                            NextNode->visit.insert(doc_idx);
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
                            SuffixNode *split = new SuffixNode(NextNode->start, NextNode->start + active_length - 1);

                            active_node->child[get_nowchar()] = split;
                            SuffixNode *leaf = new SuffixNode(str_idx, (int)number_sequence.size() - 1);
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

                            // 這兩個for迴圈，主要是做把node的資訊都丟給split。

                            split->visit = NextNode->visit;
                            split->visit.insert(doc_idx);
                            leaf->visit.insert(doc_idx);
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
                start += temp->get_len();
            }
            remaining--;
        }

        // number_sequence.pop_back();
    }

    void dfs(SuffixNode *node, int &count_link, int &count_node)
    {
        count_node++;
        // cout << node->start << " " << node->end << "= " << endl;
        // for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
        // {
        //     cout << *it << " ";
        // }
        // cout << endl;
        if (node->SuffixLink != root && node->SuffixLink != NULL)
            count_link++;
        for (auto it = node->child.begin(); it != node->child.end(); ++it)
            dfs(it->second, count_link, count_node);
        return;
    }

    void dfs_visit_str(SuffixNode *node, unordered_map<int, int> &m_str, unordered_map<int, unordered_map<int, int>> &matrix, vector<pair<int, int>> &matrix2)
    {
        for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
            m_str[*it] += node->get_len();

        if (node->SuffixLink_rev != NULL && node->SuffixLink_rev != root)
        {
            for (auto it = m_str.begin(); it != m_str.end(); ++it)
            {
                if (matrix[it->first].find(it->second) == matrix[it->first].end())
                { // 代表沒出現過
                    node->matrix_ID.push_back(matrix2.size());
                    matrix[it->first][it->second] = matrix2.size();
                    // matrix2[matrix2.size()] = {it->first, it->second};
                    matrix2.push_back({it->first, it->second});
                }
                else
                    node->matrix_ID.push_back(matrix[it->first][it->second]);
            }
        }
        for (auto it = node->child.begin(); it != node->child.end(); ++it)
            dfs_visit_str(it->second, m_str, matrix, matrix2);

        for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
        {
            m_str[*it] -= node->get_len();
            if (m_str[*it] == 0)
                m_str.erase(*it);
        }

        return;
    }

    void check(SuffixNode *node)
    {
        cout << node->parent << "\t\t" << node << "\t" << node->start << "\t" << node->end << "\t" << node->prev_ct;
        // cout << node->parent << "\t" << node << "\t" << node->start << "\t" << node->end << "\t" << node->prev_ct;
        if (node->SuffixLink != NULL)
            cout << "    Suffix link: "
                 << " " << node->SuffixLink << " " << node->SuffixLink->start << " " << node->SuffixLink->end;
        cout << endl;
        cout << "This Node have document: (doc_idx)" << endl;
        for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
        {
            cout << *it << endl;
        }
        if (node->matrix_ID.size() > 0)
        {
            cout << "This Node have these number" << endl;
            for (int i = 0; i < node->matrix_ID.size(); i++)
            {
                cout << node->matrix_ID[i] << endl;
            }
        }
        for (auto it = node->child.begin(); it != node->child.end(); ++it)
            check(it->second);
        return;
    }

    clock_t a, b, c, d;
    void new_find_LCS_str(const string &str, unordered_map<int, vector<int>> &lcs_str, double &for_time, unordered_map<int, unordered_map<int, int>> &matrix, vector<pair<int, int>> &matrix2)
    {
        ifstream myFile;
        vector<int> find_str;
        stringstream ss;
        // 這裡在處理要找的字串
        if (access(str.c_str(), 0) == F_OK)
        { // 判斷這個路徑是否存在 (為了要區分傳進來的是字串還是檔案的路徑)
            cout << str << endl;
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
        cout << "This Doc's size =  " << find_str.size() << endl;
        int node_len = 0; // 紀錄現在走到node的哪一個index
        vector<int> vec;
        SuffixNode *node = this->root;
        for (int i = 0; i < find_str.size(); i++)
        {
            int cur = find_str[i];
            if ((node == this->root && node->child.find(cur) != node->child.end()) || (node_len > node->end - node->start && node->child.find(cur) != node->child.end()))
            {
                node = node->child[cur];
                node_len = 0;
            }
            if (node != this->root && node->start + node_len <= node->end && (node->start + node_len) < this->number_sequence.size() && cur == this->number_sequence[node->start + node_len])
            {
                // vec.push_back(node->start + node_len);
                vec.push_back(this->number_sequence[node->start + node_len]);
                update_LCS_str(node, vec, lcs_str, 0, for_time, matrix, matrix2);
                node_len++;
            }
            // 把vec裡面的suffix都判斷玩了，發現完全沒有路可以走了，就繼續往下走
            else if (node == this->root && node->child.find(cur) == node->child.end())
            {
                continue;
            }
            else
            {
                vec.erase(vec.begin());
                init_path_sufflink(vec, node, node_len, lcs_str, for_time, matrix, matrix2);

                i--;
            }
        }
        if (vec.size() > 0)
        {
            update_LCS_str(node, vec, lcs_str, 0, for_time, matrix, matrix2);
        }
        return;
    }
    void update_LCS_str(SuffixNode *node, vector<int> &vec, unordered_map<int, vector<int>> &lcs_str, bool mode, double &for_time, unordered_map<int, unordered_map<int, int>> &matrix, vector<pair<int, int>> &matrix2)
    {
        double a = clock();
        // 代表一般情況 就單純搜尋visit的內容就好
        if (mode == 0)
        {
            for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
            {
                if (vec.size() > lcs_str[*it].size())
                    lcs_str[*it] = vec;
            }
        }
        // 代表使用suffixlink情況 要搜尋visit_str的內容
        else if (mode == 1)
        {
            for (int i = 0; i < node->matrix_ID.size(); i++)
            {
                if (matrix2[node->matrix_ID[i]].second > lcs_str[matrix2[node->matrix_ID[i]].first].size())
                    lcs_str[matrix2[node->matrix_ID[i]].first] = {vec.begin(), vec.begin() + matrix2[node->matrix_ID[i]].second};
            }

            // for (auto it = node->visit_str.begin(); it != node->visit_str.end(); ++it)
            // {
            //     if (it->second.size() > lcs_str[it->first].size())
            //         lcs_str[it->first] = it->second;
            // }
        }
        b = clock();
        for_time += double(double(b) - double(a));
        return;
    }
    void init_path_sufflink(vector<int> &vec, SuffixNode *&node, int &node_len, unordered_map<int, vector<int>> &lcs_str, double &for_time, unordered_map<int, unordered_map<int, int>> &matrix, vector<pair<int, int>> &matrix2)
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
                update_LCS_str(node, vec, lcs_str, 1, for_time, matrix, matrix2);
                // if (node == NULL)
                // {
                //     node = this->root;
                //     node_len = 0;
                //     return;
                // }
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
                        // if (node == NULL)
                        // {
                        //     node = this->root;
                        //     node_len = 0;
                        //     return;
                        // }
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
            update_LCS_str(node, vec, lcs_str, 1, for_time, matrix, matrix2);
        }

        if (node->child[vec[start]] == NULL)
        {
            node = this->root;
            node_len = 0;
            return;
        }
        else
            node = node->child[vec[start]];

        while (start < n)
        {
            if (start + (node->end - node->start + 1) < n)
            {
                start += (node->end - node->start + 1);

                if (node->child[vec[start]] == NULL)
                {
                    node = this->root;
                    node_len = 0;
                    return;
                }
                else
                {
                    node = node->child[vec[start]];
                    node_len = 0;
                }
                // if (node == NULL)
                // {
                //     node = this->root;
                //     node_len = 0;
                //     return;
                // }
            }
            else
            {
                node_len = n - start;
                break;
            }
        }
        return;
    }

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
            node = node->SuffixLink;

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

    void new_a_part_of_substring(const string &str, int &max_len, vector<int> &max_vec, unordered_map<int, map<int, int>> &gram, double &cal_gram_time, double &cal_tree_time)
    {
        ifstream myFile;
        vector<int> find_str;
        unordered_set<vector<int>, hashFunction> m;
        stringstream ss;
        // 這裡在處理要找的字串
        if (access(str.c_str(), 0) == F_OK)
        { // 判斷這個路徑是否存在 (為了要區分傳進來的是字串還是檔案的路徑)
            // cout << str << endl;
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
            c = clock();
            if ((node == this->root && node->child.find(cur) != node->child.end()) || (node_len > node->end - node->start && node->child.find(cur) != node->child.end()))
            {
                if (node->child[cur] == NULL)
                {
                    node = this->root;
                    d = clock();
                    cal_tree_time += (d - c);
                    if (vec.size() > 0)
                        vec.erase(vec.begin());
                    node_len = 0;
                    i--;
                    continue;
                }
                else
                {
                    c = clock();
                    node = node->child[cur];
                    d = clock();
                    cal_tree_time += (d - c);
                }
                node_len = 0;
            }
            c = clock();
            if (node != this->root && node->start + node_len <= node->end && (node->start + node_len) < this->number_sequence.size() && cur == this->number_sequence[node->start + node_len])
            {
                d = clock();
                cal_tree_time += (d - c);
                // vec.push_back(node->start + node_len);
                vec.push_back(this->number_sequence[node->start + node_len]);
                node_len++;
                if (max_len < vec.size())
                {
                    max_len = vec.size();
                    max_vec = vec;
                    // if (max_len == restrict)
                    //     break;
                }
            }

            // 把vec裡面的suffix都判斷玩了，發現完全沒有路可以走了，就繼續往下走
            else if (node == this->root && node->child.find(cur) == node->child.end())
            {
                d = clock();
                cal_tree_time += (d - c);
                continue;
            }
            else if (vec.size() > 0)
            {

                // for (int idx = 0; idx < vec.size(); idx++)
                //     cout << table_name[vec[idx]];
                // cout << endl;

                if (m.count(vec) == 0)
                {
                    a = clock();
                    for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
                    {
                        gram[*it][vec.size()]++;
                    }
                    b = clock();
                    cal_gram_time += (b - a);
                }

                m.insert(vec);
                vec.erase(vec.begin());
                c = clock();
                init_path(vec, node, node_len);
                d = clock();
                cal_tree_time += (d - c);
                i--;
            }
        }
        if (vec.size() > 0)
        {
            // for (int idx = 0; idx < vec.size(); idx++)
            //     cout << table_name[vec[idx]];
            // cout << endl;
            if (m.count(vec) == 0)
            {
                a = clock();
                for (auto it = node->visit.begin(); it != node->visit.end(); ++it)
                {
                    gram[*it][vec.size()]++;
                }
                b = clock();
                cal_gram_time += (b - a);
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