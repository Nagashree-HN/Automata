#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>

using namespace std;

map<string, vector<string>> rhsToLhs;

void addRule(string lhs, string rhs) {
    rhsToLhs[rhs].push_back(lhs);
}

vector<string> getLHS(string rhs) {
    if (rhsToLhs.count(rhs)) return rhsToLhs[rhs];
    return {};
}

string toLowerStr(string s) {
    for (auto &c : s) c = tolower(c);
    return s;
}

int main() {
    
    //  GRAMMAR IN STRICT CNF
    

    // ----- Start rule -----
    addRule("S", "NP VP");

    // ----- NP Rules -----
    addRule("NP", "Det Nominal");

    // ----- Nominal Rules -----
    addRule("Nominal", "Nominal Noun"); // Compound nouns(e.g,study room)
    addRule("Nominal", "N1 Noun");
    addRule("N1", "Nominal Noun");

    addRule("Nominal", "Nominal PP");   // Nouns with prepositions
    addRule("Nominal", "N2 PP");
    addRule("N2", "Nominal PP");

    // ----- VP Rules -----
    addRule("VP", "Verb NP");
    addRule("VP", "Verb PP");
    addRule("VP", "V1 NP");
    addRule("V1", "Verb NP");
    addRule("VP", "VP PP");

    // ----- PP Rule -----
    addRule("PP", "Preposition NP");

    //LEXICON Rules


    // Determiners
    string determiners[] = {"the", "a", "an"};
    for(string w : determiners) { addRule("Det", w); }

    // Nouns (Also acts as Nominal and NP)
    string nouns[] = {
        "student", "librarian", "system", "assistant", "user", "guide", 
        "biography", "room", "record", "newspaper", "location", "thesis", 
        "novel", "card", "availability", "journal", "book", "paper", 
        "library", "author", "section", "database", "archive", "topic", 
        "evening", "catalog", "results", "cybersecurity", "study", 
        "guidebook", "textbook", "mystery", "research", "overdue", 
        "borrowed", "archived", "indian", "allen", "public"
    };
    for(string w : nouns) { 
        addRule("Noun", w); 
        addRule("Nominal", w); 
        addRule("NP", w); 
    }

    // Proper Nouns (Also acts as Nominal and NP)
    string properNouns[] = {"nagashree", "sandeep"};
    for(string w : properNouns) { 
        addRule("ProperNoun", w); 
        addRule("Nominal", w); 
        addRule("NP", w); 
    }

    // Pronouns (Also acts as NP)
    string pronouns[] = {"them"};
    for(string w : pronouns) { 
        addRule("Pronoun", w); 
        addRule("NP", w); 
    }

    //Verbs (Also acts as VP and S for imperatives)
    string verbs[] = {
        "discovers", "identifies", "books", "revises", "retrieves", 
        "displays", "submits", "locates", "issues", "verifies", 
        "recommends", "renews", "directs", "monitors", "finds", 
        "shows", "searches"
    };
    for(string w : verbs) { 
        addRule("Verb", w); 
        addRule("VP", w); 
        addRule("S", w); 
    }

    //Prepositions
    string prepositions[] = {"in", "from", "to", "through", "for", "by", "at"};
    for(string w : prepositions) { addRule("Preposition", w); }

    cout << "Enter sentence (e.g., 'sandeep locates a novel'):\n> ";
    string input;
    getline(cin, input);
    input = toLowerStr(input);

    stringstream ss(input);
    vector<string> words;
    string w;
    while (ss >> w) words.push_back(w);

    int n = words.size();
    if (n == 0) {
        cout << "NO - Invalid Sentence (empty input)\n";
        return 0;
    }

    //CKY table
    vector<vector<set<string>>> table(n, vector<set<string>>(n));
    
    for (int i = 0; i < n; i++) {

        for (auto &tag : getLHS(words[i]))
            table[i][i].insert(tag);
    }

    for (int len = 2; len <= n; len++) {
        for (int i = 0; i <= n - len; i++) {
            int j = i + len - 1;
            for (int k = i; k < j; k++) {
                for (auto &B : table[i][k]) {
                    for (auto &C : table[k + 1][j]) {
                        for (auto &A : getLHS(B + " " + C))
                            table[i][j].insert(A);
                    }
                }
            }
        }
    }


    cout << "\n--- CKY Parse Table ---\n";
    int colW = 18;
    cout << left << setw(colW) << "i\\j";
    for (int j = 0; j < n; j++)
        cout << setw(colW) << (to_string(j+1) + ":\"" + words[j] + "\"");
    cout << "\n" << string(colW * (n + 1), '-') << "\n";

    for (int i = 0; i < n; i++) {
        cout << setw(colW) << (to_string(i+1) + ":\"" + words[i] + "\"");
        for (int j = 0; j < n; j++) {
            if (j < i) {
                cout << setw(colW) << "";
            } else {
                string cell;
                for (auto &sym : table[i][j]) {
                    if (!cell.empty()) cell += ",";
                    cell += sym;
                }
                cout << setw(colW) << (cell.empty() ? "-" : cell);
            }
        }
        cout << "\n";
    }
    cout << string(colW * (n + 1), '-') << "\n";

  
    cout << "\nInput   : \"" << input << "\"\n";
    if (table[0][n - 1].count("S"))
        cout << "Result  : YES - Valid Sentence\n";
    else
        cout << "Result  : NO  - Invalid Sentence\n";

    return 0;
}