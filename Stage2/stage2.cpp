#include <iostream>
#include <mutex>
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <string>
#include <ncurses.h>


const int NrOfPhil = 7;
std::mutex fork_mutex[NrOfPhil];
std::thread philosphers[NrOfPhil];
std::mutex cout_mutex;
std::mutex mutex;

bool dinner_end = false;


enum PhilosopherStatus {
        thinking,
        hungry,
        eating,
        waiting,
        left_fork,
        both_fork
};


struct Philosopher{


    int id = -1;
    int cycle = 0;
    PhilosopherStatus status = PhilosopherStatus::thinking;

    std::string getState()
    {

        std::string msg;
        switch(status) {
            case PhilosopherStatus::thinking:
                msg="Mysli       |";
                break;
            case PhilosopherStatus::hungry:
                msg="Jest glodny |";
                break;
            case PhilosopherStatus::eating:
                msg="Je          |";
                break;
            case PhilosopherStatus::waiting:
                msg="Czeka       |";
                break;
            case PhilosopherStatus::left_fork:
                msg="Ma lewy widelec |";
                break;
            case PhilosopherStatus::both_fork:
                msg="Ma oba widelce  |";
                break;
        }
        return msg;
    }


};


Philosopher philos[NrOfPhil];


void Visualization()
{
    for (int i = 0; i < NrOfPhil; i++)
    {
        mutex.lock();
        move(i, 0);
        printw(" Filozof nr: %d | cykl nr: %d | Stan: %s", i+1, philos[i].cycle, philos[i].getState().c_str());
        refresh();
        move(i, 55);
        clrtoeol();
        printw("Widelec[%d] jest: wolny", i+1);
        refresh();
        mutex.unlock();
    }
}


void ForkRefresh(int id, std::string _string)
{
    mutex.lock();
    move(id, 55);
    clrtoeol();
    printw("Widelec[%d] jest: %s", id+1, _string.c_str());
    refresh();
    mutex.unlock();
}


void PhilosopherRefresh(int id, Philosopher p)
{
    mutex.lock();
    move(id, 0);
    printw(" Filozof nr: %d  | cykl nr: %d  | Stan: %s", id+1, p.cycle, p.getState().c_str());
    refresh();
    mutex.unlock();
}


void log(std::string str){
    cout_mutex.lock();
    std::cout << str << std::endl;
    cout_mutex.unlock();
}


void think(const int& id){

    philos[id].status = thinking;
    PhilosopherRefresh(id, philos[id]);
    std::this_thread::sleep_for(std::chrono::milliseconds(2500+(std::rand() % 1000)));
    philos[id].status = hungry;
    PhilosopherRefresh(id, philos[id]);
}


void eat(const int& id)
{
    philos[id].status = eating;
    PhilosopherRefresh(id, philos[id]);
    std::this_thread::sleep_for(std::chrono::milliseconds(2500+(std::rand() % 1000)));
}


bool eat(int id, int left,int right){               // overloaded

    while (1) if (fork_mutex[left].try_lock())
    {
        philos[id].status = left_fork;
        ForkRefresh(left, "zajety");
       // std::this_thread::sleep_for(std::chrono::milliseconds(1500+(std::rand() % 1000)));  // niepotrzebne

        if (fork_mutex[right].try_lock())
        {
            //log("Philosopher " + std::to_string(id) + " got right fork");
            philos[id].status = both_fork;
            ForkRefresh(right, "zajety");

          //  std::this_thread::sleep_for(std::chrono::milliseconds(1500+(std::rand() % 1000)));  // niepotrzebne
            eat(id);
            return true;

        } else {

           fork_mutex[left].unlock();
           ForkRefresh(left, "wolny");

           philos[id].status = waiting;
           PhilosopherRefresh(id, philos[id]);
        }
    }
}


void putDownForks(int left, int right) {
    fork_mutex[left].unlock();
    ForkRefresh(left, "wolny");
    fork_mutex[right].unlock();
    ForkRefresh(right, "wolny");
}


void dine(int id) {
    int left = std::min(id, (id + 1) % (NrOfPhil));
    int right = std::max(id, (id + 1) % (NrOfPhil));

    while (!dinner_end){
        if (eat(id, left, right)) { // Philosopher "philID" is trying to eat, w/ forks; lIndex & rIndex
            putDownForks(left, right); // Put down the forks;
            philos[id].status = waiting;
            PhilosopherRefresh(id, philos[id]);
            philos[id].cycle++;
            think(id);
        }
    }
}


void start(){

    for (int i = 0; i < NrOfPhil; ++i) philosphers[i] = std::thread(dine, i);


    for (int i = 0; i < NrOfPhil; ++i) philosphers[i].join();
}


int main()
{
    initscr();
    srand(time(NULL));

    for(int i=0; i<NrOfPhil; ++i)
    {
        philos[i].id = i;

    }

    cbreak();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));

    Visualization();
    for (int i = 0; i < NrOfPhil; ++i) philosphers[i] = std::thread(dine, i);

    char tmp = 'p';

    //czekanie na sygnal zakonczenia pracy
    while(tmp != 'q')
    {
        tmp = (char)(std::cin.get());
        if (tmp == 'q')
        {
            dinner_end = true;
        }
    }


    for (int i = 0; i < NrOfPhil; ++i) philosphers[i].join();


    endwin();
    exit(0);
}
