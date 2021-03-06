#ifdef CHANGED
#include "pageprovider.h"
#include "machine.h"
#include "system.h"

PageProvider::PageProvider() {
    bitmap = new BitMap(NumPhysPages);
}

PageProvider::~PageProvider() {
    delete bitmap;
}

int PageProvider::GetEmptyPage() {
    int empty_page = bitmap->Find();
    ASSERT(empty_page != -1);
    int addr = (empty_page * PageSize);
    void *ptr = &(machine->mainMemory[addr]);
    memset(ptr, 0, PageSize);
    return addr;
}

void PageProvider::ReleasePage(int addr) {
    int page = addr / PageSize;
    bitmap->Clear(page);
}

int PageProvider::NumAvailPage() {
    return bitmap->NumClear();
}

#endif