
#include "moneyrequest.h"
#include "user.h"
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

MoneyRequest::MoneyRequest(string i, string f, string t, int a) {
    id = i;
    from = f;
    to = t;
    amount = a;
    status = "Pending";
}

void RequestManager::requestMoney(string from, string to, int amount) {
    if (from == to || amount <= 0) {
        cout << "\n[!] Invalid request. Check the user IDs and amount.\n";
        return;
    }

    string id = "REQ" + to_string(reqCounter++);
    requests.push_back(MoneyRequest(id, from, to, amount));

    saveRequests();

    cout << "\n========== MONEY REQUEST ==========\n";
    cout << "Request ID : " << id << endl;
    cout << "From       : " << from << endl;
    cout << "To         : " << to << endl;
    cout << "Amount     : Rs. " << amount << endl;
    cout << "Status     : Pending" << endl;
    cout << "===================================\n";
    cout << "[+] Money request created successfully!\n";
}

bool RequestManager::MoneyTransfer(string from, string to, int amount, vector<User>& users) {
    User* sender = nullptr;
    User* receiver = nullptr;

    for (auto &u : users) {
        if (u.id == from) {
            sender = &u;
        }
        if (u.id == to) {
            receiver = &u;
        }
    }

    if (sender == nullptr || receiver == nullptr) {
        cout << "\n[!] User not found.\n";
        return false;
    }

    if (from == to || amount <= 0) {
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
    cout << "Amount    : Rs. " << amount << endl;
    cout << "Status    : Successful" << endl;
    cout << "====================================\n";

    return true;
}

void RequestManager::respond(string id, string userId, bool accept, vector<User>& users) {
    for (auto &x : requests) {
        if (x.id == id) {

            if (x.to != userId) {
                cout << "\n[!] Access denied. You cannot respond to this request.\n";
                return;
            }

            if (x.status != "Pending") {
                cout << "\n[!] This request has already been processed.\n";
                return;
            }

            if (accept) {
                if (MoneyTransfer(x.to, x.from, x.amount, users)) {
                    x.status = "Accepted";
                    saveRequests();
                    cout << "\n[+] Money request accepted successfully!\n";
                }
            }
            else {
                x.status = "Rejected";
                saveRequests();
                cout << "\n[-] Money request rejected.\n";
            }

            return;
        }
    }

    cout << "\n[!] Money request not found.\n";
}

void RequestManager::showRequests(string userId) {
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
        if (r.from == userId) {
            cout << left
                 << setw(12) << r.id
                 << setw(15) << r.from
                 << setw(15) << r.to
                 << setw(15) << ("Rs. " + to_string(r.amount))
                 << setw(15) << r.status << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "No requests created by you.\n";
    }

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
        if (r.to == userId) {
            cout << left
                 << setw(12) << r.id
                 << setw(15) << r.from
                 << setw(15) << r.to
                 << setw(15) << ("Rs. " + to_string(r.amount))
                 << setw(15) << r.status << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "No requests sent to you.\n";
    }

    cout << string(72, '=') << endl;
}

void RequestManager::cancelRequest(string id, string userId) {
    for (auto &r : requests) {
        if (r.id == id && r.from == userId) {
            if (r.status == "Pending") {
                r.status = "Cancelled";
                saveRequests();
                cout << "\n[+] Request cancelled successfully.\n";
            }
            else {
                cout << "\n[!] Request is already processed.\n";
            }
            return;
        }
    }

    cout << "\n[!] Request not found or you are not authorized to cancel it.\n";
}

void RequestManager::saveRequests() {
    ofstream fout("requests.txt");

    if (!fout) {
        cout << "\n[!] Unable to open file for saving.\n";
        return;
    }

    for (auto &r : requests) {
        fout << r.id << " "
             << r.from << " "
             << r.to << " "
             << r.amount << " "
             << r.status << endl;
    }

    if (!fout) {
        cout << "\n[!] Error while writing to file.\n";
    }

    fout.close();
}

void RequestManager::loadRequests() {
    ifstream fin("requests.txt");

    if (!fin) {
        return;
    }

    requests.clear();
    reqCounter = 1;

    string id, from, to, status;
    int amount;

    while (fin >> id >> from >> to >> amount >> status) {
        MoneyRequest r(id, from, to, amount);
        r.status = status;
        requests.push_back(r);

        if (id.substr(0, 3) == "REQ") {
            try {
                int number = stoi(id.substr(3));
                if (number >= reqCounter) {
                    reqCounter = number + 1;
                }
            }
            catch (...) {
            }
        }
    }

    fin.close();
}
