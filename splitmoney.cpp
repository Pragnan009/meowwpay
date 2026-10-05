#include "splitmoney.h"
#include "user.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cctype>
using namespace std;

bool SplitMoney::validId(const string& id) {
    if (id.empty()) return false;
    for (char c : id) if (isspace(static_cast<unsigned char>(c))) return false;
    return true;
}
bool SplitMoney::walletExists(const string& id, vector<User>& users) {
    for (auto& u : users) if (u.WalletId == id) return true;
    return false;
}
void SplitMoney::createEqual(string payer, vector<string> participants, int amount, vector<User>& users) {
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
    SplitExpense e;
    e.id = "SPLIT" + to_string(splitCounter);
    e.payerWalletId = payer; e.totalAmount = amount;
    int base = amount / (int)members.size(), rem = amount % (int)members.size();
    for (int i = 0; i < (int)members.size(); i++)
        e.shares.push_back({members[i], base + (i < rem), members[i] == payer});
    expenses.push_back(e);
    if (!saveSplits()) { expenses.pop_back(); cout << "\n[!] Could not save split.\n"; return; }
    splitCounter++;
    cout << "\n[+] Equal split created. Split ID: " << e.id << endl;
}
void SplitMoney::createCustom(string payer, vector<string> participants, vector<int> amounts, int total, vector<User>& users) {
    if (!validId(payer) || total <= 0 || total > 20000 || participants.empty() ||
        participants.size() != amounts.size() || !walletExists(payer, users)) {
        cout << "\n[!] Invalid custom split details.\n"; return;
    }
    vector<string> members{payer}; int sum = 0;
    for (int i = 0; i < (int)participants.size(); i++) {
        string id = participants[i];
        if (!validId(id) || !walletExists(id, users) || id == payer || amounts[i] <= 0) {
            cout << "\n[!] Invalid participant or share amount.\n"; return;
        }
        for (const string& old : members) if (old == id) {
            cout << "\n[!] Duplicate participant Wallet ID.\n"; return;
        }
        members.push_back(id); sum += amounts[i];
    }
    if (sum > total) { cout << "\n[!] Shares exceed total amount.\n"; return; }
    SplitExpense e;
    e.id = "SPLIT" + to_string(splitCounter); e.payerWalletId = payer; e.totalAmount = total;
    e.shares.push_back({payer, total - sum, true});
    for (int i = 0; i < (int)participants.size(); i++) e.shares.push_back({participants[i], amounts[i], false});
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
            int oldM = member->balance, oldP = payer->balance;
            member->balance -= s.amount; payer->balance += s.amount; s.paid = true;
            if (!saveSplits()) {
                member->balance = oldM; payer->balance = oldP; s.paid = false;
                cout << "\n[!] Save failed; changes rolled back in memory.\n"; return false;
            }
            cout << "\n[+] Share settled: Rs. " << s.amount << endl;
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
             << "\nTotal: Rs. " << e.totalAmount << '\n';
        cout << left << setw(18) << "Wallet ID" << setw(12) << "Share" << "Status\n";
        for (const auto& s : e.shares)
            cout << left << setw(18) << s.walletId << setw(12) << s.amount << (s.paid ? "Paid" : "Pending") << '\n';
    }
    if (!found) cout << "\nNo splits found.\n";
}
bool SplitMoney::saveSplits() {
    ofstream out("splits.txt");
    if (!out) return false;
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
    vector<SplitExpense> loaded; string id, payer; int total, count;
    while (in >> id >> payer >> total >> count) {
        if (!validId(id) || !validId(payer) || total <= 0 || total > 20000 || count <= 0) return;
        SplitExpense e; e.id = id; e.payerWalletId = payer; e.totalAmount = total; int sum = 0;
        for (int i = 0; i < count; i++) {
            SplitShare s;
            if (!(in >> s.walletId >> s.amount >> s.paid) || !validId(s.walletId) || s.amount < 0) return;
            sum += s.amount; e.shares.push_back(s);
        }
        if (sum != total) return;
        loaded.push_back(e);
    }
    if (!in.eof()) return;
    expenses = loaded; splitCounter = counter > 0 ? counter : 1;
}
