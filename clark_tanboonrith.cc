#include<iostream>
#include<vector>
#include<queue>
#include<fstream>
#include<algorithm>
#include<cmath>
#include<string>

using namespace std;

//struct that holds the information of each process
struct process {
    int pid;
    int arrival_time;
    int burst_time;
    int burst_time2;
    int complete;
    int wait_time;
    int turnaround_time;
    int age = 0;
    int timeRan = 0;
};
//struct that holds a deque of pointers to process objects and queue information
struct que {
    int time_quantum;
    deque<process*> q;
    int id;
};
//function to sort that processes that are stored within a vector (last arrival -> earliest arrival)
void sort_arrival(vector<process>& v){
    sort(v.begin(), v.end(), [](process a, process b){
        return a.arrival_time > b.arrival_time;
    });
}
//function to filter out illegal processes (burst less than or 0, and arrival less than 0)
void filter(vector<process>& v){
    for(int i = 0; i < v.size(); i++){
        if(v[i].arrival_time < 0){
            v.erase(v.begin() + i);
        }
        if(v[i].burst_time <= 0){
            v.erase(v.begin() + i);
        }
        if(v[i].arrival_time == -1){
            v.erase(v.begin() + i);
        }
    }
}

int main(){
    //variables
    string filename; //file input string
    int num_queues; //number of queues input
    int time_q; //time quantum input
    int age_interval; //age interval input
    float ave_wait_time = 0; //average wait
    float ave_turnaround_time = 0; //average turnaround
    
    //prompt user for filename
    cout << "Enter file name: ";
    cin >> filename; //store

    // retrieving rest of user input
    cout << "Running MFQS ..... \n";
    cout <<"\n";
    // user inputs number of queues in the range 2 to 5
    while(num_queues < 2 || num_queues > 5){
        cout << "Enter number of queues: ";
        cin >> num_queues; //stored
        if(num_queues < 2 || num_queues > 5){
            cout << "Range of queues (2 <= x <= 5) Re-enter..." << endl;
        }
    }
    //user inputs time quantum for the top queue (greater than 0)
    while(time_q < 1){
        cout << "Enter time quantum for top queue: ";
        cin >> time_q;
        if(time_q < 1){
            cout << "Enter number greater than 0..." << endl;
        }
    }
    //user inputs age interval(greater than 0)
    while(age_interval < 1){
        cout << "Enter ageing interval: ";
        cin >> age_interval;
        if(age_interval < 1){
            cout << "Enter number greater than 0..." << endl;
        }
    }

    cout <<"\n";
    
    //vector of process struct holding information about all of the process coming in 
    vector<process> processes;
    //reading a file and then storing the information into process struct then into vector of processes
    int counter = 0;
    string nothing;
    ifstream file (filename + ".txt");
    int a,b;
    if(file.is_open()) {
        getline(file, nothing);
        int pid, burst, arrival;
        while(file >> pid >> burst >> arrival >> a >> b){
            processes.push_back(process());
            processes[counter].pid = pid; //store the pid
            processes[counter].burst_time = burst; //store the burst
            processes[counter].burst_time2 = burst; //store burst again
            processes[counter].arrival_time = arrival; //store arrival time
            counter++;
        }
        file.close();   
    }else{
        cout<< "Unable to open file\n"; //error
    }

    //pass vector into the filter and sort
    filter(processes);
    sort_arrival(processes);
    
    //intializes queues with the time quantum based on the users parameters 
    int count = 0;
    vector<que> queues; //vector of que object called queues
    while(count < num_queues-1){
        queues.push_back(que());
        queues[count].time_quantum = time_q;
        time_q = time_q * 2;
        count++;
    }
    //initializes the last queue and assigns it an id of 5
    queues.push_back(que());
    queues.back().id = 5;
    
    
    int clock = 0; //clock
    bool procRunning; //check if process is running
    int current = 0; // index of queue 
    int number_of_processes = 0; //track processes completed
    int p = processes.size(); //store the size of processes into p
    
    //while loop that exits once the number of processes completed is equal to the size of processes vector
    while(number_of_processes < p) {
        /* when the clock is equal to the arrival time push into the first queue
           print out when process arrives, pop the back off the vector 
           then break the loop when size is zero or when clock does not equal arrival time */
        
        if(processes.size() > 0){
            while(clock == processes.back().arrival_time){
                queues.front().q.emplace_back(&processes.back());
                cout << "Process " << processes.back().pid << ": Arrives @\t" << clock << "\n";
                processes.pop_back();
                if(processes.size() == 0){
                    break;
                }
            } 
        }

        /* ageing algorithm, if the current queue is not the last one (id = 5) then  
           increment the age of each item in the bottom queue by one, also checks if 
           age his reached the ageing interval, if so push it to the queue above and pop */

        if(queues[current].id != 5){
            for(int i = 0; i < queues.back().q.size(); i++){
                if(queues.back().q.front()->age < age_interval){
                    queues.back().q.front()->age++;
                    //cout << queues.back().q.front()->pid << " " << queues.back().q.front()->age <<  endl;
                    queues.back().q.emplace_back(queues.back().q.front());
                    queues.back().q.pop_front();
                }
                if(queues.back().q.front()->age == age_interval){
                    queues[num_queues-2].q.emplace_back(queues.back().q.front());
                    queues.back().q.pop_front();
                }
            }
        }
        /* method to decrement the burst time of the process that is currently running,
           checks to see if procRunning is true and we are not in the last queue, also makes sure
           the process has not ran for longer than it should have. (else if for if last queue is current queue)
        */
        if(procRunning == true && queues[current].id != 5 && queues[current].q.front()->timeRan < queues[current].time_quantum){
            queues[current].q.front()->burst_time--;
            queues[current].q.front()->timeRan++;
        }else if(queues[current].id == 5 && procRunning == true){
            queues[current].q.front()->burst_time--;
        } 

        /*method to check what the current queue we should be running processes in,
          loop through the number of queues and find the first one that is not empty
          then check to see if we interrupted a process that was ready to finish or switch.
          set the current to j and break.
        */

        for(int j = 0; j<num_queues; j++){
            if(!queues[j].q.empty()){
                if(j == (current - 1)){
                    if(queues[current].q.front()->burst_time == 0){
                        cout << "Process " << (queues[current].q.front()->pid) << ": Finished @\t" << clock << "\n";
                        ave_turnaround_time = ave_turnaround_time + (clock - queues[current].q.front()->arrival_time);
                        ave_wait_time = ave_wait_time + ((clock - queues[current].q.front()->arrival_time) - queues[current].q.front()->burst_time2);
                        number_of_processes++;
                        queues[current].q.pop_front();
                    } else if(queues[current].q.front()->timeRan == queues[current].time_quantum && queues[current].id != 5){
                        cout << "Process " << queues[current].q.front()->pid << ": Switched @\t" << clock << "\n";
                        queues[current].q.front()->timeRan = 0;
                        queues[current+1].q.emplace_back(queues[current].q.front()); 
                        queues[current].q.pop_front();
                    }
                    procRunning = false;
                    current = j;
                    break;
                } else {
                    current = j;
                    break;
                }
            }
        }
        
        //checks is the current queue is empty and a process isn't running
        //if true then print process runs at the current clock
        //set procRunning equal to true

        if(queues[current].q.size() > 0 && procRunning == false){
            cout << "Process " << queues[current].q.front()->pid << ": Runs @\t" << clock <<"\n";
            procRunning = true;
        }
        
        //if a process is running we then have to check if it has finished or switched
        //finished if burst time has hit zero, switched if timran equals the current time quantum
        //after a process has finished or switched, then check to see if another process is ready to run

        if(procRunning){
            if(queues[current].q.front()->burst_time == 0) {
                    cout << "Process " << (queues[current].q.front()->pid) << ": Finished @\t" << clock << "\n";
                    //when finished must calculate the turnaround and wait time
                    ave_turnaround_time = ave_turnaround_time + (clock - queues[current].q.front()->arrival_time); 
                    ave_wait_time = ave_wait_time + ((clock - queues[current].q.front()->arrival_time) - queues[current].q.front()->burst_time2);
                    queues[current].q.pop_front(); // pop it
                    number_of_processes++; // increment total processes
                    procRunning = false; //process not running anymore
                    
                    if(!queues[current].q.empty() || !queues[current+1].q.empty()){
                        if(!queues[current].q.empty()){
                            cout << "Process " << queues[current].q.front()->pid << ": Runs @\t" << clock << "\n";
                            procRunning = true;
                        } else if(queues[current].q.empty()){
                            cout << "Process " << queues[current+1].q.front()->pid << ": Runs @\t" << clock << "\n";
                            procRunning = true;
                            current = current + 1;
                        }
                }
            } else if(queues[current].q.front()->timeRan == queues[current].time_quantum && queues[current].id !=5 && procRunning == true) {
                cout << "Process " << queues[current].q.front()->pid << ": Switched @\t" << clock << "\n";
                queues[current].q.front()->timeRan = 0; //set time ran to zero when switched
                queues[current+1].q.emplace_back(queues[current].q.front());  // push to next queue
                queues[current].q.pop_front(); //pop
                procRunning = false; //proc is not running anymore
                if(!queues[current].q.empty() || !queues[current+1].q.empty()){
                    if(!queues[current].q.empty()){
                        cout << "Process " << queues[current].q.front()->pid << ": Runs @\t" << clock << "\n";
                        procRunning = true;
                    } else if(queues[current].q.empty()){
                        cout << "Process " << queues[current+1].q.front()->pid << ": Runs @\t" << clock << "\n";
                        procRunning = true;
                        current = current + 1;
                    }
                }
            }
        }
        //increment the clock
        clock++;
    }

    ave_turnaround_time = ave_turnaround_time/(float)number_of_processes; // get the average
    ave_wait_time = ave_wait_time/(float)number_of_processes; // get the average 
    //print out results
    cout << "Ave. wait time = " << ave_wait_time << "\t" << "Ave. turnaround time = " << ave_turnaround_time << "\n";
    cout << "Total Processes Scheduled = " << number_of_processes <<  "\n";

    return 0;
}