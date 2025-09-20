#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int M;
int N;
int currentPlayer = 0;

void handleSignal(int sig) {
    if ((sig == SIGINT && currentPlayer == 0) || (sig == SIGQUIT && currentPlayer == 1)) {
        int take;
        if (currentPlayer == 0) {
            cout << "\nPlayer A, enter number of matches (1 - " << M << "): ";
        } else {
            cout << "\nPlayer B, enter number of matches (1 - " << M << "): ";
        }
        cin >> take;

        if (take < 1 || take > M) {
            cout << "Invalid input. Try again." << endl;
        } else {
            N -= take;
            if (N <= 0) {
                if (currentPlayer == 0) {
                    cout << "\nWinner: Player A!" << endl;
                } else {
                    cout << "\nWinner: Player B!" << endl;
                }
                exit(0);
            }
            currentPlayer = 1 - currentPlayer;
            if (currentPlayer == 0) {
                cout << "\nMatches remaining on the table: " << N << ". Next up: Player A." << endl;
            } else {
                cout << "\nMatches remaining on the table: " << N << ". Next up: Player B." << endl;
            }
        }
    }
}

void terminateHandler(int) {
    cout << "\nProgram terminating..." << endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    do {
        cout << "Enter M (greater than 2): ";
        cin >> M;
        if (M <= 2) {
            cout << "Must be greater than 2. Try again." << endl;
        }
    } while (M <= 2);

    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <number of matches N>" << endl;
        return 1;
    }

    N = atoi(argv[1]);
    if (N <= M) {
        cout << "N must be greater than " << M << endl;
        return 1;
    }

    cout << "M = " << M << ". N = " << N << "." << endl;
    cout << "Parameters are valid. Starting the game." << endl;
    cout << "Matches on the table: " << N << ". Next up: Player A." << endl;

    signal(SIGINT, handleSignal);   // SIGINT for Player A (CTRL+C)
    #ifdef SIGQUIT
    signal(SIGQUIT, handleSignal);  // SIGQUIT for Player B (CTRL+\)
    #endif
    #ifdef SIGTSTP
    signal(SIGTSTP, terminateHandler); // SIGTSTP to terminate program (CTRL+Z)
    #endif

    while (true) {
        pause();
    }

    return 0;
}
