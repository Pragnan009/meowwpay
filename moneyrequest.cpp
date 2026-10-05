#include "moneyrequest.h"
#include "user.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cctype>

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

MoneyRequest::MoneyRequest(string i, string f, string t, double a) {
    id = i;
    from = f;
    to = t;
    amount = a;
    status = "Pending";
}

bool RequestManager::validId(const string& id) {
    if (id.empty()) return false;
    for (char c : id) {
        if (isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

void RequestManager::requestMoney(string from, string to, double amount, vector<User>& users) {
    if (!validId(from) || !validId(to) || from == to || amount <= 0 || amount > 20000) {
        cout << "\n[!] Invalid request. Check the Wallet IDs and amount.\n";
        return;
    }
    bool senderFound = false, receiverFound = false;

    for (auto &u : users) {
        if (u.WalletId == from) senderFound = true;
        if (u.WalletId == to) receiverFound = true;
    }

    if (!senderFound || !receiverFound) {
        cout << "\n[!] User not found. Check the Wallet IDs.\n";
        return;
    }

    string id = "REQ" + to_string(reqCounter);
    MoneyRequest request(id, from, to, amount);
    requests.push_back(request);

    if (!saveRequests()) {
        requests.pop_back();
        cout << "\n[!] Request could not be saved. Please try again.\n";
        return;
    }

    reqCounter++;

    cout << "\n========== MONEY REQUEST ==========\n";
    cout << "Request ID : " << id << endl;
    cout << "From       : " << from << endl;
    cout << "To         : " << to << endl;
    cout << "Amount     : Rs. " << money(amount) << endl;
    cout << "Status     : Pending" << endl;
    cout << "===================================\n";
    cout << "[+] Money request created successfully!\n";
}

bool RequestManager::MoneyTransfer(string from, string to, double amount, vector<User>& users) {
    User* sender = nullptr;
    User* receiver = nullptr;

    for (auto &u : users) {
        if (u.WalletId == from) sender = &u;
        if (u.WalletId == to) receiver = &u;
    }

    if (sender == nullptr || receiver == nullptr) {
        cout << "\n[!] User not found.\n";
        return false;
    }

    if (!validId(from) || !validId(to) || from == to || amount <= 0 || amount > 20000) {
        cout << "\n[!] Invalid transfer details.\n";
        return false;
    }

    if (sender->balance < amount) {
        cout << "\n[!] Insufficient balance.\n";
        return false;
    }

    sender->balance -= amount;
    receiver->balance += amount;

    cout << "\n========== MONEY TRANSFER ==========\n";
    cout << "From      : " << from << endl;
    cout << "To        : " << to << endl;
    cout << "Amount    : Rs. " << money(amount) << endl;
    cout << "Status    : Successful" << endl;
    cout << "====================================\n";

    return true;
}

void RequestManager::respond(string id, string walletId, bool accept, vector<User>& users) {
    for (auto &x : requests) {
        if (x.id == id) {
            if (x.to != walletId) {
                cout << "\n[!] Access denied. You cannot respond to this request.\n";
                return;
            }

            if (x.status != "Pending") {
                cout << "\n[!] This request has already been processed.\n";
                return;
            }

            if (accept) {

                User* payer = nullptr;
                User* receiver = nullptr;

                for (auto &u : users) {
                    if (u.WalletId == x.to) payer = &u;
                    if (u.WalletId == x.from) receiver = &u;
                }

                if (payer == nullptr || receiver == nullptr) {
                    cout << "\n[!] User not found.\n";
                    return;
                }

                double oldPayerBalance = payer->balance;
                double oldReceiverBalance = receiver->balance;

                if (!MoneyTransfer(x.to, x.from, x.amount, users)) {
                    return;
                }

                x.status = "Accepted";

                if (!saveRequests()) {
                    payer->balance = oldPayerBalance;
                    receiver->balance = oldReceiverBalance;
                    x.status = "Pending";
                    cout << "\n[!] Could not save request status. Transfer was rolled back in memory.\n";
                    return;
                }

                cout << "\n[+] Money request accepted successfully!\n";
                cout << "[!] Remember to save the updated users using your project's user-save function.\n";
            } else {
                x.status = "Rejected";

                if (!saveRequests()) {
                    x.status = "Pending";
                    cout << "\n[!] Could not save request status. Please try again.\n";
                    return;
                }

                cout << "\n[-] Money request rejected.\n";
            }

            return;
        }
    }

    cout << "\n[!] Money request not found.\n";
}

void RequestManager::showRequests(string walletId) {
    cout << "\n============== MY REQUESTS ==============\n";
    cout << left
         << setw(12) << "ID"
         << setw(15) << "FROM"
         << setw(15) << "TO"
         << setw(15) << "AMOUNT"
         << setw(15) << "STATUS" << endl;
    cout << string(72, '-') << endl;

    bool found = false;

    for (auto &r : requests) {
        if (r.from == walletId) {
            cout << left
                 << setw(12) << r.id
                 << setw(15) << r.from
                 << setw(15) << r.to
                 << setw(15) << ("Rs. " + money(r.amount))
                 << setw(15) << r.status << endl;
            found = true;
        }
    }

    if (!found) cout << "No requests created by you.\n";

    cout << "\n=========== REQUESTS TO ME ==============\n";
    cout << left
         << setw(12) << "ID"
         << setw(15) << "FROM"
         << setw(15) << "TO"
         << setw(15) << "AMOUNT"
         << setw(15) << "STATUS" << endl;
    cout << string(72, '-') << endl;

    found = false;

    for (auto &r : requests) {
        if (r.to == walletId) {
            cout << left
                 << setw(12) << r.id
                 << setw(15) << r.from
                 << setw(15) << r.to
                 << setw(15) << ("Rs. " + money(r.amount))
                 << setw(15) << r.status << endl;
            found = true;
        }
    }

    if (!found) cout << "No requests sent to you.\n";
    cout << string(72, '=') << endl;
}

void RequestManager::cancelRequest(string id, string walletId) {
    for (auto &r : requests) {
        if (r.id == id && r.from == walletId) {
            if (r.status != "Pending") {
                cout << "\n[!] Request is already processed.\n";
                return;
            }

            r.status = "Cancelled";

            if (!saveRequests()) {
                r.status = "Pending";
                cout << "\n[!] Could not save cancellation. Please try again.\n";
                return;
            }

            cout << "\n[+] Request cancelled successfully.\n";
            return;
        }
    }

    cout << "\n[!] Request not found or you are not authorized to cancel it.\n";
}

bool RequestManager::saveRequests() {
    ofstream fout("requests.txt");

    if (!fout) {
        cout << "\n[!] Unable to open file for saving.\n";
        return false;
    }

    fout << fixed << setprecision(2);

    for (const auto &r : requests) {
        fout << r.id << " "
             << r.from << " "
             << r.to << " "
             << r.amount << " "
             << r.status << '\n';

        if (!fout) {
            cout << "\n[!] Error while writing to file.\n";
            return false;
        }
    }

    fout.close();

    if (!fout) {
        cout << "\n[!] Error while closing the file.\n";
        return false;
    }

    return true;
}

void RequestManager::loadRequests() {
    ifstream fin("requests.txt");

    if (!fin) return;

    requests.clear();
    reqCounter = 1;

    string id, from, to, status;
    double amount;
    bool skipped = false;

    while (fin >> id >> from >> to >> amount >> status) {
        if (!validId(id) || !validId(from) || !validId(to) ||
            amount <= 0 || amount > 20000) {
            skipped = true;
            continue;
        }
        MoneyRequest r(id, from, to, amount);
        r.status = status;
        requests.push_back(r);

        if (id.size() > 3 && id.substr(0, 3) == "REQ") {
            try {
                int number = stoi(id.substr(3));
                if (number >= reqCounter) reqCounter = number + 1;
            } catch (...) {

            }
        }
    }

    if (!fin.eof()) skipped = true;

    fin.close();

    if (skipped) {
        backupFile("requests.txt");
        cout << "\n[!] Some lines in requests.txt were invalid and skipped. The original file was saved as requests.txt.bak\n";
    }
}
