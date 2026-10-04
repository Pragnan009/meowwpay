#include "moneyrequest.h"
#include "user.h"
#include <iostream>
#include <fstream>
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
        cout << "Error" << endl;
        return;
    }

    string id = "REQ" + to_string(reqCounter++);
    requests.push_back(MoneyRequest(id, from, to, amount));

    saveRequests();
    cout << "Money Request created successfully :)" << endl;
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
        cout << "User not found" << endl;
        return false;
    }

    if (from == to || amount <= 0) {
        cout << "Invalid transfer" << endl;
        return false;
    }

    if (sender->balance < amount) {
        cout << "Insufficient balance" << endl;
        return false;
    }

    sender->balance -= amount;
    receiver->balance += amount;

    cout << "Money transferred successfully" << endl;
    return true;
}

void RequestManager::respond(string id, string userId, bool accept, vector<User>& users) {
    for (auto &x : requests) {
        if (x.id == id) {

            if (x.to != userId) {
                cout << "Not authorized" << endl;
                return;
            }

            if (x.status != "Pending") {
                cout << "Request is already processed" << endl;
                return;
            }

            if (accept) {
                if (MoneyTransfer(x.to, x.from, x.amount, users)) {
                    x.status = "Accepted";
                    saveRequests();
                    cout << "Money request accepted" << endl;
                }
            }
            else {
                x.status = "Rejected";
                saveRequests();
                cout << "Money request rejected" << endl;
            }
            return;
        }
    }

    cout << "Money request not found" << endl;
}

void RequestManager::showRequests(string userId) {
    cout << "Money requests by me:" << endl;

    for (auto &r : requests) {
        if (r.from == userId) {
            cout << r.id << " " << r.from << " "
                 << r.to << " " << r.amount << " "
                 << r.status << endl;
        }
    }

    cout << "___________________________________________________" << endl;
    cout << "Money requests sent to me:" << endl;

    for (auto &r : requests) {
        if (r.to == userId) {
            cout << r.id << " " << r.from << " "
                 << r.to << " " << r.amount << " "
                 << r.status << endl;
        }
    }
}

void RequestManager::cancelRequest(string id, string userId) {
    for (auto &r : requests) {
        if (r.id == id && r.from == userId) {
            if (r.status == "Pending") {
                r.status = "Cancelled";
                saveRequests();
                cout << "Request cancelled" << endl;
            }
            else {
                cout << "Request is already processed" << endl;
            }
            return;
        }
    }

    cout << "Request not found" << endl;
}

void RequestManager::saveRequests() {
    ofstream fout("requests.txt");

    if (!fout) {
        cout << "Unable to open file for saving" << endl;
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
        cout << "Error while writing to file" << endl;
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
