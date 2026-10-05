#ifndef MONEYREQUEST_H
#define MONEYREQUEST_H

#include <string>
#include <vector>
using namespace std;

class User;

class MoneyRequest {
public:
    string id, from, to;
    double amount;
    string status = "Pending";

    MoneyRequest(string id, string from, string to, double amount);
};

class RequestManager {
    vector<MoneyRequest> requests;
    int reqCounter = 1;

    bool validId(const string& id);

public:
    void requestMoney(string from, string to, double amount, vector<User>& users);
    void respond(string id, string walletId, bool accept, vector<User>& users);
    void showRequests(string walletId);
    void cancelRequest(string id, string walletId);
    bool MoneyTransfer(string from, string to, double amount, vector<User>& users);
    bool saveRequests();
    void loadRequests();
};

#endif
