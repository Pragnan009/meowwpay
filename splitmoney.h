#ifndef SPLITMONEY_H
#define SPLITMONEY_H
#include <string>
#include <vector>
using namespace std;
class User;
struct SplitShare { string walletId; int amount; bool paid; };
struct SplitExpense { string id, payerWalletId; int totalAmount; vector<SplitShare> shares; };
class SplitMoney {
    vector<SplitExpense> expenses;
    int splitCounter = 1;
    bool validId(const string& id);
    bool walletExists(const string& walletId, vector<User>& users);
public:
    void createEqual(string payerWalletId, vector<string> participantWalletIds, int amount, vector<User>& users);
    void createCustom(string payerWalletId, vector<string> participantWalletIds, vector<int> amounts, int totalAmount, vector<User>& users);
    bool settleShare(string splitId, string walletId, vector<User>& users);
    void showSplits(string walletId);
    bool saveSplits();
    void loadSplits();
};
#endif
