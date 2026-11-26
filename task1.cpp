// Task 1 (Weeks 1-6): Bank Account Management System
// Concepts: string & references (W1), overloading / default args / inline (W2),
//           classes & encapsulation (W3), constructors, destructor, deep copy (W4),
//           static members & friend functions (W5), operator overloading (W6)
#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
using namespace std;

inline bool validAmount(double a) { return a > 0; }                  // W2: inline helper

string toUpperCase(const string &s) {                                // W1: string + const reference
    string r = s;
    for (char &c : r) c = toupper((unsigned char)c);
    return r;
}

class Account {
private:                                                             // W3: encapsulation
    int accNo;
    string owner;
    double balance;
    double *history;                                                 // W4: dynamic array of transactions
    int histCount, histCap;
    static int nextAccNo;                                            // W5: shared by all objects
    static int totalAccounts;

    void record(double amt) {                                        // grow the array when full
        if (histCount == histCap) {
            histCap *= 2;
            double *bigger = new double[histCap];
            for (int i = 0; i < histCount; ++i) bigger[i] = history[i];
            delete[] history;
            history = bigger;
        }
        history[histCount++] = amt;
    }

public:
    Account(const string &name = "Unknown", double opening = 0)      // W2 default args + W4 constructor
        : accNo(nextAccNo++), owner(toUpperCase(name)), balance(0),
          history(new double[2]), histCount(0), histCap(2) {
        ++totalAccounts;
        if (validAmount(opening)) { balance = opening; record(opening); }
        cout << "  [ctor] Account " << accNo << " opened for " << owner << endl;
    }
    Account(const Account &o)                                        // W4: deep copy constructor
        : accNo(nextAccNo++), owner(o.owner + " (COPY)"), balance(o.balance),
          history(new double[o.histCap]), histCount(o.histCount), histCap(o.histCap) {
        for (int i = 0; i < histCount; ++i) history[i] = o.history[i];
        ++totalAccounts;
        cout << "  [copy ctor] Account " << accNo << " cloned from " << o.accNo << endl;
    }
    Account& operator=(const Account &o) {                           // W6: deep copy assignment
        if (this != &o) {
            delete[] history;
            owner = o.owner; balance = o.balance;
            histCap = o.histCap; histCount = o.histCount;
            history = new double[histCap];
            for (int i = 0; i < histCount; ++i) history[i] = o.history[i];
        }
        return *this;
    }
    ~Account() {                                                     // W4: destructor frees memory
        delete[] history;
        --totalAccounts;
        cout << "  [dtor] Account " << accNo << " (" << owner << ") closed" << endl;
    }

    // ---- W2: function overloading and default arguments ----
    bool deposit(double amt) {
        if (!validAmount(amt)) return false;
        balance += amt; record(amt); return true;
    }
    bool deposit(double amt, const string &note) {                   // overload: deposit with a note
        bool ok = deposit(amt);
        if (ok) cout << "  note: " << note << endl;
        return ok;
    }
    bool withdraw(double amt, double fee = 0) {                      // default argument: no fee
        double total = amt + fee;
        if (!validAmount(amt) || total > balance) return false;
        balance -= total; record(-total); return true;
    }
    void display() const {
        cout << "  Acc " << accNo << " | " << setw(8) << left << owner << " | Rs. " << fixed << setprecision(2) << balance << endl;
    }
    void display(bool showHistory) const {                           // overload
        display();
        if (showHistory) {
            cout << "    history:";
            for (int i = 0; i < histCount; ++i) cout << " " << showpos << history[i] << noshowpos;
            cout << endl;
        }
    }

    // getters (const-correct)
    double getBalance() const { return balance; }
    int getAccNo() const { return accNo; }
    static int getTotal() { return totalAccounts; }                  // W5: static member function

    // ---- W6: operator overloading ----
    Account& operator+=(double amt) { deposit(amt); return *this; }
    Account& operator-=(double amt) { withdraw(amt); return *this; }
    bool operator>(const Account &o) const { return balance > o.balance; }
    bool operator==(const Account &o) const { return balance == o.balance; }
    Account& operator++() { deposit(balance * 0.01); return *this; } // unary: add 1% interest
    double operator[](int i) const {                                 // subscript: i-th transaction
        if (i < 0 || i >= histCount) { cout << "  index out of range!" << endl; return 0; }
        return history[i];
    }
    friend ostream& operator<<(ostream &os, const Account &a) {
        return os << "[" << a.accNo << ":" << a.owner << " Rs." << fixed << setprecision(2) << a.balance << "]";
    }
    friend bool transfer(Account &from, Account &to, double amt);    // W5: friend function
};

int Account::nextAccNo = 1001;
int Account::totalAccounts = 0;

bool transfer(Account &from, Account &to, double amt) {              // reads private data via friendship
    if (!validAmount(amt) || amt > from.balance) return false;
    from.balance -= amt; from.record(-amt);
    to.balance   += amt; to.record(amt);
    return true;
}

Account& richer(Account &a, Account &b) { return (a > b) ? a : b; } // W1: returns a reference

void minMaxBalance(const Account list[], int n, double &mn, double &mx) {   // W1/W2: two outputs via references
    mn = mx = list[0].getBalance();
    for (int i = 1; i < n; ++i) {
        if (list[i].getBalance() < mn) mn = list[i].getBalance();
        if (list[i].getBalance() > mx) mx = list[i].getBalance();
    }
}

int main() {
    cout << "=== 1. Constructors and static counter ===" << endl;
    Account a("asha", 5000), b("ravi", 2500);
    Account c;                                         // default constructor arguments
    cout << "  Accounts alive: " << Account::getTotal() << endl;

    cout << "\n=== 2. Overloading and default arguments ===" << endl;
    a.deposit(1500);
    b.deposit(500, "salary credited");                 // overloaded deposit
    a.withdraw(1000);                                  // fee defaults to 0
    b.withdraw(300, 15);                               // explicit fee
    cout << "  withdraw(99999) on Ravi: " << (b.withdraw(99999) ? "done" : "denied (insufficient)") << endl;
    a.display(); b.display(true);                      // two overloads of display

    cout << "\n=== 3. Operator overloading ===" << endl;
    a += 250;
    b -= 100;
    ++a;                                               // adds 1% interest
    cout << "  " << a << "  " << b << endl;
    cout << "  Asha > Ravi ? " << (a > b ? "yes" : "no") << endl;
    cout << "  Ravi's 1st transaction = " << b[0] << ", index 9 -> ";
    b[9];

    cout << "\n=== 4. Friend function (transfer) ===" << endl;
    transfer(a, c, 2000);
    cout << "  After transfer: " << a << " " << c << endl;

    cout << "\n=== 5. Deep copy ===" << endl;
    Account d(b);                                      // copy constructor
    d += 1000;                                         // changes only the copy
    cout << "  Original " << b << "  Copy " << d << endl;

    cout << "\n=== 6. References ===" << endl;
    cout << "  Richer of Asha and Ravi: " << richer(a, b) << endl;
    Account all[3] = { Account("x", 100), Account("y", 900), Account("z", 450) };
    double mn, mx;
    minMaxBalance(all, 3, mn, mx);
    cout << "  Min balance = " << mn << ", Max balance = " << mx << endl;

    cout << "\n=== 7. Destructors run at end of main ===" << endl;
    cout << "  Accounts alive before exit: " << Account::getTotal() << endl;
    return 0;
}
