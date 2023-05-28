#include "tuner.h"

Tuner::Tuner()
{
    
}

void Tuner::start()
{
    this->ga.gen_id = 0;
    this->ga.pool.clear();

    srand(uint32_t(time(nullptr)));

    // Variables
    int starting_batch = 0;

    // Get mode
    using namespace std;
    system("CLS");
    int mode;
    cout << "What do you want to do?" << endl;
    cout << "    1 - Improve standard" << endl;
    cout << "    2 - Continue from previous" << endl;
    cin >> mode;

    Eval::SaveData data;

    // Set mode
    if (mode == 1) {
        this->ga.init(Eval::DEFAULT_WEIGHT, 200);
    }
    else if (mode == 2) {
        system("CLS");
        cout << "What batch?" << endl;
        cin >> starting_batch;
        this->ga.load(starting_batch);
    }
    else return;

    system("CLS");

    // Main loop
    while (true)
    {
        srand(uint32_t(time(nullptr)));

        const int test_count = 1;

        int best_index = 0;
        double best_score = 0.0;
        double total_score = 0.0;

        std::vector<Cell::Pair> queue[test_count];
        for (int i = 0; i < test_count; ++i) {
            queue[i] = create_queue();
        }

        for (int i = 0; i < this->ga.pool.size(); ++i) {
            if (this->ga.pool[i].score != 0.0) {
                total_score += this->ga.pool[i].score;
                continue;
            }

            cout << "gen: " << this->ga.gen_id << endl;
            cout << "best: " << (best_score / test_count) << "    id: " << best_index << endl;
            cout << "avrg: " << (total_score / test_count / i) << endl;
            cout << "process: " << i << "/" << int(this->ga.pool.size()) << endl;

            for (int k = 0; k < test_count; ++k) {
                this->ga.pool[i].score += get_score(queue[k], this->ga.pool[i].heuristic);
            }

            if (this->ga.pool[i].score > best_score) {
                best_index = i;
            }

            best_score = std::max(best_score, this->ga.pool[i].score);
            total_score += this->ga.pool[i].score;

            this->ga.save();

            system("CLS");
        }

        this->ga.simulate();

        system("CLS");
    }
}
