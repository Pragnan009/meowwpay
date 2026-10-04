#ifndef MONEYREQUEST_H
#define MONEYREQUEST_H

#include <string>
#include <vector>
using namespace std;

class MoneyRequest{
    public:
    int id,from,to,amount;
    string status="Pending";
    MoneyRequest(int id,int from,int to, int amount);
};
class RequestManager{
    
    vector<MoneyRequest> requests;
    int reqid=1;
    public:
    void requestMoney(int from, int to, int amount);
    void respond(int id, bool accept);
    void showRequests(int userId);
    void cancelRequest(int id,int from);
    void saveRequests();
    void loadRequests();
};

#endif