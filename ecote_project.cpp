#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <iostream>

using namespace std;

// NFA state struct
struct nfa_state {
    vector<int> a[26];// Next state when accepting a to z  
    vector<int> e;// Next state when accepting epsilon
    bool f = 0; // ==0 means the final state
};

vector<nfa_state> nfa;

set<char> alphabet;// Alphabet of the RE

string str;

set<int> accepted;

stack<int> st;

int nfa_size = 0;

struct nfa_state initial_nfa_state;

struct production{
    int a, b, c;
};

string syntax_info = "";

/***************************** Print NFA ****************************/

void print_nfa() {
    cout << "Regular expression to NFA: \n";
    cout << "States = {";
    for (int i = 0; i < nfa.size(); i++) {
        if (i)  
        cout << ", ";
        cout << char(i + 'A');
    }
    cout << "};\n";
    cout << "Inputs = {";
    auto it = alphabet.begin();
    cout << *it, it++;
    for (; it != alphabet.end(); it++) {
        cout << ", " << *it;
    }
    cout << "};\n";

    vector<production> ans;
    set<int> in; //In to the node
    for (int i = 0; i < nfa.size(); i++) {
        bool &f = nfa[i].f;
        for (int j = 0; j < 26; j++) {
            if (nfa[i].a[j].size() == 0)    
                continue;
            f |= 1;
            for (auto k : nfa[i].a[j]) {
                ans.push_back({i, j, k});
                in.insert(k);
            } 
        }
        if (nfa[i].e.size() == 0)   
            continue;
        for (auto j : nfa[i].e){
            ans.push_back({i, -1, j});
            in.insert(j);
        }     
        f |= 1;
    }

    cout << "Transition functions: ";
    for (int i = 0; i < ans.size(); i++) {
        if (i)  cout << ", ";
        cout << "f(";
        cout << char(ans[i].a + 'A');
        cout << ", ";
        if (ans[i].b == -1) cout << "ε";
        else    cout << char(ans[i].b + 'a');
        cout << ") = ";
        cout << char(ans[i].c + 'A');
    }
    cout << ";\n";

    // If there's no in it's the start
    for (int i = 0; i < nfa.size(); i++) {
        if (!in.count(i)) {
            cout << "Initial state = " <<char(i + 'A') << ";\n";
            break;
        }
    }

    cout << "Final state = {";
    vector<int> final;
    for (int i = 0; i < nfa.size(); i++) {
        if (!nfa[i].f)    
            final.push_back(i), accepted.insert(i);
    }
    for (int i = 0; i < final.size(); i++) {
        if (i)
            cout << ", ";
        cout << char(final[i] + 'A');
    }
    cout << "};\n";
    cout << endl;
}

/***************************** RE to NFA ****************************/

// Priority of stack 
int priority(char c) {
    switch (c) {
        case '*':
            return 3;
        case '.':
            return 2;
        case '|':
            return 1;
        default:
            return 0;
    }
}

string regular_expression_to_postfix(string regular_expression) {
    string postfix = "";
    stack<char> op;
    char c;
    for (unsigned int i = 0; i < regular_expression.size(); i++) {
        char ch = regular_expression[i];
        if (ch <= 'z' && ch >= 'a')     
            postfix += ch;
        else if (ch == '(')     
            op.push(ch);
        else if (ch == ')') {
            while (op.top() != '(') {
                postfix += op.top();
                op.pop();
            }
            op.pop();
        }
        else {
            while (!op.empty()) {
                c = op.top();
                if (priority(c) >= priority(ch)) {
                    postfix += op.top();
                    op.pop();
                } else break;
            }
            op.push(regular_expression[i]);
        }
    }
    while (!op.empty()) {
        postfix += op.top();
        op.pop();
    }
    return postfix;
}

// Insert hidden concatenations
string insert_concatination(string regular_expression) {
    string ret = "";
    char c; 
    char c2;
    for (int i = 0; i < regular_expression.size(); i++) {
        c = regular_expression[i];
        if (i + 1 < regular_expression.size()) {
            c2 = regular_expression[i + 1];
            ret += c;
            if (c != '(' && c2 != ')' && c != '|' && c2 != '|' && c2 != '*') {
                ret += '.';
            }
        }
    }
    ret += regular_expression[regular_expression.size() - 1];
    return ret;
}

// Process characters
void character(int i) {
    nfa.push_back(initial_nfa_state);
    nfa.push_back(initial_nfa_state);
    nfa[nfa_size].a[i].push_back(nfa_size + 1);
    st.push(nfa_size);
    //cout << char(i + 'a') << ' ' << nfa_size << ' ';
    nfa_size++;
    st.push(nfa_size);
    //cout << nfa_size << endl;
    nfa_size++;
    //print_nfa();
}

// Process unions
void union_() {
    nfa.push_back(initial_nfa_state);
    nfa.push_back(initial_nfa_state);

    int d = st.top();
    st.pop();
    int c = st.top();
    st.pop();
    int b = st.top();
    st.pop();
    int a = st.top();
    st.pop();
    //cout << "| " << a << ' ' << b << ' ' << c << ' ' << d << endl;

    nfa[nfa_size].e.push_back(a);
    nfa[nfa_size].e.push_back(c);
    st.push(nfa_size);
    nfa_size++;
    nfa[b].e.push_back(nfa_size);
    nfa[d].e.push_back(nfa_size);
    st.push(nfa_size);
    nfa_size++;

    //print_nfa();
}

// Process concatenations
void concatenation() {
    int d = st.top();
    st.pop();
    int c = st.top();
    st.pop();
    int b = st.top();
    st.pop();
    int a = st.top();
    st.pop();
    //cout << ". " << a << ' ' << b << ' ' << c << ' ' << d << endl;
    nfa[b].e.push_back(c);
    //nfa_size++;
    st.push(a);
    st.push(d);
    //print_nfa();
}

// Process Kleene stars
void kleene_star() {
    //Pop the first two
    //cout << st.size() << endl;
    int b = st.top();
    st.pop();
    int a = st.top();
    st.pop();
    //cout << "* " << a << ' ' << b << endl;
    //cout << nfa_size << endl;
    //Add three edges
    nfa.push_back(initial_nfa_state);
    nfa.push_back(initial_nfa_state);
    nfa[b].e.push_back(a);
    nfa[nfa_size].e.push_back(a);
    nfa[nfa_size].e.push_back(nfa_size + 1);
    nfa[b].e.push_back(nfa_size + 1);
    st.push(nfa_size);
    //cout << "** " << nfa_size << ' ';
    nfa_size++;
    st.push(nfa_size);
    //cout << nfa_size << endl;
    nfa_size++;
    //print_nfa();
    //cout << "------------------------------------\n";
}

void postfix_to_nfa(string postfix) {
    for (unsigned int i = 0; i < postfix.size(); i++) {
        char ch = postfix[i];
        if (ch <= 'z' && ch >= 'a')     
            character(ch - 'a');
        else if (ch == '*')     
            kleene_star();
        else if (ch == '.')     
            concatenation();
        else if (ch == '|')     
            union_();
    }
}

/***************************** RE Syntax  ****************************/

string regular_expression_syntax_check(const string& expression) {
    stack<char> parentheses;
    bool last_is_operator = true;  // Start true to prevent leading '|'
    bool last_is_letter = false;
    int consecutive_stars = 0;  // Counter for consecutive stars

    for (int i = 0; i < expression.length(); i++) {
        char c = expression[i];

        switch (c) {
            case '(':
                if (!last_is_operator && !last_is_letter) {
                    return "Invalid placement of '(': It must follow an operator or start the expression.";
                }
                parentheses.push(c);
                last_is_operator = true;
                last_is_letter = false;
                consecutive_stars = 0;
                break;

            case ')':
                if (last_is_operator || parentheses.empty()) {
                    if (parentheses.empty()) {
                        return "Unmatched ')': No corresponding opening parenthesis.";
                    }
                    return "Invalid placement of ')': It cannot immediately follow an operator.";
                }
                parentheses.pop();
                last_is_operator = false;
                last_is_letter = true;
                consecutive_stars = 0;
                break;

            case '*':
                consecutive_stars++;
                if (last_is_operator || consecutive_stars > 1) {
                    return "Invalid placement of '*': Consecutive stars are not allowed.";
                }
                // Allow '*' after a letter or after ')'
                if (!last_is_letter && (parentheses.empty() || expression[i-1] != ')')) {
                    return "Invalid placement of '*': It can only follow a letter or a closed parenthesis.";
                }
                last_is_operator = true;
                last_is_letter = false;
                break;

            case '|':
                if (last_is_operator || i == 0 || i == expression.length() - 1) {
                    return "Invalid placement of '|': It cannot be first, last, or follow another operator.";
                }
                last_is_operator = true;
                last_is_letter = false;
                consecutive_stars = 0;
                break;

            default:
                if (c < 'a' || c > 'z') {
                    return "Invalid character: Only lowercase letters 'a' to 'z' are allowed.";
                }
                last_is_operator = false;
                last_is_letter = true;
                consecutive_stars = 0;  // Reset on encountering a valid character
                break;
        }
    }

    if (!parentheses.empty()) {
        return "Unmatched '(': Not all opening parentheses have a matching closing parenthesis.";
    }
    if (last_is_operator && expression.back() != '*') {
        return "Expression cannot end with an operator.";
    }

    return "The regular expression is valid.";
}

/***************************** Solve ****************************/

// Clear everything befor the next RE
void clear() {
    nfa_size = 0;
    while(!st.empty())  
        st.pop();
    alphabet.clear();
    nfa.clear();
}

int initial_state;           // Initial state index in the NFA

// Function to calculate the epsilon closure of a set of states
std::set<int> epsilon_closure(std::set<int> states) {
    std::set<int> closure = states;
    std::vector<int> stack(states.begin(), states.end());

    while (!stack.empty()) {
        int top = stack.back();
        stack.pop_back();
        for (int eps_state : nfa[top].e) {
            if (closure.insert(eps_state).second) {
                stack.push_back(eps_state);
            }
        }
    }

    return closure;
}

// Matching function
bool re_match_check(const std::string& test_string) {
    // Calculate the initial epsilon-closure from the initial state
    std::set<int> current_states = epsilon_closure({initial_state});

    for (char c : test_string) {
        if (c < 'a' || c > 'z') { // Validate character
            return false;
        }

        std::set<int> next_states;
        // Transition based on the character and calculate new states
        for (int state : current_states) {
            const auto& transitions = nfa[state].a[c - 'a'];
            for (int next : transitions) {
                next_states.insert(next);
            }
        }

        // Calculate epsilon-closure for the states reached by the current character
        current_states = epsilon_closure(next_states);
    }

    // Check if any current states are final states
    for (int state : current_states) {
        if (nfa[state].f) {
            return true;
        }
    }

    return false;
}

void solve() {
    clear();
    string regular_expression = str;  // 'str' should already contain the regex from the '@' line
    string postfix;

    // Remove "@" 
    regular_expression = regular_expression.substr(1);
    cout <<  "Regular expression: " << regular_expression << endl;

    // Extract alphabet from the regular expression
    for (char i : regular_expression) {
        if (i >= 'a' && i <= 'z')   
            alphabet.insert(i);
    }

    syntax_info = regular_expression_syntax_check(regular_expression);

    if (syntax_info == "The regular expression is valid.") {
        regular_expression = insert_concatination(regular_expression);
        // cout << regular_expression << endl;
        postfix = regular_expression_to_postfix(regular_expression);
        cout << "Postfix expression: " << postfix << endl;
        postfix_to_nfa(postfix);
        print_nfa();
    } else {
        cout << syntax_info << endl;
    }
}

/***************************** Files I/O ****************************/

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    while (cin >> str) {
        if (!str.empty() && str[0] == '@'){
            cout << "..........................................\n";
            solve();
        }else if (!str.empty() && str[0] != '@' && syntax_info == "The regular expression is valid."){
            bool isMatch = re_match_check(str);  
            cout << "String \"" << str << "\" matches: " << (isMatch ? "Yes" : "No") << endl;
        }
        
    }
}