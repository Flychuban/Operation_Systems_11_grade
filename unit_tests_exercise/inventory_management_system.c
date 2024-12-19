#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Structure for a task update
typedef struct TaskUpdate
{
    char description[100];   // Update description
    char timestamp[20];      // Timestamp of the update
    struct TaskUpdate *next; // Pointer to the next update
} TaskUpdate;

// Structure for a task
typedef struct Task
{
    int task_id;           // Task ID
    char title[50];        // Task title
    char description[200]; // Task description
    char status[20];       // Status (e.g., "Pending", "In Progress", "Completed")
    TaskUpdate *updates;   // Linked list of task updates
    struct Task *next;     // Pointer to the next task
} Task;

// Structure for a user
typedef struct User
{
    int user_id;       // User ID
    char name[50];     // User name
    Task *tasks;       // Linked list of tasks assigned to the user
    struct User *next; // Pointer to the next user
} User;

// Head of the users linked list
User *users_head = NULL;

// Function to create a new user
User *create_user(int user_id, const char *name)
{
    User *user = (User *)malloc(sizeof(User));
    if (!user)
        return NULL;

    user->user_id = user_id;
    strcpy(user->name, name);
    user->tasks = NULL;
    user->next = users_head; // Add to the front of the linked list
    users_head = user;

    return user;
}

// Function to find a user by ID
User *find_user(int user_id)
{
    User *current = users_head;
    while (current)
    {
        if (current->user_id == user_id)
            return current;
        current = current->next;
    }
    return NULL;
}

// Function to create a new task
Task *create_task(int task_id, const char *title, const char *description, const char *status)
{
    Task *task = (Task *)malloc(sizeof(Task));
    if (!task)
        return NULL;

    task->task_id = task_id;
    strcpy(task->title, title);
    strcpy(task->description, description);
    strcpy(task->status, status);
    task->updates = NULL;
    task->next = NULL;

    return task;
}

// Function to add a task to a user
int assign_task_to_user(int user_id, Task *task)
{
    User *user = find_user(user_id);
    if (!user || !task)
        return -1;

    task->next = user->tasks; // Add task to the front of the user's task list
    user->tasks = task;

    return 0;
}

// Function to find a task by ID for a user
Task *find_task(User *user, int task_id)
{
    Task *current = user->tasks;
    while (current)
    {
        if (current->task_id == task_id)
            return current;
        current = current->next;
    }
    return NULL;
}

// Function to add an update to a task
int add_task_update(User *user, int task_id, const char *description, const char *timestamp)
{
    Task *task = find_task(user, task_id);
    if (!task)
        return -1;

    TaskUpdate *update = (TaskUpdate *)malloc(sizeof(TaskUpdate));
    if (!update)
        return -1;

    strcpy(update->description, description);
    strcpy(update->timestamp, timestamp);
    update->next = task->updates; // Add update to the front of the update list
    task->updates = update;

    return 0;
}

// Function to change the status of a task
int change_task_status(User *user, int task_id, const char *new_status)
{
    Task *task = find_task(user, task_id);
    if (!task)
        return -1;

    strcpy(task->status, new_status);
    return 0;
}

// Function to display all tasks for a user
void display_tasks(User *user)
{
    if (!user)
        return;

    printf("Tasks for user %s (ID: %d):\n", user->name, user->user_id);
    Task *current = user->tasks;

    while (current)
    {
        printf("Task ID: %d\nTitle: %s\nDescription: %s\nStatus: %s\n", current->task_id, current->title, current->description, current->status);
        printf("Updates:\n");

        TaskUpdate *update = current->updates;
        while (update)
        {
            printf(" - [%s] %s\n", update->timestamp, update->description);
            update = update->next;
        }

        printf("\n");
        current = current->next;
    }
}

// Function to free all users, tasks, and updates
void free_all()
{
    while (users_head)
    {
        User *user = users_head;
        users_head = users_head->next;

        while (user->tasks)
        {
            Task *task = user->tasks;
            user->tasks = user->tasks->next;

            while (task->updates)
            {
                TaskUpdate *update = task->updates;
                task->updates = task->updates->next;
                free(update);
            }

            free(task);
        }

        free(user);
    }
}


void test_create_user()
{
    User* user = create_user(1, "Alice");
    assert(user != NULL);
    assert(user->user_id == 1);
    assert(strcmp(user->name, "Alice") == 0);
    assert(user->tasks == NULL);
    printf("test_create_user passed.\n");
}

void test_create_task() {
    Task* task = create_task(101, "Inventory Check", "Check all items in stock", "Pending");
    assert(task != NULL);
    assert(task->task_id == 101);
    assert(strcmp(task->title, "Inventory Check") == 0);
    assert(strcmp(task->description, "Check all items in stock") == 0);
    assert(strcmp(task->status, "Pending") == 0);
    assert(task->updates == NULL);
    printf("test_create_task passed.\n");
}

void test_assign_task_to_user() {
    User* user = create_user(2, "Bob");
    Task* task = create_task(102, "Order Restock", "Restock all low items", "In Progress");

    int result = assign_task_to_user(2, task);
    assert(result == 0);
    assert(user->tasks == task);
    printf("test_assign_task_to_user passed.\n");
}

void test_add_task_update() {
    User* user = create_user(3, "Charlie");
    Task* task = create_task(103, "Barcode Generation", "Generate barcodes for new items", "Pending");
    assign_task_to_user(3, task);

    int result = add_task_update(user, 103, "Initial barcode setup", "2024-12-19");
    assert(result == 0);
    assert(task->updates != NULL);
    assert(strcmp(task->updates->description, "Initial barcode setup") == 0);
    assert(strcmp(task->updates->timestamp, "2024-12-19") == 0);
    printf("test_add_task_update passed.\n");
}

void test_change_task_status() {
    User* user = create_user(4, "Diana");
    Task* task = create_task(104, "Sales Report", "Prepare monthly sales report", "Pending");
    assign_task_to_user(4, task);

    int result = change_task_status(user, 104, "Completed");
    assert(result == 0);
    assert(strcmp(task->status, "Completed") == 0);
    printf("test_change_task_status passed.\n");
}

void test_display_tasks() {
    User* user = create_user(5, "Eve");
    Task* task1 = create_task(105, "Inventory Audit", "Audit the inventory for discrepancies", "Pending");
    Task* task2 = create_task(106, "Supplier Review", "Review supplier performance", "In Progress");

    assign_task_to_user(5, task1);
    assign_task_to_user(5, task2);

    printf("Expected output for test_display_tasks:\n");
    display_tasks(user);  // This function will print the tasks; ensure the printed content is correct.
    printf("test_display_tasks passed (check the printed output).\n");
}

void test_find_user()
{
    create_user(6, "Frank");
    create_user(7, "Grace");

    User *user1 = find_user(6);
    User *user2 = find_user(7);
    User *user3 = find_user(999); // Non-existent user

    assert(user1 != NULL && strcmp(user1->name, "Frank") == 0);
    assert(user2 != NULL && strcmp(user2->name, "Grace") == 0);
    assert(user3 == NULL); // Should return NULL for a non-existent user
    printf("test_find_user passed.\n");
}

void test_find_task()
{
    User *user = create_user(8, "Helen");
    Task *task1 = create_task(107, "System Backup", "Backup inventory and sales data", "Pending");
    Task *task2 = create_task(108, "Data Cleanup", "Remove obsolete records", "Pending");

    assign_task_to_user(8, task1);
    assign_task_to_user(8, task2);

    Task *found_task1 = find_task(user, 107);
    Task *found_task2 = find_task(user, 108);
    Task *not_found_task = find_task(user, 999); // Non-existent task

    assert(found_task1 != NULL && strcmp(found_task1->title, "System Backup") == 0);
    assert(found_task2 != NULL && strcmp(found_task2->title, "Data Cleanup") == 0);
    assert(not_found_task == NULL); // Should return NULL for a non-existent task
    printf("test_find_task passed.\n");
}

void test_assign_multiple_tasks()
{
    User *user = create_user(9, "Ian");
    Task *task1 = create_task(109, "Order Processing", "Process pending customer orders", "Pending");
    Task *task2 = create_task(110, "Stock Replenishment", "Replenish low-stock items", "In Progress");

    int result1 = assign_task_to_user(9, task1);
    int result2 = assign_task_to_user(9, task2);

    assert(result1 == 0);
    assert(result2 == 0);
    assert(user->tasks == task2); // Newest task should be at the head of the list
    assert(user->tasks->next == task1);
    printf("test_assign_multiple_tasks passed.\n");
}

void test_add_multiple_updates()
{
    User *user = create_user(10, "Jack");
    Task *task = create_task(111, "Generate Reports", "Generate sales and inventory reports", "Pending");
    assign_task_to_user(10, task);

    int result1 = add_task_update(user, 111, "Report template created", "2024-12-01");
    int result2 = add_task_update(user, 111, "Data analysis completed", "2024-12-15");

    assert(result1 == 0);
    assert(result2 == 0);
    assert(task->updates != NULL);
    assert(strcmp(task->updates->description, "Data analysis completed") == 0); // Most recent update
    assert(strcmp(task->updates->next->description, "Report template created") == 0);
    printf("test_add_multiple_updates passed.\n");
}

void test_change_task_status_invalid_task()
{
    User *user = create_user(11, "Karen");
    Task *task = create_task(112, "Barcode Verification", "Verify barcode scanner setup", "Pending");
    assign_task_to_user(11, task);

    int result = change_task_status(user, 999, "Completed"); // Invalid task ID
    assert(result == -1);                                    // Should fail
    assert(strcmp(task->status, "Pending") == 0);            // Original status should remain unchanged
    printf("test_change_task_status_invalid_task passed.\n");
}

void test_free_all()
{
    create_user(12, "Leo");
    Task *task1 = create_task(113, "Stock Audit", "Audit the stock in the warehouse", "Pending");
    Task *task2 = create_task(114, "System Maintenance", "Perform regular system maintenance", "Pending");
    assign_task_to_user(12, task1);
    assign_task_to_user(12, task2);

    free_all();

    // After freeing, the user list should be empty
    assert(users_head == NULL);
    printf("test_free_all passed.\n");
}

void run_all_tests()
{
    test_create_user();
    test_create_task();
    test_assign_task_to_user();
    test_add_task_update();
    test_change_task_status();
    test_display_tasks();
    test_find_user();
    test_find_task();
    test_assign_multiple_tasks();
    test_add_multiple_updates();
    test_change_task_status_invalid_task();
    test_free_all();
    printf("All tests passed.\n");
}

int main() {
    run_all_tests();
    free_all();  // Clean up after tests.
    return 0;
}
