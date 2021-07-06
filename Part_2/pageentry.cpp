#include "pageentry.h"



PageEntry::PageEntry() {
    this->referenced = false;
    this->modified = false;
    this->protection = "XXX";
    this->present_absentBit = false;
    this->pageFrameNumber = -1;
}
        
PageEntry::PageEntry(const PageEntry &p) {
    this->referenced = p.referenced;
    this->modified = p.modified;
    this->protection = p.protection;
    this->present_absentBit = p.present_absentBit;
    this->pageFrameNumber = p.pageFrameNumber;
}

PageEntry& PageEntry::operator=(const PageEntry &p) {
    if(this != &p) {
        this->referenced = p.referenced;
        this->modified = p.modified;
        this->protection = p.protection;
        this->present_absentBit = p.present_absentBit;
        this->pageFrameNumber = p.pageFrameNumber;
    }
    return *this;
}

PageEntry::~PageEntry() {
    // destructor ...
}


bool operator>(const PageEntry &p1, const PageEntry &p2) {
    return (p1.pageFrameNumber > p2.pageFrameNumber);
}

bool operator<=(const PageEntry &p1, const PageEntry &p2) {
    return p1.pageFrameNumber <= p2.pageFrameNumber;
}

void PageEntry::setReferenced(bool x) {
    this->referenced = x;
}

void PageEntry::setModified(bool x) {
    this->modified = x;
}

void PageEntry::setProtection(string x) {
    this->protection = x;
}

void PageEntry::setPresentAbsentBit(bool x) {
    this->present_absentBit = x;
}

void PageEntry::setPageFrameNumber(int x) {
    this->pageFrameNumber = x;
}

void PageEntry::setRandomVal(int x) {
    this->randomVal = x;
}

void PageEntry::setIndex(int x) {
    this->index = x;
}


bool PageEntry::getReferenced() {
    return this->referenced;
}

bool PageEntry::getModified() {
    return this->modified;
}

string PageEntry::getProtection() {
    return this->protection;
}

bool PageEntry::getPresentAbsentBit() {
    return this->present_absentBit;
}

int PageEntry::getPageFrameNumber() {
    return this->pageFrameNumber;
}

int PageEntry::getIndex() {
    return this->index;
}

int PageEntry::getRandomVal() {
    return this->randomVal;
}