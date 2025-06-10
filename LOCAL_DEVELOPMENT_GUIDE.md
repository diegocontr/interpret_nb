# Building and Testing Local Changes in InterpretML

This guide explains how to build the InterpretML library with your local modifications (e.g., a new loss function in C++) and test them in your local Python environment.

These instructions assume you are working from the root directory of the `interpret` repository and will be running `make` commands from the `scripts` subdirectory.

## Prerequisites

Before you begin, ensure you have the following installed:
*   A C++ compiler and standard build tools (like `make`).
*   Python (the version compatible with the project) and `pip`.
*   Node.js and `npm` (for building JavaScript components).

The `Makefile` located in the `/home/diego/Dropbox/DropboxGit/interpret/scripts/` directory orchestrates the build process.

## Build Process

Follow these steps to compile and install your local version of the library:

1.  **Navigate to the scripts directory:**
    ```bash
    cd /home/diego/Dropbox/DropboxGit/interpret/scripts
    ```

2.  **Compile Native Code and Build Python Wheels:**
    Run the following command:
    ```bash
    make build
    ```
    This target performs several actions:
    *   `build-native`: Compiles the C++ components of the library.
    *   `build-javascript`: Builds the necessary JavaScript assets for visualizations.
    *   Builds Python wheels for `interpret-core` and `interpret`.

3.  **Install Local Packages into Your Python Environment:**
    After a successful build, install the packages:
    ```bash
    make install
    ```
    This target will:
    *   Ensure `make build` is up-to-date.
    *   Uninstall any existing versions of `interpret` and `interpret-core` from your current Python environment.
    *   Install the locally built `interpret-core` and `interpret` packages in "editable" mode (`pip install -e .`). This means your Python environment will directly use the code from your repository.

## Testing Your Changes

Once `make install` completes, your local Python environment is configured to use your modified version of InterpretML.

1.  **Open a Python Interpreter or Jupyter Notebook:**
    You can now import and use the `interpret` library as you normally would.

2.  **Verify Your Implementation:**
    Write Python code to specifically test the new loss function or other C++ modifications you've made. You can use the provided `Test_Local_Build.ipynb` notebook (located in `/home/diego/Dropbox/DropboxGit/interpret/examples/python/notebooks/`) as a starting point.

    Example:
    ```python
    import interpret
    print(f"Using InterpretML version: {interpret.__version__}")

    # Your test code here, e.g., initializing a model
    # that uses your new loss function and evaluating its behavior.
    ```

## Iterative Development Cycle

When you make further changes to the C++ code or other parts of the library that require recompilation:

1.  **Make your code changes.**
2.  **Navigate to the `scripts` directory** (if not already there).
3.  **Re-run the build and install commands:**
    ```bash
    make build
    make install
    ```
    This ensures your Python environment is updated with the latest compiled code.
    *Note: The `make install` command uses an "editable" install (`pip install -e .`). This means for pure Python file changes (i.e., changes to `.py` files that are not part of the native extension build process and do not affect `setup.py`), your Python environment will typically pick up these changes automatically without needing to re-run `make install`. However, for C++ changes (like your new loss function), modifications to `setup.py`, or any other changes that affect the compiled native components, you **must** re-run `make build` followed by `make install` (or simply `make install`, as it depends on `make build`) to recompile the native code and ensure your Python environment uses the updated versions.*

## Troubleshooting

If you encounter issues during the build process, here are some common problems and solutions:

### Error: `/usr/bin/make: /usr/bin/make: cannot execute binary file`

This error indicates a problem with the `make` executable itself on your system. Here's how to troubleshoot:

1.  **Verify `make` Installation and Version:**
    Open your terminal and run:
    ```bash
    make --version
    ```
    If this command fails or shows an error, `make` might not be installed correctly or your `PATH` might be misconfigured.

2.  **Check `make` Path and Permissions:**
    Confirm where the `make` command is located:
    ```bash
    which make
    ```
    This typically outputs `/usr/bin/make` or a similar path. Then check its permissions:
    ```bash
    ls -l $(which make)
    ```
    The output should show execute permissions (e.g., `-rwxr-xr-x`). If not, there might be a deeper system issue.

3.  **Reinstall Build Tools:**
    The `make` utility is usually part of a larger package of development tools. Reinstalling these might resolve the issue.
    *   **Debian/Ubuntu:**
        ```bash
        sudo apt update
        sudo apt install --reinstall build-essential
        ```
    *   **Fedora/CentOS/RHEL:**
        ```bash
        sudo dnf groupinstall "Development Tools" 
        # or sudo yum groupinstall "Development Tools"
        ```
    *   **macOS:** Ensure Xcode Command Line Tools are installed:
        ```bash
        xcode-select --install
        ```

4.  **Check Filesystem Mount Options:**
    If `/usr` or the filesystem containing `make` is mounted with the `noexec` option, binaries cannot be executed. Check with:
    ```bash
    mount | grep $(dirname $(which make))
    ```
    If `noexec` is listed for the relevant mount point, you'll need to remount it without this option (this usually requires system administrator privileges and understanding of your system setup).

5.  **Architecture Mismatch:**
    This is rare for standard utilities like `make`, but ensure your operating system and the `make` binary are for the same architecture (e.g., x86_64, arm64).

### General Build Failures

*   **Missing Compilers/Headers:** Ensure you have a C++ compiler (like g++) and development libraries installed. The `build-essential` package (or equivalents mentioned above) usually covers this.
*   **Python Version:** Verify that the `python` and `pip` commands in your terminal point to the versions intended for this project. Using a Python virtual environment is highly recommended.
*   **Node.js/npm Issues:** If `make build-javascript` fails, ensure Node.js and npm are correctly installed and accessible in your `PATH`.
*   **Dependency Conflicts:** If `pip install` steps fail, there might be Python package dependency conflicts. Carefully review the error messages.

If problems persist, check the project's issue tracker for similar problems or consider providing detailed error logs when asking for help.

## Cleaning Up (Optional)

If you want to remove build artifacts:
```bash
make clean
```
This command will delete build directories and, if you've used Docker targets from the Makefile, associated Docker volumes and images.

By following these steps, you can effectively develop, build, and test your contributions to the InterpretML library locally.
