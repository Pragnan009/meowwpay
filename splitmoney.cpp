#include "splitmoney.h"
#include "user.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <cmath>
using namespace std;

static string money(double a) {
    ostringstream os;
    os << fixed << setprecision(2) << a;
    return os.str();
}

static void backupFile(const string& path) {
    ifstream in(path, ios::binary);
    if (!in) return;
    ofstream out(path + ".bak", ios::binary);
    out << in.rdbuf();
}

bool SplitMoney::validId(const string& id) {
    if (id.empty()) return false;
    for (char c : id) if (isspace(static_cast<unsigned char>(c))) return false;
    return true;
}
bool SplitMoney::walletExists(const string& id, vector<User>& users) {
    for (auto& u : users) if (u.WalletId == id) return true;
    return false;
}
void SplitMoney::createEqual(string payer, vector<string> participants, double amount, vector<User>& users) {
    if (!validId(payer) || amount <= 0 || amount > 20000 || participants.empty() || !walletExists(payer, users)) {
        cout << "\n[!] Invalid split details or payer Wallet ID.\n"; return;
    }
    vector<string> members{payer};
    for (const string& id : participants) {
        if (!validId(id) || !walletExists(id, users) || id == payer) {
            cout << "\n[!] Invalid participant Wallet ID.\n"; return;
        }
        for (const string& old : members) if (old == id) {
            cout << "\n[!] Duplicate participant Wallet ID.\n"; return;
        }
        members.push_back(id);
    }
    long long totalP = llround(amount * 100), n = (long long)members.size();
    if (totalP < n) {
        cout << "\n[!] Amount is too small to split between this many people.\n"; return;
    }
    SplitExpense e;
    e.id = "SPLIT" + to_string(splitCounter);
    e.payerWalletId = payer; e.totalAmount = totalP / 100.0;
    long long base = totalP / n, rem = totalP % n;
    for (long long i = 0; i < n; i++)
        e.shares.push_back({members[i], (base + (i < rem)) / 100.0, members[i] == payer});
    expenses.push_back(e);
    if (!saveSplits()) { expenses.pop_back(); cout << "\n[!] Could not save split.\n"; return; }
    splitCounter++;
    cout << "\n[+] Equal split created. Split ID: " << e.id << endl;
}
void SplitMoney::createCustom(string payer, vector<string> participants, vector<double> amounts, double total, vector<User>& users) {
    if (!validId(payer) || total <= 0 || total > 20000 || participants.empty() ||
        participants.size() != amounts.size() || !walletExists(payer, users)) {
        cout << "\n[!] Invalid custom split details.\n"; return;
    }
    vector<string> members{payer}; long long sum = 0;
    for (int i = 0; i < (int)participants.size(); i++) {
        string id = participants[i];
        if (!validId(id) || !walletExists(id, users) || id == payer || llround(amounts[i] * 100) <= 0) {
            cout << "\n[!] Invalid participant or share amount.\n"; return;
        }
        for (const string& old : members) if (old == id) {
            cout << "\n[!] Duplicate participant Wallet ID.\n"; return;
        }
        members.push_back(id); sum += llround(amounts[i] * 100);
    }
    long long totalP = llround(total * 100);
    if (sum > totalP) { cout << "\n[!] Shares exceed total amount.\n"; return; }
    SplitExpense e;
    e.id = "SPLIT" + to_string(splitCounter); e.payerWalletId = payer; e.totalAmount = totalP / 100.0;
    e.shares.push_back({payer, (totalP - sum) / 100.0, true});
    for (int i = 0; i < (int)participants.size(); i++) e.shares.push_back({participants[i], llround(amounts[i] * 100) / 100.0, false});
    expenses.push_back(e);
    if (!saveSplits()) { expenses.pop_back(); cout << "\n[!] Could not save split.\n"; return; }
    splitCounter++;
    cout << "\n[+] Custom split created. Split ID: " << e.id << endl;
}
bool SplitMoney::settleShare(string splitId, string walletId, vector<User>& users) {
    for (auto& e : expenses) if (e.id == splitId) {
        if (walletId == e.payerWalletId) { cout << "\n[!] Payer's share is already paid.\n"; return false; }
        for (auto& s : e.shares) if (s.walletId == walletId) {
            if (s.paid) { cout << "\n[!] Share already paid.\n"; return false; }
            User *payer = nullptr, *member = nullptr;
            for (auto& u : users) {
                if (u.WalletId == e.payerWalletId) payer = &u;
                if (u.WalletId == walletId) member = &u;
            }
            if (!payer || !member) { cout << "\n[!] Wallet ID not found.\n"; return false; }
            if (member->balance < s.amount) { cout << "\n[!] Insufficient balance.\n"; return false; }
            double oldM = member->balance, oldP = payer->balance;
            member->balance -= s.amount; payer->balance += s.amount; s.paid = true;
            if (!saveSplits()) {
                member->balance = oldM; payer->balance = oldP; s.paid = false;
                cout << "\n[!] Save failed; changes rolled back in memory.\n"; return false;
            }
            cout << "\n[+] Share settled: Rs. " << money(s.amount) << endl;
            cout << "[!] Save updated users using your user-save function.\n"; return true;
        }
        cout << "\n[!] Wallet ID is not part of this split.\n"; return false;
    }
    cout << "\n[!] Split not found.\n"; return false;
}
void SplitMoney::showSplits(string walletId) {
    bool found = false;
    for (const auto& e : expenses) {
        bool member = false;
        for (const auto& s : e.shares) if (s.walletId == walletId) member = true;
        if (!member) continue;
        found = true;
        cout << "\nSplit ID: " << e.id << "\nPayer: " << e.payerWalletId
             << "\nTotal: Rs. " << money(e.totalAmount) << '\n';
        cout << left << setw(18) << "Wallet ID" << setw(12) << "Share" << "Status\n";
        for (const auto& s : e.shares)
            cout << left << setw(18) << s.walletId << setw(12) << money(s.amount) << (s.paid ? "Paid" : "Pending") << '\n';
    }
    if (!found) cout << "\nNo splits found.\n";
}
bool SplitMoney::saveSplits() {
    ofstream out("splits.txt");
    if (!out) return false;
    out << fixed << setprecision(2);
    out << splitCounter << '\n';
    for (const auto& e : expenses) {
        out << e.id << ' ' << e.payerWalletId << ' ' << e.totalAmount << ' ' << e.shares.size() << '\n';
        for (const auto& s : e.shares) out << s.walletId << ' ' << s.amount << ' ' << s.paid << '\n';
    }
    out.close(); return (bool)out;
}
void SplitMoney::loadSplits() {
    ifstream in("splits.txt"); if (!in) return;
    int counter; if (!(in >> counter)) return;
    vector<SplitExpense> loaded; string id, payer; double total; int count;
    bool corrupt = false;
    while (!corrupt && (in >> id >> payer >> total >> count)) {
        if (!validId(id) || !validId(payer) || total <= 0 || total > 20000 || count <= 0) { corrupt = true; break; }
        SplitExpense e; e.id = id; e.payerWalletId = payer; e.totalAmount = total; long long sum = 0;
        for (int i = 0; i < count; i++) {
            SplitShare s;
            if (!(in >> s.walletId >> s.amount >> s.paid) || !validId(s.walletId) || s.amount < 0) { corrupt = true; break; }
            sum += llround(s.amount * 100); e.shares.push_back(s);
        }
        if (corrupt) break;
        if (sum != llround(total * 100)) { corrupt = true; break; }
        loaded.push_back(e);
    }
    if (!corrupt && !in.eof()) corrupt = true;
    in.close();
    if (corrupt) {
        backupFile("splits.txt");
        cout << "\n[!] splits.txt is invalid, so no splits were loaded. The original file was saved as splits.txt.bak\n";
        return;
    }
    expenses = loaded; splitCounter = counter > 0 ? counter : 1;
}
