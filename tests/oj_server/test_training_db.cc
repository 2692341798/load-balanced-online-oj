#include "../../oj_server/oj_model.hpp"
#include <iostream>
#include <cassert>

using namespace ns_model;

void TestTrainingListCRUD() {
    std::cout << "Starting Training List CRUD Test..." << std::endl;
    Model model;

    // 1. Create
    TrainingList list;
    list.title = "Test Training List";
    list.description = "Description for test";
    list.difficulty = "Easy";
    list.tags = "[\"DP\", \"Graph\"]";
    list.author_id = "1"; // Assuming user 1 exists, or at least foreign key is disabled (it is commented out)
    list.visibility = "public";

    int new_id = -1;
    bool res = model.CreateTrainingList(list, &new_id);
    if (!res) {
        std::cerr << "CreateTrainingList Failed!" << std::endl;
        exit(1);
    }
    std::cout << "CreateTrainingList Success, ID: " << new_id << std::endl;

    // 2. Read
    TrainingList get_list;
    res = model.GetTrainingList(std::to_string(new_id), &get_list);
    if (!res) {
        std::cerr << "GetTrainingList Failed!" << std::endl;
        exit(1);
    }
    std::cout << "GetTrainingList Success" << std::endl;
    
    // Verify fields
    assert(get_list.title == list.title);
    assert(get_list.description == list.description);
    assert(get_list.difficulty == list.difficulty);
    assert(get_list.tags == list.tags);
    assert(get_list.visibility == list.visibility);
    std::cout << "Fields Verification Passed" << std::endl;

    // 3. Update
    get_list.title = "Updated Title";
    get_list.difficulty = "Hard";
    res = model.UpdateTrainingList(get_list);
    if (!res) {
        std::cerr << "UpdateTrainingList Failed!" << std::endl;
        exit(1);
    }
    std::cout << "UpdateTrainingList Success" << std::endl;

    // Verify Update
    TrainingList updated_list;
    model.GetTrainingList(std::to_string(new_id), &updated_list);
    assert(updated_list.title == "Updated Title");
    assert(updated_list.difficulty == "Hard");
    std::cout << "Update Verification Passed" << std::endl;

    // 4. Delete
    res = model.DeleteTrainingList(std::to_string(new_id));
    if (!res) {
        std::cerr << "DeleteTrainingList Failed!" << std::endl;
        exit(1);
    }
    std::cout << "DeleteTrainingList Success" << std::endl;

    // Verify Delete
    TrainingList deleted_list;
    res = model.GetTrainingList(std::to_string(new_id), &deleted_list);
    assert(res == false);
    std::cout << "Delete Verification Passed" << std::endl;

    std::cout << "All Training List Tests Passed!" << std::endl;
}

int main() {
    TestTrainingListCRUD();
    return 0;
}
