#ifndef _PAGETABLEENTRY_H
#define _PAGETABLEENTRY_H

#include <iostream>
#include <string>

using namespace std;

class PageEntry {

    public:
        PageEntry();
        
        PageEntry(const PageEntry &p);
        PageEntry& operator=(const PageEntry &p);
        ~PageEntry();

        friend bool operator>(const PageEntry &p1, const PageEntry &p2);
        friend bool operator<=(const PageEntry &p1, const PageEntry &p2);

        void setReferenced(bool x);
        void setModified(bool x);
        void setProtection(string x);
        void setPresentAbsentBit(bool x);
        void setPageFrameNumber(int x);
        void setIndex(int x);
        void setRandomVal(int x);

        bool getReferenced();
        bool getModified();
        string getProtection();
        bool getPresentAbsentBit();
        int getPageFrameNumber();
        int getIndex();
        int getRandomVal();

    private:
        int index;
        bool referenced;
        bool modified;
        string protection;
        bool present_absentBit;
        int pageFrameNumber;
        int randomVal;

};

#endif // !_PAGETABLEENTRY_H