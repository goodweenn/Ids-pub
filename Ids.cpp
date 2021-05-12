﻿// Ids.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <mutex>

using namespace std;

#define ALPHABET 'A','B','C','E','H','I','K','L','N','O','P','R','S','T','U','W','X','Y','Z'
#define NO_PAIR -1
#define DELIMETER '-'
#define PAIR_SIZE 2


class IDs
{
    vector<pair<char, unsigned int>> Id;

    inline static const vector<char> AllowedSymbols = { ALPHABET };

    mutex Mutex;

    bool    Assign(string _Id);
    string  ComposeString();
    bool    Increment(string& result, int TargetPair);

public:

    string  Increment()
    {
        string Tmp;

        Mutex.lock();
        bool res = Increment(Tmp, NO_PAIR);
        Mutex.unlock();

        if (!res) throw std::out_of_range("Upper bound is violated");
        return Tmp;
    }

    string  Get()
    { 
        Mutex.lock();
        string tmp = ComposeString();
        Mutex.unlock();

        return tmp;
    }

    void Set(string _Id)
    {
        Mutex.lock();

        Id.clear();
        bool res = Assign(_Id);

        Mutex.unlock();

        if (!res) throw std::out_of_range("Incorrect Value");
    }

    // Constructors
    IDs(string _Id)
    {
        if (!Assign(_Id)) throw std::out_of_range("Incorrect Value");;
    }

    IDs() {}

};


bool IDs::Assign(string _Id)
{
    if (_Id.empty() || _Id.size() > 29) return false;

    string token;
    istringstream tokenStream(_Id);

    while (getline(tokenStream, token, DELIMETER))
    {
        if (token.size() != PAIR_SIZE || !isalpha(token[0]) || !isdigit(token[1])) return false;

        char _first = toupper(token[0]);
        if (find(begin(AllowedSymbols), end(AllowedSymbols), _first) == end(AllowedSymbols)) return false;

        unsigned int _second = atoi(token.data()+1);
        if (!_second) return false;

        Id.push_back(make_pair(_first, _second));
    }

    return true;
}

string IDs::ComposeString()
{
    string Res = "";

    for (auto Pair : Id)
    {
        Res += Pair.first;
        Res += to_string(Pair.second);
        Res += DELIMETER;
    }

    Res.erase(Res.size() - 1);

    return Res;
}

bool IDs::Increment(string &result, int TargetPair)
{
    size_t CurrentPair = (TargetPair == NO_PAIR) ? Id.size() - 1 : TargetPair;

    Id[CurrentPair].second++;

    if (Id[CurrentPair].second == 10)
    {
        Id[CurrentPair].second = 1;
        auto CurLetter = find(begin(AllowedSymbols), end(AllowedSymbols), Id[CurrentPair].first);

        if (*CurLetter != AllowedSymbols[AllowedSymbols.size()-1])
        {
            Id[CurrentPair].first = *(CurLetter + 1);
        }
        else 
        {
            if (CurrentPair == 0)
             {
                if (Id.size() == 10)  return false;

                // Create new pair
                pair<char, unsigned int> NewPair;
                NewPair.first = AllowedSymbols[0];
                NewPair.second = 1;

                Id[CurrentPair] = NewPair;
                Id.push_back(NewPair); 
            }
            else
            {
                // Propagatre the increment
                Id[CurrentPair].first = AllowedSymbols[0];
                Id[CurrentPair].second = 1;

                return Increment(result, --CurrentPair);
            }
        }
    }

    result = ComposeString();
    
    return true;
}


//
// Some tests
//


void Test(string sID)
{
    static size_t TestNumber = 1;

    cout << "Test " << TestNumber << endl;
    IDs Id(sID);
    cout << "Inital ID:\t" << sID << endl << "Incremented ID:\t" << Id.Increment() << endl << endl;
    TestNumber++;
}

IDs CommonID;

void ThreadTest()
{
    CommonID.Increment();
}


int main()
{
    Test("A1");
    Test("A9");
    Test("Z9");
    Test("C9-Z9");
    Test("Z9-Z9");
    Test("Z1-Z9-Z9-Z9-Z9-Z9-Z9-Z9-Z9-Z9");

    // Threads
    CommonID.Set("Z9");

    thread Th1(ThreadTest);
    thread Th2(ThreadTest);
    thread Th3(ThreadTest);

    Th1.join();
    Th2.join();
    Th3.join();

    cout << "Thread test (Initial is 'Z9'): " << CommonID.Get() << endl << endl;

    // Exceptions
    try
    {
        Test("Z9-Z9-Z9-Z9-Z9-Z9-Z9-Z9-Z9-Z9");
    }
    catch (const std::exception& ex)
    {
        cout << ex.what() << endl;
    }

    try
    {
        IDs Test;
        Test.Set("D1-A0");
    }
    catch (const std::exception& ex)
    {
        cout << endl<< "Test of an approach to set incorrect ID:" << endl << ex.what() << endl;
    }


    return 0;

}
