#include "parser.hpp"

using namespace std;
using namespace harmful;

namespace harmful {
    size_t trim_indent(string& s);
    node_t* node_from_line(const string& s);
    bool is_white(char c);
    void delete_tree(node_t* root);
}

bool harmful::is_white(char c)
{
    return c == ' ' || c == '\t';
}

size_t harmful::trim_indent(string& s)
{
    size_t i;
    for (i = 0; i < s.length() && s[i] == ' '; i++);

    s.erase(s.begin(), s.begin() + i);
    return i;
}

void harmful::delete_tree(node_t* root)
{
    for (node_t* child : root->children) {
        delete_tree(child);
    }

    delete root;
}

node_t* harmful::node_from_line(const string& s)
{
    size_t first_space = s.find(' ');
    if (first_space == 0) {
        return nullptr;
    }

    string key = s.substr(0, first_space);
    string param;
    vector<string> params;
    if (first_space != string::npos) {
        for (size_t i = first_space + 1; i < s.size(); i++) {
            if (is_white(s[i]) && !is_white(s[i - 1])) {
                params.push_back(param);
                param.clear();
            } else if (!is_white(s[i])) {
                param.push_back(s[i]);
            }
        }

        if (!param.empty()) {
            params.push_back(param);
        }
    }

    node_t *res = new node_t();
    res->key = key;
    res->params = params;

    return res;
}

node_t* harmful::parse_from(istream& s)
{
    vector<pair<node_t*, int>> node_stack;
    node_t* root = new node_t();
    node_stack.push_back(make_pair(root, -1));

    for (int line_i = 1; !s.eof(); line_i++) {
        string line;
        getline(s, line);
        int original_length = line.length();
        int indent = trim_indent(line);
        if (line[0] == '#' || indent == static_cast<int>(original_length)) {
            continue;
        }

        node_t* node = node_from_line(line);
        if (node == nullptr) {
            cerr << "Parse error on line " << line_i << endl;
            delete_tree(root);
            return nullptr;
        }

        if (indent == node_stack.back().second) {
            node_stack.pop_back();
        }

        while (!node_stack.empty() && indent <= node_stack.back().second) {
            node_stack.pop_back();
        }

        if (node_stack.empty()) {
            cerr << "Mismatching indentation on line " << line_i << endl;
            delete_tree(root);
            return nullptr;
        }

        node_stack.back().first->children.push_back(node);
        node_stack.push_back(make_pair(node, indent));
    }

    return root;
}
