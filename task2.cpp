// Task 2 (Weeks 7-13): Student Result Management System
// Concepts: inheritance (W7), virtual functions / polymorphism (W8), exception handling (W9),
//           file I/O (W10), vector + templates/STL (W11), STL algorithms + lambdas (W12),
//           smart pointers (W13)
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <exception>
#include <iomanip>
using namespace std;

const int SUBJECTS = 5;
const int PASS_MARK = 35;

// ---------- W9: custom exceptions ----------
class InvalidMarks : public exception {
    string msg;
public:
    InvalidMarks(int m) : msg("Invalid marks " + to_string(m) + " (must be 0-100)") {}
    const char* what() const noexcept override { return msg.c_str(); }
};
class FileError : public exception {
    string msg;
public:
    FileError(const string &f) : msg("Cannot open file '" + f + "'") {}
    const char* what() const noexcept override { return msg.c_str(); }
};

// ---------- W7: base class ----------
class Student {
protected:
    string name, usn;
    vector<int> marks;                                      // W11: vector of marks
public:
    Student(const string &n, const string &u, const vector<int> &m) : name(n), usn(u), marks(m) {
        for (int x : marks) if (x < 0 || x > 100) throw InvalidMarks(x);   // validate on creation
    }
    virtual ~Student() {}                                   // W8: virtual destructor
    virtual void display() const {                          // W8: virtual function
        cout << left << setw(12) << usn << setw(14) << name;
    }
    virtual double percentage() const {
        return accumulate(marks.begin(), marks.end(), 0) * 100.0 / (SUBJECTS * 100);
    }
    string getUSN() const { return usn; }
    string getName() const { return name; }
    const vector<int>& getMarks() const { return marks; }
};

// ---------- W7: derived class (Student IS-A base, Result adds grading) ----------
class Result : public Student {
    int total; double pct; string grade; bool passed;
public:
    Result(const string &n, const string &u, const vector<int> &m) : Student(n, u, m) {
        total = accumulate(marks.begin(), marks.end(), 0);
        pct = percentage();
        passed = all_of(marks.begin(), marks.end(), [](int x) { return x >= PASS_MARK; });   // W12 lambda
        if (!passed)       grade = "F";
        else if (pct >= 80) grade = "A+";
        else if (pct >= 70) grade = "A";
        else if (pct >= 60) grade = "B";
        else if (pct >= 50) grade = "C";
        else                grade = "D";
    }
    void display() const override {                         // W8: overrides base version
        Student::display();
        for (int x : marks) cout << setw(5) << x;
        cout << setw(7) << total << setw(8) << fixed << setprecision(1) << pct
             << setw(6) << grade << (passed ? "PASS" : "FAIL") << endl;
    }
    bool hasPassed() const { return passed; }
    string getGrade() const { return grade; }
    int getTotal() const { return total; }
};

using Record = unique_ptr<Student>;                         // W13: smart pointer, polymorphic storage

void header() {
    cout << left << setw(12) << "USN" << setw(14) << "Name";
    for (int i = 1; i <= SUBJECTS; ++i) cout << setw(5) << ("S" + to_string(i));
    cout << setw(7) << "Total" << setw(8) << "%" << setw(6) << "Gr" << "Status" << endl;
    cout << string(74, '-') << endl;
}

// W10: file output / input
void saveToFile(const vector<Record> &db, const string &file) {
    ofstream out(file);
    if (!out) throw FileError(file);
    for (auto &s : db) {
        out << s->getUSN() << "," << s->getName();
        for (int m : s->getMarks()) out << "," << m;
        out << "\n";
    }
}

int loadFromFile(vector<Record> &db, const string &file) {
    ifstream in(file);
    if (!in) throw FileError(file);
    string line; int lineNo = 0, loaded = 0;
    while (getline(in, line)) {
        ++lineNo;
        try {
            stringstream ss(line);
            string usn, name, tok; vector<int> m;
            getline(ss, usn, ','); getline(ss, name, ',');
            while (getline(ss, tok, ',')) m.push_back(stoi(tok));
            if ((int)m.size() != SUBJECTS) throw runtime_error("expected " + to_string(SUBJECTS) + " marks");
            db.push_back(make_unique<Result>(name, usn, m));
            ++loaded;
        } catch (const exception &e) {
            cout << "  Line " << lineNo << " skipped: " << e.what() << endl;
        }
    }
    return loaded;
}

void showAll(const vector<Record> &db, const string &title) {
    cout << "\n" << title << " (" << db.size() << ")\n";
    if (db.empty()) { cout << "  (no records)\n"; return; }
    header();
    for (auto &s : db) s->display();                        // W8: late binding picks Result::display
}

void addStudent(vector<Record> &db) {
    string name, usn; vector<int> m(SUBJECTS);
    cin.ignore(1000, '\n');
    cout << "Name: ";  getline(cin, name);
    cout << "USN: ";   getline(cin, usn);
    cout << "Enter " << SUBJECTS << " marks (0-100): ";
    for (int &x : m) cin >> x;
    try {
        db.push_back(make_unique<Result>(name, usn, m));
        cout << "  Record added for " << name << endl;
    } catch (const InvalidMarks &e) {
        cout << "  [ERROR] " << e.what() << " - record NOT added\n";
    }
}

int main() {
    vector<Record> db;
    int choice;
    do {
        cout << "\n===== STUDENT RESULT MANAGEMENT =====\n"
             << "1.Add  2.Display all  3.Save  4.Load  5.Distinction (>=70%)\n"
             << "6.Failed  7.Rank list  8.Class statistics  0.Exit\nChoice: ";
        if (!(cin >> choice)) break;
        try {
            switch (choice) {
            case 1: addStudent(db); break;
            case 2: showAll(db, "All students"); break;
            case 3: saveToFile(db, "students.csv"); cout << "  Saved " << db.size() << " records to students.csv\n"; break;
            case 4: {
                string f; cout << "File name: "; cin >> f;
                db.clear();
                int n = loadFromFile(db, f);                     // load first, then report
                cout << "  Loaded " << n << " records from " << f << endl;
                break; }
            case 5: {                                       // W12: filter by percentage
                vector<const Student*> v;
                for (auto &s : db) if (s->percentage() >= 70) v.push_back(s.get());
                cout << "\nDistinction students (>=70%): " << v.size() << endl;
                if (!v.empty()) header();
                for (auto *s : v) s->display();
                break; }
            case 6: {
                vector<const Student*> v;
                for (auto &s : db) { auto *r = dynamic_cast<Result*>(s.get()); if (r && !r->hasPassed()) v.push_back(r); }
                cout << "\nFailed students: " << v.size() << endl;
                if (!v.empty()) header();
                for (auto *s : v) s->display();
                break; }
            case 7: {                                       // W12: sort with lambda comparator
                vector<const Student*> v;
                for (auto &s : db) v.push_back(s.get());
                sort(v.begin(), v.end(), [](const Student *a, const Student *b) { return a->percentage() > b->percentage(); });
                cout << "\nRANK LIST\n";
                int rank = 1;
                for (auto *s : v) cout << "  " << rank++ << ". " << left << setw(14) << s->getName()
                                       << fixed << setprecision(1) << s->percentage() << "%\n";
                break; }
            case 8: {
                if (db.empty()) { cout << "  (no records)\n"; break; }
                double sum = accumulate(db.begin(), db.end(), 0.0, [](double a, const Record &s) { return a + s->percentage(); });
                auto best = max_element(db.begin(), db.end(), [](const Record &a, const Record &b) { return a->percentage() < b->percentage(); });
                int passed = count_if(db.begin(), db.end(), [](const Record &s) { auto *r = dynamic_cast<Result*>(s.get()); return r && r->hasPassed(); });
                cout << fixed << setprecision(2)
                     << "\n  Students       : " << db.size()
                     << "\n  Class average  : " << sum / db.size() << "%"
                     << "\n  Topper         : " << (*best)->getName() << " (" << (*best)->percentage() << "%)"
                     << "\n  Pass count     : " << passed << " / " << db.size() << endl;
                break; }
            case 0: cout << "Goodbye!\n"; break;
            default: cout << "  Invalid choice\n";
            }
        } catch (const exception &e) {                      // W9: any error in a menu action is reported, not fatal
            cout << "  [ERROR] " << e.what() << endl;
        }
    } while (choice != 0);
    return 0;
}
