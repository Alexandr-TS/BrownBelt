/*

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

using namespace std;

// ������������ ��� ��� ������� ������
enum class TaskStatus {
    NEW,          // �����
    IN_PROGRESS,  // � ����������
    TESTING,      // �� ������������
    DONE          // ���������
};

// ��������� ���-������� ��� map<TaskStatus, int>,
// ������������ ������� ���������� ����� ������� �������
using TasksInfo = map<TaskStatus, int>;
*/

#include <unordered_map>

class TeamTasks {
public:
    // �������� ���������� �� �������� ����� ����������� ������������
    const TasksInfo& GetPersonTasksInfo(const string& person) const {
        return info.at(person);
    }

    // �������� ����� ������ (� ������� NEW) ��� ����������� �������������
    void AddNewTask(const string& person) {
        ++info[person][TaskStatus::NEW];
    }

    // �������� ������� �� ������� ���������� ����� ����������� ������������,
    // ����������� ��. ����
    tuple<TasksInfo, TasksInfo> PerformPersonTasks(
        const string& person, int task_count) {
        auto& pers_info = info[person];

        TasksInfo performed, left;
        for (auto& el : pers_info) {
            if (el.first == TaskStatus::DONE) {
                left[el.first] += el.second;
                break;
            }
            int cnt_performed = min(task_count, el.second);
            performed[GetNext(el.first)] += cnt_performed;
            left[el.first] += (el.second - cnt_performed);
            task_count -= cnt_performed;
        }
        Clear(performed);
        Clear(left);
        pers_info.clear();
        for (auto el : performed) {
            pers_info[el.first] += el.second;
        }
        for (auto el : left) {
            pers_info[el.first] += el.second;
        }
        left.erase(TaskStatus::DONE);
        return { performed, left };
    }

private:
    unordered_map<string, TasksInfo> info;

    TaskStatus GetNext(TaskStatus status) {
        switch (status) {
        case TaskStatus::NEW:
            return TaskStatus::IN_PROGRESS;
        case TaskStatus::IN_PROGRESS:
            return TaskStatus::TESTING;
        case TaskStatus::TESTING:
            return TaskStatus::DONE;
        default:
            return TaskStatus::DONE;
        }
    }

    void Clear(TasksInfo& status) {
        TasksInfo news;
        for (auto el : status) {
            if (el.second) {
                news.insert(el);
            }
        }
        status = news;
    }
};
/*

// ��������� ������� �� ��������, ����� ����� �����������
// ���������� � ������������� ������ � ������� [] � �������� 0,
// �� ����� ��� ���� �������� �������
void PrintTasksInfo(TasksInfo tasks_info) {
    cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
        ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
        ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
        ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << endl;
}

int main() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    tie(updated_tasks, untouched_tasks) =
        tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    return 0;
}


*/
