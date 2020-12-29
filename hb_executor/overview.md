# channel
# future
join: `when_all()`, `when_any()`
# fork_joint_task
`fork_join_task fork()`
`fork_join_task complete(V val)`
`bool ready()`
`V join()`

# executor
`.cancel()`, `.done()`

fork_join_task fork(fork_join_task& task)
{
    auto subtask = task.fork();
    if (subtask) {
        local_queue.push(subtask);
    }
    return subtask;
}

void join(fork_join_task task, fork_join_task subtask)
{
    if (timeout(subtask.ready())) {
        local_deque.push(task);
    }
    task.complete(subtask.join());
}

void run_task(fork_join_task task)
{
    auto subtask = fork(task);
    task.run();
    join(std::move(task), std::move(subtask));
}

void worker()
{
    while (!done()) {
        fork_join_task task;
        if (local_queue.pop(task)) ||
            steal(task)) {
                run_task(std::move(task));
        }
        else {
            yield();
        }
    }
}

