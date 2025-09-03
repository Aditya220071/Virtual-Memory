#include<bits/stdc++.h>
using namespace std;

#define printv(v) \
    for(auto vEleM: v) cout << vEleM << " "; cout << endl;
//program;
int M, V, P;
string inFile, outFile;
bool isFifo = false;
int globalPid = 1;

ofstream outFileTxt;
#define cout outFileTxt

enum class instructionType {ADD, SUB, PRINT, LOAD };
enum class commandType {LOAD, RUN, KILL, LISTPR, PTET, PTEALL, PRINT, EXIT};

void printTimeDate(ofstream& ofile) {
    time_t now = time(nullptr);
    tm *local_time = localtime(&now);
    char buffer[100];

    strftime(buffer, sizeof(buffer), "%a %b %d %I:%M:%S %p IST %Y", local_time);
    ofile << buffer << endl;
}

void printPQ(priority_queue<int, vector<int>, greater<int>> pq) {
    // while(!pq.empty()) {
    //     cout << pq.top() << " ";
    //     pq.pop();
    // }
    // cout << endl;
}


vector<string> parseLine(string line) {
    int i = 0;
    int maxSize = line.size();
    vector<string> words;
    string word;
    while(i < maxSize) {
        if(line[i] == ' ' || line[i] == ','|| line[i] == '\0' || line[i] == '\r'){
            if(word != "") words.push_back(word);
            word = "";
        }
        else
        word.push_back(line[i]);
        i++;
    }
    if(word != "") words.push_back(word);
    return words;
}


struct instClass {
    instructionType iType;
    int x = -1;
    int y = -1;
    int z = -1;
    int a = -1;
    instClass(instructionType iT, int x) : iType(iT), x(x) {};
    instClass(instructionType iT, int x, int y, int z): iType(iT), x(x), y(y), z(z) {};
    instClass(instructionType iT, int a, int y): iType(iT), a(a), y(y) {};
    void printInst() {
        // cout << (int)iType << " " ;
        // if(a != -1) cout << a << " ";
        // if(x != -1) cout << x << " ";
        // if(y != -1 ) cout << y << " ";
        // if (z != -1) cout << z << " " ;
        // cout << endl;
    } 
};


class PTE {
public:
    int processPN;
    bool isInMainMemory;
    vector<int> Values;
    int physicalOffset;

    PTE() {Values.resize(P);}
    PTE(int va, int pi, bool iimm) {
        Values.resize(P);
        processPN = va/P;
        physicalOffset = pi;
        isInMainMemory = iimm;
    }
    void printPTE(ofstream& ofile) {
        ofile << " " << physicalOffset << " " << isInMainMemory << endl;
    }

    void printPTE() {
        cout << " " << physicalOffset << " " << isInMainMemory << endl;
    }
};

class MMU {
public:
    int pteMM;
    int pteSM;
    vector<PTE> mainMemory;
    vector<PTE> swapMemory;
    priority_queue<int, vector<int>, greater<int>> idle_mM_PTE;
    priority_queue<int, vector<int>, greater<int>> idle_sM_PTE;
    queue<int> fifoMemory;
    list<int> lruMemory;
    MMU() {
        pteMM = M * (int)((float)1024/(float)P);
        pteSM = V * (int)((float)1024/(float)P);
        mainMemory.resize(pteMM);
        swapMemory.resize(pteSM);
        for(int i = 0; i < mainMemory.size(); i++) {
            idle_mM_PTE.push(i);
        }
        for(int i = 0; i < swapMemory.size(); i++) {
            idle_sM_PTE.push(i);
        }
    }

    void debug() {
        // cout << "Memory status \n";
        // printPQ(idle_mM_PTE);
        // printPQ(idle_sM_PTE);
        // cout << "Main Memory #PTE : "<< pteMM << endl;
        // cout << "Swap Memory #PTE : " << pteSM << endl;
        // cout<< "#################" << endl;
    }

    int replacementAlgo(PTE P) {
        if(isFifo) return fifoReplacement(P);
        else return lruReplacement(P);
    }

    int fifoReplacement(PTE P) {
        // cout << "Running Fifo\n";
        if(P.isInMainMemory) {
            return P.physicalOffset;
        }
        else {
            if(idle_mM_PTE.empty()) {
                int toReplace = fifoMemory.front();
                fifoMemory.pop();
                PTE pte = mainMemory[toReplace];
                pte.isInMainMemory = false;
                pte.physicalOffset = P.physicalOffset;
                P.isInMainMemory = true;
                P.physicalOffset = toReplace;
                mainMemory[toReplace] = P;
                swapMemory[pte.physicalOffset] = pte;
                fifoMemory.push(toReplace);
                return toReplace;
            }
            else {
                int idle_mmLoc = idle_mM_PTE.top();
                idle_mM_PTE.pop();
                idle_sM_PTE.push(P.physicalOffset);
                fifoMemory.push(idle_mmLoc);
                P.isInMainMemory = true;
                P.physicalOffset = idle_mmLoc;
                mainMemory[idle_mmLoc] = P;
                return idle_mmLoc;
            }
        }
    }
    // void LRU();

    int lruReplacement(PTE P) {
        // cout << "LRU: ";
        // for(auto it: lruMemory) {
        //     cout << it << " ";
        // }
        // cout << endl;
        // cout << "Running LRU\n";
        if(P.isInMainMemory) {
            lruMemory.remove(P.physicalOffset);
            lruMemory.push_back(P.physicalOffset);
            return P.physicalOffset;
        }
        else {
            if(idle_mM_PTE.empty()) {
                //
                int toReplace = lruMemory.front();
                lruMemory.pop_front();
                lruMemory.push_back(toReplace);
                //
                PTE pte = mainMemory[toReplace];    //MAIN MEMORY TO BE REPLCED
                //
                pte.isInMainMemory = false;
                pte.physicalOffset = P.physicalOffset;
                swapMemory[pte.physicalOffset] = pte; 
                //
                P.isInMainMemory = true;
                P.physicalOffset = toReplace;
                //
                mainMemory[toReplace] = P;
                //
                return toReplace;
            }
            else {
                int idle_mmLoc = idle_mM_PTE.top();
                idle_mM_PTE.pop();
                idle_sM_PTE.push(P.physicalOffset);
                lruMemory.push_back(idle_mmLoc);
                P.isInMainMemory = true;
                P.physicalOffset = idle_mmLoc;
                mainMemory[idle_mmLoc] = P;
                return idle_mmLoc;
            }
        }
    }
} Memory;

class executableFile {
public:
    int pSize; //in KB            noPTE = psize*(1024/P)
    int noPTE;
    int pid;
    vector<PTE> virtualMemory;
    vector<instClass> instructions;
    string fileName;
    executableFile() {}
    executableFile(string ifile) {
        // cout << ifile << endl;
        fileName = ifile;
        // cout << ifile << endl;
        ifstream inpFile(ifile);
        // if (!inpFile) {
        //     cout << ifile <<  " could not be loaded - file does not exist" << endl;
        //     return;
        // }
        string line;
        vector<string> words;
        getline(inpFile, line);
        pSize = stoi(line);
        while (getline(inpFile, line))
        {
            words = parseLine(line);
            string iT = words[0];
            words.erase(words.begin());
            if(iT == "add") {
                int x = stoi(words[0]);
                int y = stoi(words[1]);
                int z = stoi(words[2]);
                instructions.push_back(instClass(instructionType::ADD, x, y, z));
            }
            else if(iT == "sub") {
                int x = stoi(words[0]);
                int y = stoi(words[1]);
                int z = stoi(words[2]);
                instructions.push_back(instClass(instructionType::SUB, x, y, z));
            }
            else if(iT == "print") {
                int x = stoi(words[0]);
                instructions.push_back(instClass(instructionType::PRINT, x));
            }
            else if(iT == "load") {
                int a = stoi(words[0]);
                int y = stoi(words[1]);
                instructions.push_back(instClass(instructionType::LOAD, a, y));
            }
        }
        // pid = gloalPid++;
        noPTE = pSize* (float(1024)/float(P));
        virtualMemory.resize(noPTE);
        for(int i = 1; i <= noPTE; i++) {
            virtualMemory[i-1].processPN = i;
        }
        // cout << "Process has Virutual Memory of size " << virtualMemory.size() <<"KB" << endl;
        inpFile.close();
    }

    void printProcess() {
        // cout << "Printing Process\n";
        // cout << "pid: " << pid << endl;
        // cout << "Process Size: " << pSize << "KB\n";
        // cout << "Total pages: " << noPTE << endl; 
        // for(auto it: instructions) {
        //     it.printInst();
        // }
        // cout << "***********\n";
    }

    void runProcess() {
        // cout << "Running Process\n";
        for(auto inst: instructions) {
            // cout << "Printing Instruction\n";
            // inst.printInst();

            if(inst.iType == instructionType::ADD) {
                if(inst.x >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.x<<" specified for process id " << pid << endl; 
                    return;
                }
                if(inst.y >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.y<<" specified for process id " << pid << endl; 
                    return;
                }
                if(inst.z >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.z<<" specified for process id " << pid << endl; 
                    return;
                }
                // *z = *x + *y;
                auto& pte1 = virtualMemory[inst.x/P];
                auto& pte2 = virtualMemory[inst.y/P];
                auto& pte3 = virtualMemory[inst.z/P];
                int i, j, k;
                i = Memory.replacementAlgo(pte1);
                j = Memory.replacementAlgo(pte2);
                k = Memory.replacementAlgo(pte3);
                //use replacement algorithm to push it to main memory
                //assuming it in main memory;
                //assuming i, j, k as effective pte in mm;
                // cout << "Main Memory PTE: ";
                // cout << "i: " << i << " j : " << j << " k : " << k << endl;
                auto& physicalPTE1 = Memory.mainMemory[i];
                auto& physicalPTE2 = Memory.mainMemory[j];
                auto& physicalPTE3 = Memory.mainMemory[k];

                virtualMemory[inst.x/P] = Memory.mainMemory[i];
                virtualMemory[inst.y/P] = Memory.mainMemory[j];
                virtualMemory[inst.z/P] = Memory.mainMemory[k];
                physicalPTE3.Values[inst.z%P] = physicalPTE1.Values[inst.x%P] + physicalPTE2.Values[inst.y%P];
                cout << "Command: add "<< inst.x << ", "<< inst.y << ", "<< inst.z << "; Result: Value in addr "<< inst.x << " = " << physicalPTE1.Values[inst.x%P]<< ", addr "<< inst.y << " = " << physicalPTE2.Values[inst.y%P] << ", addr "<< inst.z << " = " << physicalPTE3.Values[inst.z%P] << endl;
            }
            else if(inst.iType == instructionType::SUB) {
                if(inst.x >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.x<<" specified for process id " << pid << endl; 
                    return;
                }
                if(inst.y >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.y<<" specified for process id " << pid << endl; 
                    return;
                }
                if(inst.z >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.z<<" specified for process id " << pid << endl; 
                    return;
                }
                // *z = *y - *x
                //Command: sub "<< inst.x << ", "<< inst.y << ", "<< inst.z << "; Result: Value in addr "<< inst.x << " = 100, addr "<< inst.y << " = 66, addr "<< inst.z << " = 34
                auto& pte1 = virtualMemory[inst.x/P];
                auto& pte2 = virtualMemory[inst.y/P];
                auto& pte3 = virtualMemory[inst.z/P];
                int i, j, k;
                i = Memory.replacementAlgo(pte1);
                j = Memory.replacementAlgo(pte2);
                k = Memory.replacementAlgo(pte3);
                //use replacement algorithm to push it to main memory
                //assuming it in main memory;
                //assuming i, j, k as their offsets after replacement;
                // i = pte1.physicalOffset;
                // j = pte2.physicalOffset;
                // k = pte3.physicalOffset;
                // cout << "Main Memory PTE: ";
                // cout << "i: " << i << " j : " << j << " k : " << k << endl;
                auto& physicalPTE1 = Memory.mainMemory[i];
                auto& physicalPTE2 = Memory.mainMemory[j];
                auto& physicalPTE3 = Memory.mainMemory[k];

                virtualMemory[inst.x/P] = Memory.mainMemory[i];
                virtualMemory[inst.y/P] = Memory.mainMemory[j];
                virtualMemory[inst.z/P] = Memory.mainMemory[k];
                physicalPTE3.Values[inst.z%P] = physicalPTE1.Values[inst.x%P] - physicalPTE2.Values[inst.y%P];
                cout << "Command: sub "<< inst.x << ", " << inst.y << ", "<< inst.z << "; Result: Value in addr "<< inst.x << " = " << physicalPTE1.Values[inst.x%P]<< ", addr "<< inst.y << " = " << physicalPTE2.Values[inst.y%P] << ", addr "<< inst.z << " = " << physicalPTE3.Values[inst.z%P] << endl;
            }
            else if(inst.iType == instructionType::PRINT) {
                if(inst.x >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.x<<" specified for process id " << pid << endl; 
                    return;
                }
                auto& pte1 = virtualMemory[inst.x/P];
                //replacement algo
                int i;
                // i = pte1.physicalOffset;
                // cout << "Printing PTE : " ;
                // pte1.printPTE();
                i = Memory.replacementAlgo(pte1);
                // cout << "Main Memory PTE: ";
                // cout << "i: " << i << " index: "; 
                // cout << inst.x%P <<endl;

                auto& physicalPTE1 = Memory.mainMemory[i]; 
                virtualMemory[inst.x/P] = Memory.mainMemory[i];

                // cout << "Printing PTE : " ;  
                // physicalPTE1.printPTE();         
                cout << "Command: print "<< inst.x << "; Result: Value in addr "<< inst.x << " = " << Memory.mainMemory[i].Values[inst.x%P] << endl;
            }
            else if(inst.iType == instructionType::LOAD) {
                if(inst.y >= 1024*pSize) {
                    cout << "Invalid Memory Address "<<inst.y<<" specified for process id " << pid << endl; 
                    return; 
                }
                // *y = a
                auto& pte1 = virtualMemory[inst.y/P];
                int i;
                i = Memory.replacementAlgo(pte1);
                cout << "Command: load " << inst.a << ", "<< inst.y <<"; Result: Value of " << inst.a << " is now stored in addr "<< inst.y << "" << endl;
                auto& physicalPTE1 = Memory.mainMemory[i];
                physicalPTE1.Values[inst.y%P] = inst.a;
                virtualMemory[inst.y/P] = Memory.mainMemory[i];
            }
            // cout << endl;
            // for(int j = 0; j < Memory.pteMM; j++) {
            // cout << ";Priting physical memory PTE " << j << endl; 
            //     for(int i = 0; i < P; i++) {
            //         if(Memory.mainMemory[j].Values[i] != 0) cout << i << " " << Memory.mainMemory[j].Values[i] << endl;
            //     }
            // }
        }
    }

    void killProcess();
};

map <int, executableFile> pidToProcess;

void loadInMMU(executableFile& exeFile) {
    Memory.debug();
    if(exeFile.noPTE > Memory.idle_mM_PTE.size() + Memory.idle_sM_PTE.size()) {
        cout << exeFile.fileName << " could not be loaded - memory is full\n";
        return;
    }//wrong
    exeFile.pid = globalPid;
    for(int i = 0; i < exeFile.noPTE; i++) {
        if(!Memory.idle_mM_PTE.empty()) {
            int idle_loc = Memory.idle_mM_PTE.top();
            Memory.idle_mM_PTE.pop();
            Memory.fifoMemory.push(idle_loc);
            Memory.lruMemory.push_back(idle_loc);
            exeFile.virtualMemory[i] = PTE(i, idle_loc, true); //check as passing by ref
            Memory.mainMemory[idle_loc] = exeFile.virtualMemory[i];
        }
        else if(!Memory.idle_sM_PTE.empty()) {
            int idle_loc = Memory.idle_sM_PTE.top();
            Memory.idle_sM_PTE.pop();
            exeFile.virtualMemory[i] = PTE(i, idle_loc, false); //check as passing by ref
            Memory.swapMemory[idle_loc] = exeFile.virtualMemory[i];                
        }
    }
    pidToProcess[globalPid] = exeFile;
    cout << exeFile.fileName << " is loaded and is assigned process id " << exeFile.pid << endl;
    globalPid++;
}

void executableFile::killProcess() {
    for(auto &pte: virtualMemory) {
        if(pte.isInMainMemory) {
            int mmpte = pte.physicalOffset;
            //doubt? do we need to nullify the vector
            Memory.idle_mM_PTE.push(mmpte);
        }
        else {
            int swpte = pte.physicalOffset;
            Memory.idle_sM_PTE.push(swpte);
        }
    }
}

class command {
public:
    commandType cType;
    vector<string> files;                             //for load
    int pid = -1;                                        //for run and kill, pte
    int memloc = -1;                                     //for print
    int length = -1;                                     //for print
    string outFile = "";                                 //for pte and pteall
    command(commandType cT, vector<string> fN): cType(cT), files(fN) {};
    command(commandType cT, string s): cType(cT), outFile(s) {};
    command(commandType cT, int pid): cType(cT), pid(pid) {};
    command(commandType cT): cType(cT) {};
    command(commandType cT, int pid, string fN): cType(cT), pid(pid), outFile(fN) {};
    command(commandType cT, int ml, int l): cType(cT), memloc(ml), length(l) {};

    void printCommand() {
        if(files.size() > 0) {
            cout << "Files: ";
            printv(files);
        }
        if(pid != -1) cout << "pid: " << pid << endl;
        if(memloc != -1) cout << "memloc: " << memloc << endl;
        if(length != -1) cout << "length: " << length << endl;
        if(outFile != "") cout << "outFile: " << outFile << endl;
    }

    void runCommand() {
        if(cType == commandType::LOAD) {
            for(auto file : files) {
                // cout << "file name " << file << endl;
                ifstream inpFile(file);
                if (!inpFile) {
                    cout << file <<  " could not be loaded - file does not exist" << endl;
                    continue;
                }
                executableFile exeFile(file);
                loadInMMU(exeFile);
                // exeFile.printProcess();
                // cout << "file name2 " << file << endl;
            }
            Memory.debug();
        }

        else if(cType == commandType::RUN) {
            //it should be in main memory if not use replacement algo 
            if(pidToProcess.find(pid) == pidToProcess.end()) {
                cout << "Invalid PID "<< pid << "\n";
                return;
            }
            auto& Process = pidToProcess[pid];
            // Process.printProcess();
            Process.runProcess();
        }
        else if(cType == commandType::KILL) {
            // cout << "Command: Kill "<< pid << endl;
            if(pidToProcess.find(pid) == pidToProcess.end()) {
                cout << "Invalid PID "<< pid << "\n";
                return;
            }
            executableFile Process;
            Process = pidToProcess[pid];
            //doubt? do we have to replace it to mm
            pidToProcess.erase(pid);
            Process.killProcess();
            cout << "killed " << pid << "\n";
            // Memory.debug();
        }
        else if(cType == commandType::LISTPR) {
            for(auto pids: pidToProcess) {
                cout << pids.first << " " ;
            }
            cout << endl;
        }
        else if(cType == commandType::PTET) {
            if(pidToProcess.find(pid) == pidToProcess.end()) {
                cout << "Invalid PID "<<pid<<"\n";
                return;
            }
            ofstream ofile(outFile);
            if(ofile.is_open()) printTimeDate(ofile);
            //if the file already exists, append the output to the file
            auto& exeFile = pidToProcess[pid];
            int i = 0;
            for(auto ptes: exeFile.virtualMemory) {
                ofile << i;
                ptes.printPTE(ofile);
                i++;
            }
            // ofile.close();
        }
        else if(cType == commandType::PTEALL) {
            ofstream ofile(outFile);
            if(ofile.is_open()) printTimeDate(ofile);
            //if the file already exists, append the output to the file
            for(auto& prcs: pidToProcess) {
                int i = 0;
                for(auto ptes: prcs.second.virtualMemory) {
                    ofile << prcs.first << " ";
                    ofile << i;
                    ptes.printPTE(ofile);
                    i++;
                }
            }
            // ofile.close();
        }
        else if(cType == commandType::PRINT) {
            int l = memloc;
            int r = memloc + length -1;
            int t = r - l + 1;
            for(int i = 0; i < t; i++) {
                int pteN = (l + i)/P;
                int offset = (l + i)%P;
                cout << "Value of " << memloc + i << ": " << Memory.mainMemory[pteN].Values[offset] << endl;
            }
        }
        else if(cType == commandType::EXIT) {
            ofstream(outFile).close();
        }
    }
};

// void MMU::FIFO() {

// }

// void MMU::LRU() {
    
// }

class systemCommands {
public:
//M, V, P
    vector<command> sysCommands;
    
    systemCommands() {}
    systemCommands(vector<command> cm) : sysCommands(cm) {}

    void runProgram() {
        globalPid = 1;
        for(auto cmd: sysCommands) {
            cmd.runCommand();
        }
    }
} Program;

void args(int argc, char** argv) {
    int c;
    while ((c = getopt (argc, argv, "fM:V:P:i:o:")) != -1) {
        switch(c)
        {
            case 'f':
                isFifo = true;
                break;
            case 'M':
                M = stoi(optarg);
                break;
            case 'V':
                V = stoi(optarg);
                break;
            case 'P':
                P = stoi(optarg);
                break;
            case 'i':
                inFile = optarg;
                break;
            case 'o':
                outFile = optarg;
                break;
             default:
                cerr << "Unknown option: " << c << "\n";
                break;
        }
    }
}

vector<command> parseSysCommand(ifstream& fin) {
    vector<string> words;
    string line;
    vector<command> cmd;
    while (getline(fin, line)) {
        words = parseLine(line);
        string cT = words[0];
        words.erase(words.begin());
        if(cT== "load") {
            cmd.push_back(command(commandType::LOAD, words));
        }
        else if(cT== "run") {
            cmd.push_back(command(commandType::RUN, stoi(words[0])));
        }
        else if(cT== "kill") {
            cmd.push_back(command(commandType::KILL, stoi(words[0])));
        }
        else if(cT== "listpr") {
            cmd.push_back(command(commandType::LISTPR));
        }
        else if(cT== "pte") {
            cmd.push_back(command(commandType::PTET, stoi(words[0]), words[1]));
        }
        else if(cT== "pteall") {
            cmd.push_back(command(commandType::PTEALL, words[0]));
        }
        else if(cT== "print") {
            cmd.push_back(command(commandType::PRINT, stoi(words[0]), words[1]));
        }
        else if(cT== "exit") {
            cmd.push_back(command(commandType::EXIT));
        }
        else {
            cerr << "Unknown command\n";
            exit(1);
        }
    }
    return cmd;
}

void printCmd() {
    for(auto elem: Program.sysCommands)
    {cout << (int) elem.cType << " "; elem.printCommand(); }
}



int main(int argc, char** argv) {
    args(argc, argv);
    outFileTxt.open(outFile);
    isFifo = true;
    // cout << "M = " << M << "\nV = " << V << "\nP = " << P << endl;
    Memory = MMU();
    ifstream fin(inFile);
    Program = systemCommands(parseSysCommand(fin));
    // printCmd();
    Program.runProgram();
    fin.close();
}