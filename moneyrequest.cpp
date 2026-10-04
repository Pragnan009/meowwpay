#include "moneyrequest.h"
#include <iostream>
#include<fstream>
#include "User.h"
using namespace std;

MoneyRequest::MoneyRequest(int i,int f,int t,int a){
    id =i;
    from =f;
    to=t;
    amount=a;
    status="Pending";
}

void RequestManager::requestMoney(int from,int to, int amount){
    if(from == to || amount <= 0 || amount >= 20000){ cout<<"Error"<<endl;return;}
    requests.push_back(MoneyRequest(reqid++,from,to,amount));
    saveRequests();
    cout<<"Money Request created succesfully :)"<<endl;
}

void RequestManager::respond(int i, bool a) {
    for (auto &x : requests) {
        if (i == x.id && x.status == "Pending") {
            if (a) {
                if (user.balance >= x.amount) {
                    MoneyTransfer(x.from,x.to,x.amount);
                    x.status = "Accepted";
                    saveRequests();
                    cout << "Money request accepted" << endl;
                }
                else {
                    cout << "Insufficient balance" << endl;
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
    cout << "Money request not found or already processed" << endl;
}
void RequestManager::showRequests(int userid) {
    cout << "Money requests by me:" << endl;

    for (auto &r : requests) {
        if (r.from == userid) {
            cout << r.id << " " << r.from << " "
                 << r.to << " " << r.amount << " "
                 << r.status << endl;
        }
    }
    cout<<"___________________________________________________"<<endl;
    cout << "Money requests sent to me:" << endl;

    for (auto &r : requests) {
        if (r.to == userid) {
            cout << r.id << " " << r.from << " "
                 << r.to << " " << r.amount << " "
                 << r.status << endl;
        }
    }
}

void RequestManager::cancelRequest(int id, int userId) {
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
    reqid = 1;

    int id, from, to, amount;
    string status;

    while (fin >> id >> from >> to >> amount >> status) {
        MoneyRequest r(id, from, to, amount);
        r.status = status;

        requests.push_back(r);

        if (id >= reqid) {
            reqid = id + 1;
        }
    }

    fin.close();
}