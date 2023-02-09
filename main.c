#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#define MAX_SIZE 1024

typedef struct {
    unsigned    pid;
    double      mem_usage;
    TCHAR*       name;
} process;

void getMemUsage(process *const p) {
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,p->pid);
    if (!hProcess) {
        return;
    }
    double mem_usage;
    if (GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
        mem_usage =  (pmc.WorkingSetSize + pmc.QuotaNonPagedPoolUsage + pmc.QuotaPagedPoolUsage) / 1000000.;
    }

    CloseHandle( hProcess);
    p->mem_usage = mem_usage;
}



void getName(process *p) {
    TCHAR* processName = malloc(sizeof(TCHAR) * MAX_SIZE);
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, p->pid );


    if (hProcess != NULL){
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),&cbNeeded) ) {
            GetModuleBaseName( hProcess, hMod, processName,sizeof(processName) * 1024 / sizeof(TCHAR) );
        }
    }

    CloseHandle(hProcess);
    p->name = processName;
}

int main() {
    while (1) {

        DWORD pids[MAX_SIZE], cbNeeded, amount;

        if (!EnumProcesses(pids, sizeof(pids), &cbNeeded)) {
            return 1;
        }

        amount = cbNeeded / sizeof(DWORD);
        process processes[MAX_SIZE];
        int head = 0;
        for (unsigned i = 0; i < amount; ++i) {
            if (pids[i] != 0) {
                process p = (process) {.pid = pids[i]};
                getMemUsage(&p);
                getName(&p);
                if (p.mem_usage != 0) processes[head++] = p;
            }
        }

        //Bubble sort💪🤡
        for (int i = 0; i < head; ++i) {
            for (int j = i; j < head; ++j) {
                if (processes[j].mem_usage > processes[i].mem_usage) {
                    process temp = processes[i];
                    processes[i] = processes[j];
                    processes[j] = temp;
                }
            }
        }
        for (int i = 0; i < head; ++i) {
            printf("%d %s %lf\n", processes[i].pid, processes[i].name, processes[i].mem_usage);
            free(processes[i].name);
        }
        Sleep(1000);
        system("cls");
    }
}