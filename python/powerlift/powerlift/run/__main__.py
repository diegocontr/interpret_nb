"""This is called to run a trial by worker nodes (local / remote)."""


def run_trials(
    experiment_id,
    runner_id,
    db_url,
    timeout,
    raise_exception,
    print_exceptions=False,
    max_attempts=5,
    return_after_one=False,
):
    """Runs trials. Includes wheel installation and timeouts."""
    import ast
    import gc
    import traceback

    from powerlift.bench.store import Store
    from powerlift.executors.base import timed_run

    store = Store(db_url, print_exceptions=print_exceptions, max_attempts=max_attempts)

    trial_run_fn = None
    if return_after_one:
        try:
            with open("trial_run_fn.py") as file:
                trial_run_fn = file.read()
        except FileNotFoundError:
            pass

    if trial_run_fn is None:
        if print_exceptions:
            print("Getting trial_run_fn from database.")
        trial_run_fn = store.get_trial_fn(experiment_id)
        if return_after_one:
            with open("trial_run_fn.py", "w") as file:
                file.write(trial_run_fn)

    trial_run_fn = ast.parse(trial_run_fn)
    if not isinstance(trial_run_fn, ast.Module) or not isinstance(
        trial_run_fn.body[0], ast.FunctionDef
    ):
        msg = "Serialized code not valid."
        raise RuntimeError(msg)

    func_name = r"wired_function"
    trial_run_fn.body[0].name = func_name
    compiled = compile(trial_run_fn, "<string>", "exec")
    scope = locals()
    exec(compiled, scope, scope)
    trial_run_fn = locals()[func_name]

    while True:
        errmsg = "UNKNOWN FAILURE"

        trial = store.pick_trial(experiment_id, runner_id)
        if trial is None:
            if print_exceptions:
                print("No more work to start!")
            return False

        # Run trial
        try:
            # if the previous trial function created cyclic garbage, clear it.
            gc.collect()
            _, duration, timed_out = timed_run(
                lambda: trial_run_fn(trial), timeout_seconds=timeout
            )
            if timed_out:
                msg = f"Timeout failure ({duration})"
                raise RuntimeError(msg)
            errmsg = None
        except Exception:
            errmsg = f"EXCEPTION: {trial.task.origin}, {trial.task.name}, {trial.method}, {trial.meta}, {trial.task.n_classes}, {trial.task.n_features}, {trial.task.n_samples}\n{traceback.format_exc()}"
            if raise_exception:
                raise
        except BaseException:
            errmsg = f"EXCEPTION: {trial.task.origin}, {trial.task.name}, {trial.method}, {trial.meta}, {trial.task.n_classes}, {trial.task.n_features}, {trial.task.n_samples}\n{traceback.format_exc()}"
            raise
        finally:
            store.end_trial(trial.id, errmsg)

        if return_after_one:
            return True


if __name__ == "__main__":
    print("STARTING POWERLIFT RUNNER")

    import sys
    import traceback

    try:
        import os

        experiment_id = os.getenv("EXPERIMENT_ID")
        runner_id = os.getenv("RUNNER_ID")
        db_url = os.getenv("DB_URL")
        timeout = float(os.getenv("TIMEOUT", 0.0))
        is_more = run_trials(
            experiment_id,
            runner_id,
            db_url,
            timeout,
            False,
            True,
            None,
            True,
        )
    except Exception as e:
        print("EXCEPTION:")
        print("".join(traceback.format_exception(type(e), e, e.__traceback__)))
        sys.exit(65)
    except BaseException as e:
        print("EXCEPTION:")
        print("".join(traceback.format_exception(type(e), e, e.__traceback__)))
        sys.exit(64)

    sys.exit(1 if is_more else 0)
