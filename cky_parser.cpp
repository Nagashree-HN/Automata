/**
 * CKY (Cocke-Kasami-Younger) Parser
 * Language: C++17
 *
 * Grammar (CNF):
 *   Binary rules:
 *     S   -> NP VP
 *     VP  -> V NP  |  V X1
 *     X1  -> NP PP
 *     NP  -> Det N  |  Det X2
 *     X2  -> N PP
 *     PP  -> P NP
 *
 *   Lexical rules:
 *     Det -> the | a | an
 *     V   -> discovers | identifies | books | revises | retrieves |
 *            displays | submits | lookup | issues | verifies |
 *            recommends | renews | directs | monitors
 *     P   -> in | from | to | through | for | by
 *     N   -> student | librarian | system | assistant | user | guide |
 *            biography | room | record | newspaper | location | thesis |
 *            novel | card | availability | journal | book | paper |
 *            library | author | section | database | archive | topic |
 *            evening | guidebook
 *
 * Compile:  g++ -std=c++17 -o cky_parser cky_parser.cpp
 * Run:      ./cky_parser
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iomanip>

// ======================================================================== //
//  Grammar definition                                                       //
// ======================================================================== //

// Binary rules stored as:  {B, C} -> list of A   (A -> B C)
using RuleKey = std::pair<std::string, std::string>;
static std::map<RuleKey, std::vector<std::string>> BINARY_RULES;

// Lexical rules:  word -> list of pre-terminal tags
static std::map<std::string, std::vector<std::string>> LEXICAL_RULES;

static bool grammarInitialised = false;

static void initGrammar() {
    if (grammarInitialised) return;
    grammarInitialised = true;

    // ----- Binary rules -----
    auto addB = [](const std::string& B, const std::string& C,
                   const std::string& A) {
        BINARY_RULES[{B, C}].push_back(A);
    };

    addB("NP", "VP", "S");
    addB("V",  "NP", "VP");
    addB("V",  "X1", "VP");
    addB("NP", "PP", "X1");
    addB("Det","N",  "NP");
    addB("Det","X2", "NP");
    addB("N",  "PP", "X2");
    addB("P",  "NP", "PP");

    // ----- Lexical rules -----
    auto addL = [](const std::string& tag,
                   std::initializer_list<std::string> words) {
        for (auto& w : words) LEXICAL_RULES[w].push_back(tag);
    };

    addL("Det", {"the","a","an"});

    addL("V", {"discovers","identifies","books","revises","retrieves",
               "displays","submits","lookup","issues","verifies",
               "recommends","renews","directs","monitors"});

    addL("P", {"in","from","to","through","for","by"});

    addL("N", {"student","librarian","system","assistant","user","guide",
               "biography","room","record","newspaper","location","thesis",
               "novel","card","availability","journal","book","paper",
               "library","author","section","database","archive","topic",
               "evening","guidebook"});
}

// ======================================================================== //
//  Utility: lowercase + tokenise                                            //
// ======================================================================== //

static std::string toLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

static std::vector<std::string> tokenise(const std::string& sentence) {
    std::vector<std::string> tokens;
    std::istringstream iss(sentence);
    std::string word;
    while (iss >> word) tokens.push_back(toLowerCase(word));
    return tokens;
}

// ======================================================================== //
//  CKY algorithm                                                            //
// ======================================================================== //

/**
 * Runs the CKY algorithm on a tokenised sentence.
 * Returns the full parse table.
 * table[i][j] = set of non-terminals that derive words[i..j].
 */
static std::vector<std::vector<std::set<std::string>>>
buildTable(const std::vector<std::string>& words) {
    int n = static_cast<int>(words.size());
    std::vector<std::vector<std::set<std::string>>>
        table(n, std::vector<std::set<std::string>>(n));

    // Step 1: diagonal – length-1 spans
    for (int i = 0; i < n; i++) {
        auto it = LEXICAL_RULES.find(words[i]);
        if (it != LEXICAL_RULES.end())
            for (auto& tag : it->second)
                table[i][i].insert(tag);
    }

    // Step 2: upper triangle – spans of length 2 to n
    for (int span = 2; span <= n; span++) {
        for (int i = 0; i <= n - span; i++) {
            int j = i + span - 1;
            for (int k = i; k < j; k++) {
                for (auto& B : table[i][k]) {
                    for (auto& C : table[k+1][j]) {
                        auto it = BINARY_RULES.find({B, C});
                        if (it != BINARY_RULES.end())
                            for (auto& A : it->second)
                                table[i][j].insert(A);
                    }
                }
            }
        }
    }
    return table;
}

/**
 * Primary parse function.
 * @param sentence  raw input string
 * @return          true if sentence is accepted by the grammar
 */
bool parse(const std::string& sentence) {
    initGrammar();
    auto words = tokenise(sentence);
    if (words.empty()) return false;
    auto table = buildTable(words);
    int n = static_cast<int>(words.size());
    return table[0][n-1].count("S") > 0;
}

// ======================================================================== //
//  Pretty-print the CKY table                                               //
// ======================================================================== //

static void printTable(const std::string& sentence) {
    initGrammar();
    auto words = tokenise(sentence);
    int n = static_cast<int>(words.size());
    if (n == 0) { std::cout << "(empty sentence)\n"; return; }

    auto table = buildTable(words);
    bool accepted = table[0][n-1].count("S") > 0;

    const int W = 16;   // column width

    // Header
    std::cout << std::left << std::setw(W) << "i\\j";
    for (int j = 0; j < n; j++) {
        std::string hdr = std::to_string(j+1) + ":\"" + words[j] + "\"";
        std::cout << std::setw(W) << hdr;
    }
    std::cout << "\n" << std::string(W * (n+1), '-') << "\n";

    // Rows
    for (int i = 0; i < n; i++) {
        std::string rowLbl = std::to_string(i+1) + ":\"" + words[i] + "\"";
        std::cout << std::setw(W) << rowLbl;
        for (int j = 0; j < n; j++) {
            if (j < i) {
                std::cout << std::setw(W) << "";
            } else {
                std::string cell;
                for (auto& s : table[i][j]) {
                    if (!cell.empty()) cell += ",";
                    cell += s;
                }
                if (cell.empty()) cell = "—";
                std::cout << std::setw(W) << cell;
            }
        }
        std::cout << "\n";
    }
    std::cout << std::string(W * (n+1), '-') << "\n";
    std::cout << "Cell[1," << n << "] = { ";
    for (auto& s : table[0][n-1]) std::cout << s << " ";
    std::cout << "}\n";
    std::cout << "Result: " << (accepted ? "YES – ACCEPTED" : "NO  – REJECTED") << "\n";
}

// ======================================================================== //
//  Test driver                                                               //
// ======================================================================== //

struct TestCase {
    std::string description;
    std::string tokens;       // lowercase, out-of-vocabulary words dropped
    bool expectedValid;
};

int main() {
    initGrammar();

    std::vector<TestCase> tests = {
        // --- 15 valid sentences ---
        {"1.  The student discovers a cybersecurity guide in the library",
         "the student discovers a guide in the library",           true},
        {"2.  The student identifies a biography by an Indian author",
         "the student identifies a biography by an author",        true},
        {"3.  The student books a study room for the evening",
         "the student books a room for the evening",               true},
        {"4.  The librarian revises a textbook record in the system",
         "the librarian revises a record in the system",           true},
        {"5.  The librarian retrieves an archived newspaper from the library",
         "the librarian retrieves an newspaper from the library",  true},
        {"6.  The system displays a book location in the database",
         "the system displays a location in the database",         true},
        {"7.  The student submits a thesis from the library",
         "the student submits a thesis from the library",          true},
        {"8.  The student lookup a novel in the mystery section",
         "the student lookup a novel in the section",              true},
        {"9.  The assistant issues a library card to the student",
         "the assistant issues a card to the student",             true},
        {"10. The student verifies a book availability in the system",
         "the student verifies a availability in the system",      true},
        {"11. The system recommends a research journal for the topic",
         "the system recommends a journal for the topic",          true},
        {"12. The student renews a borrowed book through the system",
         "the student renews a book through the system",           true},
        {"13. The user retrieves a research paper from the database",
         "the user retrieves a paper from the database",           true},
        {"14. The librarian directs a student to the archive section",
         "the librarian directs a student to the section",         true},
        {"15. The system monitors an overdue book in the database",
         "the system monitors an book in the database",            true},
        // --- 3 invalid sentences ---
        {"INVALID 1. Student searches book catalog",
         "student searches book catalog",                          false},
        {"INVALID 2. The system shows book in results",
         "the system shows book in results",                       false},
        {"INVALID 3. The user in catalog finds a book",
         "the user in catalog finds a book",                       false}
    };

    std::cout << std::string(72, '=') << "\n";
    std::cout << "  CKY PARSER  –  Library/University Domain Grammar  (C++)\n";
    std::cout << std::string(72, '=') << "\n";

    int correct = 0;
    for (auto& t : tests) {
        bool result  = parse(t.tokens);
        bool match   = (result == t.expectedValid);
        if (match) correct++;

        std::cout << "\nSentence : " << t.description  << "\n";
        std::cout << "Tokens   : " << t.tokens         << "\n";
        std::cout << "Result   : " << (result ? "YES (ACCEPTED)" : "NO  (REJECTED)") << "\n";
        std::cout << "Expected : " << (t.expectedValid ? "ACCEPTED" : "REJECTED")
                  << "  ->  " << (match ? "CORRECT" : "MISMATCH") << "\n";
        std::cout << std::string(72, '-') << "\n";
    }

    std::cout << "\nSummary: " << correct << " / " << tests.size()
              << " sentences matched expected outcome\n";
    std::cout << std::string(72, '=') << "\n";

    // Print a sample CKY table
    std::cout << "\n" << std::string(72, '=') << "\n";
    std::cout << "  SAMPLE CKY TABLE – Sentence 1\n";
    std::cout << std::string(72, '=') << "\n";
    printTable("the student discovers a guide in the library");

    // Interactive mode
    std::cout << "\n" << std::string(72, '=') << "\n";
    std::cout << "  INTERACTIVE MODE – type a sentence (or 'quit' to exit)\n";
    std::cout << std::string(72, '=') << "\n";
    std::cout << "> ";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit" || line.empty()) break;
        bool r = parse(line);
        std::cout << "Result: " << (r ? "YES – ACCEPTED" : "NO – REJECTED") << "\n> ";
    }

    return 0;
}